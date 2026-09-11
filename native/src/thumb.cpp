#include <napi.h>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>

#include "ffmpeg.h"

constexpr int MAX_WIDTH = 640;

// Helper function to write JPEG image
static bool writeJPEG(const std::string& outputPath, const uint8_t* data, int size) {
	std::ofstream file(outputPath, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}
	file.write(reinterpret_cast<const char*>(data), size);
	file.close();
	return true;
}

// Helper function to encode frame as JPEG
static bool encodeJPEG(AVCodecContext* enc_ctx, AVFrame* frame, std::vector<uint8_t>& output) {
	int ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		return false;
	}

	AVPacket* packet = av_packet_alloc();
	if (!packet) {
		return false;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			break;
		} else if (ret < 0) {
			av_packet_free(&packet);
			return false;
		}

		// Append packet data to output
		size_t current_size = output.size();
		output.resize(current_size + packet->size);
		memcpy(output.data() + current_size, packet->data, packet->size);
		av_packet_unref(packet);
	}

	av_packet_free(&packet);
	return true;
}

Napi::Value ExtractThumbnail(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	
	// Parse arguments: ExtractThumbnail(videoPath, outputPath, timestampSeconds)
	if (info.Length() < 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsNumber()) {
		Napi::TypeError::New(env, "Expected (videoPath, outputPath, seconds)")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	std::string videoPath = info[0].As<Napi::String>().Utf8Value();
	std::string outputPath = info[1].As<Napi::String>().Utf8Value();
	double timestampSeconds = info[2].As<Napi::Number>().DoubleValue();

	int max_width = MAX_WIDTH;
	if (info.Length() > 3 && info[3].IsNumber()) {
		max_width = info[3].As<Napi::Number>().Int32Value();
	}

	// --- Open input file ---
	AVFormatContext* fmt_ctx = nullptr;
	if (avformat_open_input(&fmt_ctx, videoPath.c_str(), nullptr, nullptr) < 0) {
		Napi::Error::New(env, "Could not open input file").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not find stream info").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Find the best video stream ---
	int video_stream_index = -1;
	for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			// Skip attached pictures (cover art)
			if (fmt_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
				continue;
			}
			video_stream_index = i;
			break;
		}
	}

	if (video_stream_index == -1) {
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "No video stream found").ThrowAsJavaScriptException();
		return env.Null();
	}

	AVStream* video_stream = fmt_ctx->streams[video_stream_index];
	AVCodecParameters* codecpar = video_stream->codecpar;

	// --- Find decoder ---
	const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
	if (!decoder) {
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not find decoder").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Allocate decoder context ---
	AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
	if (!dec_ctx) {
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not allocate decoder context").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (avcodec_parameters_to_context(dec_ctx, codecpar) < 0) {
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not copy codec parameters").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Open decoder ---
	if (avcodec_open2(dec_ctx, decoder, nullptr) < 0) {
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not open decoder").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Seek to timestamp ---
	// Convert timestamp to stream time base
	int64_t seek_target = av_rescale_q(
		(int64_t)(timestampSeconds * AV_TIME_BASE),
		AV_TIME_BASE_Q,
		video_stream->time_base
	);

	if (av_seek_frame(fmt_ctx, video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
		// If seeking fails, try seeking to a slightly earlier time
		int64_t seek_earlier = av_rescale_q(
			(int64_t)((timestampSeconds - 1.0) * AV_TIME_BASE),
			AV_TIME_BASE_Q,
			video_stream->time_base
		);
		if (av_seek_frame(fmt_ctx, video_stream_index, seek_earlier, AVSEEK_FLAG_BACKWARD) < 0) {
			avcodec_free_context(&dec_ctx);
			avformat_close_input(&fmt_ctx);
			Napi::Error::New(env, "Could not seek to timestamp").ThrowAsJavaScriptException();
			return env.Null();
		}
	}

	// Flush decoder buffers after seek
	avcodec_flush_buffers(dec_ctx);

	// --- Allocate frame and packet ---
	AVFrame* frame = av_frame_alloc();
	AVPacket* packet = av_packet_alloc();
	if (!frame || !packet) {
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not allocate frame or packet").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Read and decode until we get a frame ---
	bool got_frame = false;
	int ret;
	while (!got_frame) {
		ret = av_read_frame(fmt_ctx, packet);
		if (ret < 0) {
			break;
		}

		if (packet->stream_index != video_stream_index) {
			av_packet_unref(packet);
			continue;
		}

		ret = avcodec_send_packet(dec_ctx, packet);
		if (ret < 0) {
			av_packet_unref(packet);
			continue;
		}

		while (ret >= 0) {
			ret = avcodec_receive_frame(dec_ctx, frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				break;
			} else if (ret < 0) {
				av_packet_unref(packet);
				avcodec_free_context(&dec_ctx);
				avformat_close_input(&fmt_ctx);
				Napi::Error::New(env, "Error decoding frame").ThrowAsJavaScriptException();
				return env.Null();
			}

			// We got a frame!
			got_frame = true;
			break;
		}

		av_packet_unref(packet);
		if (got_frame) {
			break;
		}
	}

	if (!got_frame) {
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not decode any frame").ThrowAsJavaScriptException();
		return env.Null();
	}

	int dst_width = frame->width;
	int dst_height = frame->height;

	if (dst_width > max_width) {
		dst_width = max_width;
		dst_height = av_rescale(frame->height, max_width, frame->width);

		// YUV420P requires even dimensions
		dst_width &= ~1;
		dst_height &= ~1;
	}


	// --- Encode frame as JPEG ---
	// Find JPEG encoder
	const AVCodec* jpeg_encoder = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	if (!jpeg_encoder) {
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not find JPEG encoder").ThrowAsJavaScriptException();
		return env.Null();
	}

	// Allocate encoder context
	AVCodecContext* enc_ctx = avcodec_alloc_context3(jpeg_encoder);
	if (!enc_ctx) {
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not allocate encoder context").ThrowAsJavaScriptException();
		return env.Null();
	}

	// Set encoder parameters
	enc_ctx->width = dst_width;
	enc_ctx->height = dst_height;
	enc_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
	enc_ctx->time_base = (AVRational){1, 90000};
	enc_ctx->global_quality = FF_QP2LAMBDA * 2;
	enc_ctx->flags |= AV_CODEC_FLAG_QSCALE;
	
	// Set quality (q:v) - 2 is high quality
	//av_dict_set_int((AVDictionary**)&enc_ctx->priv_data, "q", 2, 0);

	if (avcodec_open2(enc_ctx, jpeg_encoder, nullptr) < 0) {
		avcodec_free_context(&enc_ctx);
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not open JPEG encoder").ThrowAsJavaScriptException();
		return env.Null();
	}

	// Convert frame to YUVJ420P if needed
	AVFrame* jpeg_frame = av_frame_alloc();
	if (!jpeg_frame) {
		avcodec_free_context(&enc_ctx);
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not allocate JPEG frame").ThrowAsJavaScriptException();
		return env.Null();
	}

	jpeg_frame->width = dst_width;
	jpeg_frame->height = dst_height;
	jpeg_frame->format = AV_PIX_FMT_YUVJ420P;

	if (av_frame_get_buffer(jpeg_frame, 32) < 0) {
		av_frame_free(&jpeg_frame);
		avcodec_free_context(&enc_ctx);
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not allocate JPEG frame buffer").ThrowAsJavaScriptException();
		return env.Null();
	}

	// Convert if necessary
	SwsContext* sws_ctx = sws_getContext(
		frame->width,
		frame->height,
		static_cast<AVPixelFormat>(frame->format),

		dst_width,
		dst_height,
		AV_PIX_FMT_YUVJ420P,

		SWS_LANCZOS, //SWS_BILINEAR,
		nullptr,
		nullptr,
		nullptr
	);

	if (!sws_ctx) {
		av_frame_free(&jpeg_frame);
		avcodec_free_context(&enc_ctx);
		// cleanup...
		Napi::Error::New(env, "Could not create scale context")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	sws_scale(
		sws_ctx,
		frame->data,
		frame->linesize,
		0,
		frame->height,
		jpeg_frame->data,
		jpeg_frame->linesize
	);

	sws_freeContext(sws_ctx);

	

	// Encode as JPEG
	std::vector<uint8_t> jpeg_data;
	if (!encodeJPEG(enc_ctx, jpeg_frame, jpeg_data)) {
		av_frame_free(&jpeg_frame);
		avcodec_free_context(&enc_ctx);
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not encode JPEG").ThrowAsJavaScriptException();
		return env.Null();
	}

	// Send NULL frame to flush encoder (get any remaining packets)
	encodeJPEG(enc_ctx, nullptr, jpeg_data);

	// --- Write JPEG file ---
	if (!writeJPEG(outputPath, jpeg_data.data(), jpeg_data.size())) {
		av_frame_free(&jpeg_frame);
		avcodec_free_context(&enc_ctx);
		av_packet_free(&packet);
		av_frame_free(&frame);
		avcodec_free_context(&dec_ctx);
		avformat_close_input(&fmt_ctx);
		Napi::Error::New(env, "Could not write output file").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Cleanup ---
	av_frame_free(&jpeg_frame);
	avcodec_free_context(&enc_ctx);
	av_packet_free(&packet);
	av_frame_free(&frame);
	avcodec_free_context(&dec_ctx);
	avformat_close_input(&fmt_ctx);

	return Napi::Boolean::New(env, true);
}
