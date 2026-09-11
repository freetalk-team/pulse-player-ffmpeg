#include <napi.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include "ffmpeg.h"
#include "common.h"

Napi::Number to_js_number(Napi::Env& env, uint64_t val) {
    uint64_t safe_int = val & 0x1FFFFFFFFFFFFFULL;
    return Napi::Number::New(env, static_cast<double>(safe_int));
}

// Helper to safely get a metadata string or return an empty string if missing
std::string get_metadata_val(AVDictionary* metadata, const char* key) {
    AVDictionaryEntry* tag = av_dict_get(metadata, key, nullptr, 0);
    return tag ? tag->value : "";
}

std::string get_metadata(AVFormatContext* ctx, const char* key)
{
    // First try format-level metadata
    auto value = get_metadata_val(ctx->metadata, key);

    if (!value.empty())
        return value;

    // Then try stream-level metadata
    for (unsigned int i = 0; i < ctx->nb_streams; ++i) {
        if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {

            value = get_metadata_val(ctx->streams[i]->metadata, key);

            if (!value.empty())
                return value;
        }
    }

    return {};
}

// Helper to convert MD5 byte array to a hex string
std::string to_hex_string(const uint8_t* digest)
{
    static constexpr char hex[] = "0123456789abcdef";

    std::string result(32, '0');

    for (int i = 0; i < 16; ++i) {
        result[i * 2]     = hex[(digest[i] >> 4) & 0x0f];
        result[i * 2 + 1] = hex[digest[i] & 0x0f];
    }

    return result;
}

uintmax_t get_file_size(const std::string& filePath) {
    std::error_code ec;

    uintmax_t fileSize = std::filesystem::file_size(filePath, ec);

    if (ec) {
        fprintf(stderr, "Failed to get file size for %s: %s\n", filePath.c_str(), ec.message().c_str());
        fileSize = 0;
    }

    return fileSize;
}

Napi::Value ParseMetadata(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    //auto start = std::chrono::steady_clock::now();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String path expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string filePath = info[0].As<Napi::String>().Utf8Value();

    log("Processing file: %s", filePath.c_str());

    AVFormatContext* formatContext = nullptr;

    int ret = avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr);
    //if (avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr) < 0) {
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        logerr("avformat_open_input: %s", errbuf);

        Napi::Error::New(env, "Could not open file").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        logerr("avformat_find_stream_info: %s", errbuf);


        avformat_close_input(&formatContext);
        Napi::Error::New(env, "Could not find stream info").ThrowAsJavaScriptException();
        return env.Null();
    }

    log("Format: %s, mime=%s", formatContext->iformat->name, formatContext->iformat->mime_type);

    Metadata meta(formatContext);

    Napi::Object result = Napi::Object::New(env);
    Napi::Object metadata = meta.toJs(env);

    auto duration = formatContext->duration / (double)AV_TIME_BASE;
    auto bitrate = formatContext->bit_rate;

    log("Duration: %f", duration);
    log("Bitrate: %d", bitrate);

    result.Set("metadata", metadata);
    result.Set("duration", Napi::Number::New(env, duration));
    result.Set("bitrate", Napi::Number::New(env, bitrate));
    result.Set("size", Napi::Number::New(env, static_cast<double>(get_file_size(filePath))));

	log("Set metdata: done\n");	

    // --- Cover Art Extraction ---
    // bool coverFound = false;
    // for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
    //     AVStream* stream = formatContext->streams[i];
    //     if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
    //         AVPacket pkt = stream->attached_pic;
    //         if (pkt.size > 0 && pkt.data != nullptr) {
    //             // 1. Create the nested child object
    //             Napi::Object imgObj = Napi::Object::New(env);

    //             // 2. Sniff the magic bytes to determine the correct MIME type
    //             std::string sniffedMime = "image/jpeg"; // Default fallback
    //             if (pkt.size >= 4 && pkt.data[0] == 0x89 && pkt.data[1] == 0x50 && pkt.data[2] == 0x4E && pkt.data[3] == 0x47) {
    //                 sniffedMime = "image/png";
    //             }

    //             // 3. Populate properties into the sub-object
    //             imgObj.Set("mime", Napi::String::New(env, sniffedMime));
                
    //             Napi::Buffer<uint8_t> coverBuffer = Napi::Buffer<uint8_t>::Copy(env, pkt.data, pkt.size);
    //             imgObj.Set("buffer", coverBuffer);

    //             uint64_t imageHash = murmurhash3_64_update(HASH_SEED, pkt.data, pkt.size);

    //             imgObj.Set("hash", to_js_number(env, imageHash));

    //             // 4. Assign the structured object to the parent response
    //             metadata.Set("image", imgObj);
    //             coverFound = true;
    //             break;
    //         }
    //     }
    // }

    // Setup MD5 Hashing
    struct AVMD5* md5_ctx = av_md5_alloc();
    if (!md5_ctx) {
        avformat_close_input(&formatContext);
        Napi::Error::New(env, "Failed to allocate MD5 context").ThrowAsJavaScriptException();
        return env.Null();
    }
    av_md5_init(md5_ctx);

    AVPacket* packet = av_packet_alloc();
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->size > 0 && packet->data != nullptr) {
            av_md5_update(md5_ctx, packet->data, packet->size);
        }
        av_packet_unref(packet);
    }

    uint8_t digest[16];
    av_md5_final(md5_ctx, digest);
    av_free(md5_ctx);
    av_packet_free(&packet);

    avformat_close_input(&formatContext);

    result.Set("contentHash", Napi::String::New(env, to_hex_string(digest)));

    // auto end = std::chrono::steady_clock::now();
    // std::cout << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    return result;
}
