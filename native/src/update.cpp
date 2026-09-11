
#include <napi.h>

#include <fstream>
#include <map>
#include <vector>
#include <filesystem>

#include "ffmpeg.h"
#include "common.h"

static void setMetadata(Napi::Object tags, AVDictionary* metadata) {
	Napi::Array propertyNames = tags.GetPropertyNames();

	for (unsigned int i = 0; i < propertyNames.Length(); i++) {

		auto prop = propertyNames.Get(i);
		if (!prop.IsString()) continue;

		std::string key = prop.As<Napi::String>().Utf8Value();
		if (key == "cover") continue;
		
		Napi::Value val = tags.Get(key);
		if (val.IsString()) {
			std::string value = val.As<Napi::String>().Utf8Value();
			
			// Map common tag names
			if (key == "year") key = "date";
			if (key == "track_no") key = "track";
			if (key == "track_number") key = "track";
			
			av_dict_set(&metadata, key.c_str(), value.c_str(), 0);
		}
	}
}

Napi::Value UpdateMetadata(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	// Parse arguments
	std::string srcPath;
	std::string dstPath;
	Napi::Object inputTags;
	bool isInPlaceUpdate = false;

	if (info.Length() == 2 && info[0].IsString() && info[1].IsObject()) {
		srcPath = info[0].As<Napi::String>().Utf8Value();

		std::filesystem::path p(srcPath);

		std::string filename = p.filename().string();
		size_t dot_pos = filename.find_last_of('.');
		if (dot_pos != std::string::npos) {
			filename.insert(dot_pos, ".tmp");
		}
	
		std::filesystem::path newPath = p.parent_path() / filename;

		dstPath = newPath.string();
		inputTags = info[1].As<Napi::Object>();
		isInPlaceUpdate = true;
	} 
	else if (info.Length() >= 3 && info[0].IsString() && 
			 info[1].IsString() && info[2].IsObject()) {
		srcPath = info[0].As<Napi::String>().Utf8Value();
		dstPath = info[1].As<Napi::String>().Utf8Value();
		inputTags = info[2].As<Napi::Object>();
	} 
	else {
		Napi::TypeError::New(env, 
			"Invalid arguments. Use (String, Object) or (String, String, Object)")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Parse image parameter ---
	CoverImage cover;
	std::string targetDesc = "Cover (front)";

	auto image = inputTags.Get("cover");
	if (image) {

		if (image.IsString()) {
			auto path = image.As<Napi::String>().Utf8Value();

			cover.load(path);
		}
		else if (image.IsBuffer()) {
			Napi::Buffer<uint8_t> buf = image.As<Napi::Buffer<uint8_t>>();
			cover.load(buf.Data(), buf.Length());
		}
		else {
			auto obj = image.As<Napi::Object>();

			Napi::Buffer<uint8_t> buf = obj.Get("data").As<Napi::Buffer<uint8_t>>();
			auto mime = obj.Get("mime").As<Napi::String>().Utf8Value();

			cover.load(buf.Data(), buf.Length(), mime);
		}
	}

	// --- Open input file ---
	AVFormatContext* ifmt_ctx = nullptr;
	if (avformat_open_input(&ifmt_ctx, srcPath.c_str(), nullptr, nullptr) < 0) {
		Napi::Error::New(env, "Could not open source file").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (avformat_find_stream_info(ifmt_ctx, nullptr) < 0) {
		avformat_close_input(&ifmt_ctx);
		Napi::Error::New(env, "Could not find stream info").ThrowAsJavaScriptException();
		return env.Null();
	}

	// --- Create output context ---
	AVFormatContext* ofmt_ctx = nullptr;
	
	const AVOutputFormat* oformat = av_guess_format(nullptr, dstPath.c_str(), nullptr);
	if (!oformat) {
		oformat = ifmt_ctx->oformat;
	}

	if (!oformat) {
		avformat_close_input(&ifmt_ctx);
		Napi::Error::New(env, "Could not determine output format").ThrowAsJavaScriptException();
		return env.Null();
	}

	log("Out format: %s", oformat->name);

	if (avformat_alloc_output_context2(&ofmt_ctx, oformat, nullptr, dstPath.c_str()) < 0) {
		avformat_close_input(&ifmt_ctx);
		Napi::Error::New(env, "Could not create output context").ThrowAsJavaScriptException();
		return env.Null();
	}

	bool hasFormatMeta = strcmp(oformat->name, "ogg");

	// --- First, identify the cover stream in input ---
	int existingCoverIndex = -1;
	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream* in_stream = ifmt_ctx->streams[i];
		if (in_stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
			existingCoverIndex = i;
			break;
		}
	}

	// --- Copy streams from input to output ---
	std::map<int, int> stream_map;
	int newCoverStreamIndex = -1;
	
	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream* in_stream = ifmt_ctx->streams[i];
		
		// Skip the existing cover stream if we're adding a new one
		if (!cover.empty() && i == existingCoverIndex) {
			continue;
		}
		
		AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
		if (!out_stream) {
			avformat_close_input(&ifmt_ctx);
			avformat_free_context(ofmt_ctx);
			Napi::Error::New(env, "Could not create output stream").ThrowAsJavaScriptException();
			return env.Null();
		}
		
		// Copy codec parameters
		int ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
		if (ret < 0) {
			avformat_close_input(&ifmt_ctx);
			avformat_free_context(ofmt_ctx);
			Napi::Error::New(env, "Could not copy codec parameters").ThrowAsJavaScriptException();
			return env.Null();
		}
		
		// If this is a cover stream and dimensions are 0, try to extract them
		if (i == existingCoverIndex && cover.empty()) {
			int width = 0, height = 0;
			
			// Try to get dimensions using FFmpeg
			if (getDimensionsFromStream(in_stream, width, height)) {
				out_stream->codecpar->width = width;
				out_stream->codecpar->height = height;
				fprintf(stderr, "Detected cover dimensions: %dx%d\n", width, height);
			} else {
				// If we still can't get dimensions, use reasonable defaults
				fprintf(stderr, "Warning: Could not detect cover dimensions, using defaults\n");
				out_stream->codecpar->width = 500;
				out_stream->codecpar->height = 500;
			}
			
			// Copy the attached picture data to the output stream
			if (in_stream->attached_pic.data && in_stream->attached_pic.size > 0) {
				ret = av_new_packet(&out_stream->attached_pic, in_stream->attached_pic.size);
				if (ret < 0) {
					avformat_close_input(&ifmt_ctx);
					avformat_free_context(ofmt_ctx);
					Napi::Error::New(env, "Could not copy attached picture").ThrowAsJavaScriptException();
					return env.Null();
				}
				memcpy(out_stream->attached_pic.data, in_stream->attached_pic.data, 
					   in_stream->attached_pic.size);
				out_stream->attached_pic.size = in_stream->attached_pic.size;
				out_stream->attached_pic.stream_index = out_stream->index;
				out_stream->attached_pic.flags |= AV_PKT_FLAG_KEY;
				out_stream->attached_pic.pts = AV_NOPTS_VALUE;
				out_stream->attached_pic.dts = AV_NOPTS_VALUE;
				out_stream->attached_pic.duration = 0;
			}
		}
		
		// Clear codec tag to let muxer choose appropriate tag
		out_stream->codecpar->codec_tag = 0;
		
		// Copy disposition
		out_stream->disposition = in_stream->disposition;
		
		// Copy time base
		out_stream->time_base = in_stream->time_base;
		
		// Copy other important stream properties
		out_stream->avg_frame_rate = in_stream->avg_frame_rate;
		out_stream->r_frame_rate = in_stream->r_frame_rate;
		out_stream->sample_aspect_ratio = in_stream->sample_aspect_ratio;
		
		// Store mapping
		stream_map[i] = out_stream->index;
	}

	// --- Add new cover stream if needed ---

	if (hasFormatMeta) {
		AVDictionary* metadata = nullptr;
		av_dict_copy(&metadata, ifmt_ctx->metadata, 0);

		setMetadata(inputTags, metadata);

		// Set ID3v2 version for MP3 files
		std::string format_name = ofmt_ctx->oformat->name;
		if (format_name.find("mp3") != std::string::npos) {
			av_dict_set(&metadata, "id3v2_version", "3", 0);
		}

		ofmt_ctx->metadata = metadata;

		if (!cover.empty()) {

			newCoverStreamIndex = cover.addStream(ofmt_ctx);

			log("Added cover stream: %d", newCoverStreamIndex);

			if (newCoverStreamIndex < 0) {
				avformat_close_input(&ifmt_ctx);
				avformat_free_context(ofmt_ctx);
				Napi::Error::New(env, "Could not create cover stream").ThrowAsJavaScriptException();
				return env.Null();
			}
		}
	}
	else {
		AVDictionary* metadata = nullptr;
		av_dict_copy(&metadata, ifmt_ctx->streams[0]->metadata, 0);

		setMetadata(inputTags, metadata);


		if (!cover.empty()) {
			auto picture = cover.encode();

			if (!picture.empty()) {
				log("Adding METADATA_BLOCK_PICTURE: %lu", picture.size());

				av_dict_set(
					&metadata,
					"METADATA_BLOCK_PICTURE",
					picture.c_str(),
					0
				);
			}
		}

		ofmt_ctx->streams[0]->metadata = metadata;
	}

	// --- Open output file ---
	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		int ret = avio_open(&ofmt_ctx->pb, dstPath.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			std::string error = "Could not open output file: " + std::string(errbuf);
			avformat_close_input(&ifmt_ctx);
			avformat_free_context(ofmt_ctx);
			Napi::Error::New(env, error).ThrowAsJavaScriptException();
			return env.Null();
		}
	}

	// --- Write header ---
	AVDictionary* opts = nullptr;
	int ret = avformat_write_header(ofmt_ctx, &opts);
	if (ret < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(ret, errbuf, sizeof(errbuf));
		std::string error = "avformat_write_header failed: " + std::string(errbuf);
		
		if (opts) {
			av_dict_free(&opts);
		}
		avio_closep(&ofmt_ctx->pb);
		avformat_close_input(&ifmt_ctx);
		avformat_free_context(ofmt_ctx);
		Napi::Error::New(env, error).ThrowAsJavaScriptException();
		return env.Null();
	}
	if (opts) {
		av_dict_free(&opts);
	}

	// If we have a new cover, send it as a packet first
	if (newCoverStreamIndex >= 0) {
		log("Adding cover image to stream: %d", newCoverStreamIndex);
		cover.addToStream(ofmt_ctx, newCoverStreamIndex);
	}

	// --- Copy frames ---
	AVPacket* packet = av_packet_alloc();
	if (!packet) {
		avio_closep(&ofmt_ctx->pb);
		avformat_close_input(&ifmt_ctx);
		avformat_free_context(ofmt_ctx);
		Napi::Error::New(env, "Could not allocate packet").ThrowAsJavaScriptException();
		return env.Null();
	}

	// Read and write the rest of the packets
	while (av_read_frame(ifmt_ctx, packet) >= 0) {
		int in_stream_index = packet->stream_index;
		
		// Skip the original cover stream if we added a new one
		if (!cover.empty() && in_stream_index == existingCoverIndex) {
			av_packet_unref(packet);
			continue;
		}
		
		// Find output stream mapping
		auto it = stream_map.find(in_stream_index);
		if (it == stream_map.end()) {
			av_packet_unref(packet);
			continue;
		}
		
		int out_stream_index = it->second;
		AVStream* in_stream = ifmt_ctx->streams[in_stream_index];
		AVStream* out_stream = ofmt_ctx->streams[out_stream_index];
		
		// Rescale timestamps
		packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, 
									   out_stream->time_base, 
									   (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, 
									   out_stream->time_base, 
									   (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->duration = av_rescale_q(packet->duration, in_stream->time_base, 
										out_stream->time_base);
		packet->pos = -1;
		packet->stream_index = out_stream_index;
		
		ret = av_interleaved_write_frame(ofmt_ctx, packet);
		if (ret < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			fprintf(stderr, "Error writing packet: %s\n", errbuf);
		}
		
		av_packet_unref(packet);
	}

	// --- Write trailer ---
	av_write_trailer(ofmt_ctx);

	// --- Cleanup ---
	av_packet_free(&packet);
	avformat_close_input(&ifmt_ctx);
	
	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		avio_closep(&ofmt_ctx->pb);
	}
	avformat_free_context(ofmt_ctx);

	// --- Atomic replace for in-place update ---
	if (isInPlaceUpdate) {
		if (std::remove(srcPath.c_str()) != 0) {
			Napi::Error::New(env, "Could not remove original file").ThrowAsJavaScriptException();
			return env.Null();
		}
		if (std::rename(dstPath.c_str(), srcPath.c_str()) != 0) {
			Napi::Error::New(env, "Could not rename temporary file").ThrowAsJavaScriptException();
			return env.Null();
		}
	}

	return Napi::Boolean::New(env, true);
}
