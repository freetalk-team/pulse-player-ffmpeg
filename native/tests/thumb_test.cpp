#include <string>
#include <vector>
#include <fstream>
#include <cmath>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavutil/avassert.h>
}

void extract(const std::string& videoPath, const std::string& outputPath, int sec);


int main(int argc, const char* argv[]) {

    std::string videoPath(argv[1]);
    std::string outPath(argv[2]);

    int sec = 5;
    if (argc > 3) {

    }

    extract(videoPath, outPath, sec);

    return 0;
}

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

static void log_callback(void *, int level, const char *fmt, va_list vl)
{
    vfprintf(stderr, fmt, vl);
}

void extract(const std::string& videoPath, const std::string& outputPath, int timestampSeconds) {
    
    int ret;

    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(log_callback);

    // --- Open input file ---
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, videoPath.c_str(), nullptr, nullptr) < 0) {
        fprintf(stderr, "Could not open input file: %s\n", videoPath.c_str());
        return;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not find stream info\n");
        return;
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
        fprintf(stderr, "No video stream found\n");
        return;
    }

    AVStream* video_stream = fmt_ctx->streams[video_stream_index];
    AVCodecParameters* codecpar = video_stream->codecpar;

    // const AVCodec* decoder = nullptr;
    // if (codecpar->codec_id == AV_CODEC_ID_AV1) {
    //     // Prefer the software decoder from libdav1d
    //     decoder = avcodec_find_decoder_by_name("libdav1d");
    //     if (!decoder) {
    //         // Fallback to libaom or the native decoder
    //         decoder = avcodec_find_decoder_by_name("libaom-av1");
    //     }
    //     if (!decoder) {
    //         decoder = avcodec_find_decoder(codecpar->codec_id); // Native fallback
    //     }
    // } else {
    //     decoder = avcodec_find_decoder(codecpar->codec_id);
    // }

    // --- Find decoder ---
    const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
    if (!decoder) {
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not find decoder\n");
        return;
    }

    // --- Allocate decoder context ---
    AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
    if (!dec_ctx) {
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not allocate decoder context\n");
        return;
    }

    if (avcodec_parameters_to_context(dec_ctx, codecpar) < 0) {
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not copy codec parameters\n");
        return;
    }

    // --- Open decoder ---
    //dec_ctx->hwaccel = nullptr;  // Explicitly disable hardware acceleration

    // If you're using a dictionary to open the codec, add:
    // AVDictionary* opts = nullptr;
    // av_dict_set(&opts, "hwaccel", "none", 0);  // Disable hardware acceleratio


    if (ret = avcodec_open2(dec_ctx, decoder, nullptr); ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));

        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not open decoder: %s\n", errbuf);
        return;
    }

    // if (opts) {
    //     av_dict_free(&opts);
    // }

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
            fprintf(stderr, "Could not seek to timestamp\n");
            return;
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
        fprintf(stderr, "Could not allocate frame or packet\n");
        return;
    }

    // --- Read and decode until we get a frame ---
    bool got_frame = false;
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
                fprintf(stderr, "Error decoding frame\n");
                return;
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

        fprintf(stderr, "Could not decode any frame\n");
        return;
    }

    // --- Encode frame as JPEG ---
    // Find JPEG encoder
    const AVCodec* jpeg_encoder = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!jpeg_encoder) {
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);

        fprintf(stderr, "Could not find JPEG encoder\n");
        return;
    }

    // Allocate encoder context
    AVCodecContext* enc_ctx = avcodec_alloc_context3(jpeg_encoder);
    if (!enc_ctx) {
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not allocate encoder context\n");
        return;
    }

    // Set encoder parameters
    enc_ctx->width = frame->width;
    enc_ctx->height = frame->height;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    enc_ctx->time_base = (AVRational){1, 90000};
    
    // Set quality (q:v) - 2 is high quality
    av_dict_set_int((AVDictionary**)&enc_ctx->priv_data, "q", 2, 0);

    if (avcodec_open2(enc_ctx, jpeg_encoder, nullptr) < 0) {
        avcodec_free_context(&enc_ctx);
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not open JPEG encoder\n");
        return;
    }

    // Convert frame to YUVJ420P if needed
    AVFrame* jpeg_frame = av_frame_alloc();
    if (!jpeg_frame) {
        avcodec_free_context(&enc_ctx);
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not allocate JPEG frame\n");
        return;
    }

    jpeg_frame->width = frame->width;
    jpeg_frame->height = frame->height;
    jpeg_frame->format = AV_PIX_FMT_YUVJ420P;

    if (av_frame_get_buffer(jpeg_frame, 32) < 0) {
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&enc_ctx);
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not allocate JPEG frame buffer\n");
        return;
    }

#if 1
    // Convert if necessary
    SwsContext* sws_ctx = nullptr;
    if (frame->format != AV_PIX_FMT_YUVJ420P) {
        sws_ctx = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_YUVJ420P,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
        if (!sws_ctx) {
            av_frame_free(&jpeg_frame);
            avcodec_free_context(&enc_ctx);
            av_packet_free(&packet);
            av_frame_free(&frame);
            avcodec_free_context(&dec_ctx);
            avformat_close_input(&fmt_ctx);
            fprintf(stderr, "Could not create scale context\n");
            return;
        }
        
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                  jpeg_frame->data, jpeg_frame->linesize);
        sws_freeContext(sws_ctx);
    } else {
        // Copy frame if format already matches
        av_frame_copy(jpeg_frame, frame);
    }
#endif

    // Encode as JPEG
    std::vector<uint8_t> jpeg_data;
    if (!encodeJPEG(enc_ctx, jpeg_frame, jpeg_data)) {
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&enc_ctx);
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        fprintf(stderr, "Could not encode JPEG\n");
        return;
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
        fprintf(stderr, "Could not write output file\n");
        return;
    }

    // --- Cleanup ---
    av_frame_free(&jpeg_frame);
    avcodec_free_context(&enc_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);

}