#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cmath>

extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/url.h>  
#include <libavformat/demux.h>  
#include <libavcodec/avcodec.h>
#include <libavutil/md5.h>
#include <libavutil/mem.h>
#include <libavutil/dict.h>
}

#define TRACE 0
#define INT_HASH 0 // 0, 32, 64
#define HASH_SEED 5381
#define MURMUR_HASH 1

#if TRACE

#define log(fmt, ...) printf("# " fmt "\n", __VA_ARGS__)

static void log_callback(void *, int level, const char *fmt, va_list vl)
{
    vfprintf(stderr, fmt, vl);
}

#else
#define log(...)
#endif

void parse(const std::string& inputPath);

int main(int argc, const char* argv[]) {

    std::string inputPath(argv[1]);

    parse(inputPath);

    return 0;
}




// Helper to safely get a metadata string or return an empty string if missing
std::string get_metadata_val(AVDictionary* metadata, const char* key) {
    AVDictionaryEntry* tag = av_dict_get(metadata, key, nullptr, 0);
    return tag ? tag->value : "";
}

// Helper to convert MD5 byte array to a hex string
std::string to_hex_string(const uint8_t* digest) {
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}


void parse(const std::string& filePath) {

#if TRACE
    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(log_callback);
#endif


    log("Processing file: %s", filePath.c_str());

    AVFormatContext* formatContext = nullptr;

    int ret = avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr);
    //if (avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr) < 0) {
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));

        fprintf(stderr, "Could not open file: %s\n", errbuf);
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        fprintf(stderr, "Could not find stream info\n");
        return;
    }

#if TRACE
    dump_stream_info(formatContext);
#endif


    auto title = get_metadata_val(formatContext->metadata, "title");
    auto artist = get_metadata_val(formatContext->metadata, "artist");
    auto album = get_metadata_val(formatContext->metadata, "album");
    auto genre = get_metadata_val(formatContext->metadata, "genre");

    printf("title => %s\n", title.c_str());
    printf("artist => %s\n", artist.c_str());
    printf("album => %s\n", album.c_str());
    printf("genre => %s\n", genre.c_str());


  

    // 2. Extract ALL raw/custom metadata tags dynamically
    // Napi::Object nativeTags = Napi::Object::New(env);
    // AVDictionaryEntry* tag = nullptr;
    // while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
    //     nativeTags.Set(tag->key, tag->value);
    // }
    // metadata.Set("native", nativeTags);

    auto duration = formatContext->duration / (double)AV_TIME_BASE;
    auto bitrate = formatContext->bit_rate;

    log("Duration: %f", duration);
    log("Bitrate: %d", bitrate);

   

    // --- Cover Art Extraction ---
    bool coverFound = false;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        AVStream* stream = formatContext->streams[i];
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            AVPacket pkt = stream->attached_pic;
            if (pkt.size > 0 && pkt.data != nullptr) {
                // 1. Create the nested child object

                // 2. Sniff the magic bytes to determine the correct MIME type
                std::string sniffedMime = "image/jpeg"; // Default fallback
                if (pkt.size >= 4 && pkt.data[0] == 0x89 && pkt.data[1] == 0x50 && pkt.data[2] == 0x4E && pkt.data[3] == 0x47) {
                    sniffedMime = "image/png";
                }

                printf("cover => %s, size=%d\n", sniffedMime.c_str(), pkt.size);

                coverFound = true;
                break;
            }
        }
    }
    // if (!coverFound) {
    //     result.Set("image", env.Null());
    // }

    // Setup MD5 Hashing
    struct AVMD5* md5_ctx = av_md5_alloc();
    if (!md5_ctx) {
        avformat_close_input(&formatContext);
        fprintf(stderr, "Failed to allocate MD5 context\n");
        return ;
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
}
