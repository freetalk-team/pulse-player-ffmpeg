#include "common.h"
#include "ffmpeg.h"
#include "hash.h"

#include <filesystem>

#define HASH_SEED 5381

extern "C" {

#include <libavutil/base64.h>

}

namespace fs = std::filesystem;

std::vector<uint8_t> readFile(const std::string& path) {

    std::vector<uint8_t> buf;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        if (size > 0) {
            buf.resize(size);

            file.read(reinterpret_cast<char*>(buf.data()), size);

            if (file.gcount() != size) {
                buf.clear();
            }

        }

        file.close();
    }

    return buf;
}

static std::string getMime(const std::vector<uint8_t>& data) {
    if (data.size() < 4)
        return {};

    if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47)
        return "image/png";

    if (data[0] == 0xFF && data[1] == 0xD8)
        return "image/jpeg";

    // if (data[0] == 0x42 && data[1] == 0x4D) 
    //     return "image/bmp";

    return {};
}

bool getImageDimensionsFFmpeg(const uint8_t* data, size_t size, int& width, int& height) {
    if (!data || size == 0) return false;
    
    // Find the appropriate decoder
    AVCodecID codec_id = AV_CODEC_ID_NONE;
    
    // Detect codec from signature
    if (size >= 4) {
        if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
            codec_id = AV_CODEC_ID_PNG;
        } else if (data[0] == 0xFF && data[1] == 0xD8) {
            codec_id = AV_CODEC_ID_MJPEG;
        } else if (data[0] == 0x42 && data[1] == 0x4D) {
            codec_id = AV_CODEC_ID_BMP;
        } else if (size >= 6 && data[0] == 0x47 && data[1] == 0x49 && 
                   data[2] == 0x46 && data[3] == 0x38) {
            codec_id = AV_CODEC_ID_GIF;
        }
    }
    
    if (codec_id == AV_CODEC_ID_NONE) {
        return false;
    }
    
    // Find decoder
    const AVCodec* codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        return false;
    }
    
    // Allocate codec context
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        return false;
    }
    
    // Open codec
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        avcodec_free_context(&codec_ctx);
        return false;
    }
    
    // Create packet with the image data
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        avcodec_free_context(&codec_ctx);
        return false;
    }
    
    packet->data = const_cast<uint8_t*>(data);
    packet->size = size;
    
    // Send the packet to the decoder
    int ret = avcodec_send_packet(codec_ctx, packet);
    if (ret < 0) {
        av_packet_free(&packet);
        avcodec_free_context(&codec_ctx);
        return false;
    }
    
    // Try to receive a frame to get dimensions
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        av_packet_free(&packet);
        avcodec_free_context(&codec_ctx);
        return false;
    }
    
    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret >= 0) {
        // Got a frame, get dimensions
        width = frame->width;
        height = frame->height;
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&codec_ctx);
        return (width > 0 && height > 0);
    }
    
    // If we didn't get a frame, try to get dimensions from the codec context
    if (codec_ctx->width > 0 && codec_ctx->height > 0) {
        width = codec_ctx->width;
        height = codec_ctx->height;
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&codec_ctx);
        return true;
    }
    
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    return false;
}


bool getDimensionsFromStream(AVStream* stream, int& width, int& height) {
    if (!stream) return false;
    
    // Check if we already have dimensions in codecpar
    if (stream->codecpar->width > 0 && stream->codecpar->height > 0) {
        width = stream->codecpar->width;
        height = stream->codecpar->height;
        return true;
    }
    
    // Try to get from attached picture data
    if (stream->attached_pic.data && stream->attached_pic.size > 0) {
        return getImageDimensionsFFmpeg(stream->attached_pic.data, 
                                        stream->attached_pic.size, 
                                        width, height);
    }
    
    return false;
}

int addCoverImageStream(AVFormatContext* ctx, const std::vector<uint8_t>& buf) {

    AVStream* stream = avformat_new_stream(ctx, nullptr);
    if (!stream) {
        return -1;
    }
    
    int streamIndex = stream->index;
    
    // Initialize codec parameters
    AVCodecParameters* codecpar = stream->codecpar;
    codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    codecpar->codec_id = AV_CODEC_ID_MJPEG; // Default, will be detected
    
    // Try to get actual dimensions using FFmpeg
    int width = 0, height = 0;

    if (getImageDimensionsFFmpeg(buf.data(), buf.size(), width, height)) {
        codecpar->width = width;
        codecpar->height = height;
        // fprintf(stderr, "Detected new cover dimensions: %dx%d\n", width, height);
    } else {
        // Use reasonable defaults
        fprintf(stderr, "Warning: Could not detect new cover dimensions, using defaults\n");
        codecpar->width = 500;
        codecpar->height = 500;
    }
    
    // Set pixel format
    codecpar->format = AV_PIX_FMT_YUVJ420P;
    codecpar->extradata = nullptr;
    codecpar->extradata_size = 0;
    
    // Set disposition
    stream->disposition = AV_DISPOSITION_ATTACHED_PIC;
    stream->time_base = (AVRational){1, 90000};
    
    // Allocate and set attached picture packet
    int ret = av_new_packet(&stream->attached_pic, buf.size());
    if (ret < 0) {
        return -1;
    }
    
    memcpy(stream->attached_pic.data, buf.data(), buf.size());
    stream->attached_pic.size = buf.size();
    stream->attached_pic.stream_index = stream->index;
    stream->attached_pic.flags |= AV_PKT_FLAG_KEY;
    stream->attached_pic.pts = AV_NOPTS_VALUE;
    stream->attached_pic.dts = AV_NOPTS_VALUE;
    stream->attached_pic.duration = 0;
    
    av_dict_set(&stream->metadata, "title", "Album cover", 0);
    av_dict_set(&stream->metadata, "comment", "Cover (front)", 0);

    return streamIndex;
}

bool addCoverImage(AVFormatContext* ctx, int streamIndex, const std::vector<uint8_t>& buf) {

    AVPacket* packet = av_packet_alloc();
    if (!packet) return false;

    int ret = av_new_packet(packet, buf.size());
    if (ret == 0) {
        memcpy(packet->data, buf.data(), buf.size());
        packet->stream_index = streamIndex;
        packet->flags |= AV_PKT_FLAG_KEY;
        packet->pts = AV_NOPTS_VALUE;
        packet->dts = AV_NOPTS_VALUE;
        packet->duration = 0;
        
        // Write the cover packet
        ret = av_interleaved_write_frame(ctx, packet);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            fprintf(stderr, "Error writing cover packet: %s\n", errbuf);

            av_packet_free(&packet);
            return false;
        }
    }

    av_packet_free(&packet);
    return true;
}

static std::string getImageMime(const std::vector<uint8_t>& image)
{
    // PNG signature
    if (image.size() >= 8 &&
        image[0] == 0x89 &&
        image[1] == 0x50 &&
        image[2] == 0x4e &&
        image[3] == 0x47 &&
        image[4] == 0x0d &&
        image[5] == 0x0a &&
        image[6] == 0x1a &&
        image[7] == 0x0a)
    {
        return "image/png";
    }

    // JPEG signature
    if (image.size() >= 3 &&
        image[0] == 0xff &&
        image[1] == 0xd8 &&
        image[2] == 0xff)
    {
        return "image/jpeg";
    }

    return {};
}

static void appendBE32(std::vector<uint8_t>& out, uint32_t value)
{
    out.push_back((value >> 24) & 0xff);
    out.push_back((value >> 16) & 0xff);
    out.push_back((value >> 8) & 0xff);
    out.push_back(value & 0xff);
}

static std::string makeMetadataBlockPicture(
    const std::vector<uint8_t>& image,
    const std::string& mime,
    int width,
    int height,
    CoverImage::Type type = CoverImage::FRONT_COVER)   
{
    std::vector<uint8_t> picture;

    // Picture type:
    // 3 = Cover (front)
    appendBE32(picture, type);

    // MIME type
    appendBE32(picture, static_cast<uint32_t>(mime.size()));
    picture.insert(
        picture.end(),
        mime.begin(),
        mime.end()
    );

    // Description
    const std::string description = "Cover (front)";

    appendBE32(
        picture,
        static_cast<uint32_t>(description.size())
    );

    picture.insert(
        picture.end(),
        description.begin(),
        description.end()
    );

    // Width
    appendBE32(picture, static_cast<uint32_t>(width));

    // Height
    appendBE32(picture, static_cast<uint32_t>(height));

    // Color depth.
    // 0 = unknown.
    appendBE32(picture, 0);

    // Number of colors.
    // 0 for non-indexed images / unknown.
    appendBE32(picture, 0);

    // Image data length
    appendBE32(
        picture,
        static_cast<uint32_t>(image.size())
    );

    // Image data
    picture.insert(
        picture.end(),
        image.begin(),
        image.end()
    );

    // Base64 encode
    std::vector<char> encoded(AV_BASE64_SIZE(picture.size()));

    if (!av_base64_encode(
            encoded.data(),
            static_cast<int>(encoded.size()),
            picture.data(),
            static_cast<int>(picture.size())))
    {
        return {};
    }

    return std::string(encoded.data());
}

int addCoverImageStream(AVStream* stream, const std::vector<uint8_t>& buf) {
    std::string mime = getImageMime(buf);

    if (mime.empty()) {
        // throw std::runtime_error("Unsupported cover image format");
        return -1;
    }

    int width = 0;
    int height = 0;

    if (!getImageDimensionsFFmpeg(
            buf.data(),
            buf.size(),
            width,
            height))
    {
        //throw std::runtime_error("Could not determine cover image dimensions");
        return -1;
    }

    std::string picture = makeMetadataBlockPicture(
        buf,
        mime,
        width,
        height
    );

    if (picture.empty()) {
        //throw std::runtime_error("Could not create METADATA_BLOCK_PICTURE");
        return -1;
    }

    av_dict_set(
        &stream->metadata,
        "METADATA_BLOCK_PICTURE",
        picture.c_str(),
        0
    );

    return -1;
}

static bool extractCover(
    AVFormatContext* ctx,
    std::vector<uint8_t>& data,
    std::string& mime)
{
    for (unsigned i = 0; i < ctx->nb_streams; ++i) {
        AVStream* stream = ctx->streams[i];

        if (!(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
            continue;

        const AVPacket& pkt = stream->attached_pic;

        if (!pkt.data || pkt.size <= 0)
            continue;

        data.assign(pkt.data, pkt.data + pkt.size);

        switch (stream->codecpar->codec_id) {
            case AV_CODEC_ID_MJPEG:
                mime = "image/jpeg";
                break;

            case AV_CODEC_ID_PNG:
                mime = "image/png";
                break;

            case AV_CODEC_ID_WEBP:
                mime = "image/webp";
                break;

            default:
                mime.clear();
                break;
        }

        return true;
    }

    return false;
}

static uint32_t readBE32(const uint8_t*& p)
{
    uint32_t value =
        (static_cast<uint32_t>(p[0]) << 24) |
        (static_cast<uint32_t>(p[1]) << 16) |
        (static_cast<uint32_t>(p[2]) << 8)  |
        static_cast<uint32_t>(p[3]);

    p += 4;
    return value;
}

static CoverImage::Type extractCoverFromBase64(
    const std::string& base64,
    CoverImage& cover)
{

    // Calculate decoded size
    const int decodedSize = AV_BASE64_DECODE_SIZE(base64.size());

    std::vector<uint8_t> decoded(decodedSize);

    const int size = av_base64_decode(
        decoded.data(),
        base64.data(),
        decodedSize
    );

    if (size <= 0)
        return CoverImage::INVALID;

    const uint8_t* p = decoded.data();
    const uint8_t* end = p + size;

    // picture type
    if (end - p < 4)
        return CoverImage::INVALID;

    auto pictureType = readBE32(p);

    // MIME type
    if (end - p < 4)
        return CoverImage::INVALID;

    uint32_t mimeLen = readBE32(p);

    if (mimeLen > static_cast<uint32_t>(end - p))
        return CoverImage::INVALID;

    cover.mime.assign(
        reinterpret_cast<const char*>(p),
        mimeLen
    );

    p += mimeLen;

    // Description
    if (end - p < 4)
        return CoverImage::INVALID;

    uint32_t descriptionLen = readBE32(p);

    if (descriptionLen > static_cast<uint32_t>(end - p))
        return CoverImage::INVALID;

    p += descriptionLen;

    // Width, height, color depth, colors
    if (end - p < 16)
        return CoverImage::INVALID;

    uint32_t width       = readBE32(p);
    uint32_t height      = readBE32(p);
    uint32_t colorDepth  = readBE32(p);
    uint32_t colors      = readBE32(p);

    //(void)pictureType;
    (void)width;
    (void)height;
    (void)colorDepth;
    (void)colors;

    // Image data
    if (end - p < 4)
        return CoverImage::INVALID;

    uint32_t imageSize = readBE32(p);

    if (imageSize > static_cast<uint32_t>(end - p))
        return CoverImage::INVALID;

    cover.data.assign(p, p + imageSize);

    return (CoverImage::Type) pictureType;
}

template <typename T>
static void addTag(T& meta, AVDictionaryEntry* tag) {
    std::string key = tag->key;
    std::transform(
        key.begin(),
        key.end(),
        key.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    meta.emplace(key, tag->value);
}

static void parseTags(AVFormatContext* ctx, Metadata& meta) {
    AVDictionaryEntry* tag = nullptr;
    CoverImage::Type pictureType = CoverImage::INVALID;

    while ((tag = av_dict_get(ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) 
        addTag(meta, tag);

    for (unsigned i = 0; i < ctx->nb_streams; ++i) {
        tag = nullptr;

        auto stream = ctx->streams[i];

        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            auto pkt = &stream->attached_pic;

            if (pkt->data && pkt->size > 0) {
                // pkt->data / pkt->size = image data

                meta.cover.data.assign(pkt->data, pkt->data + pkt->size);
            }

            auto codecpar = stream->codecpar;

            const char* mime = nullptr;

            switch (codecpar->codec_id) {
                case AV_CODEC_ID_MJPEG:
                meta.cover.mime = "image/jpeg";
                break;

                case AV_CODEC_ID_PNG:
                meta.cover.mime = "image/png";
                break;

                case AV_CODEC_ID_WEBP:
                meta.cover.mime = "image/webp";
                break;
            }

            continue;
        }

        while ((tag = av_dict_get(
            ctx->streams[i]->metadata,
            "",
            tag,
            AV_DICT_IGNORE_SUFFIX
        ))) {
            if (strcmp(tag->key, "METADATA_BLOCK_PICTURE") == 0) {

                if (meta.cover.data.empty() || pictureType != CoverImage::FRONT_COVER)
                    pictureType = meta.cover.decode(tag->value);
            }
            else {
                addTag(meta, tag);
            }
        }
    }

    if (meta.cover.data.size() > 0) {
        meta.cover.hash = murmurhash3_64_update(HASH_SEED, meta.cover.data.data(), meta.cover.data.size());
    }
}

Metadata::Metadata(AVFormatContext* ctx) {
    parseTags(ctx, *this);
}

bool Metadata::parse(AVFormatContext* ctx) {
    if (avformat_find_stream_info(ctx, nullptr) < 0)
        return false;

    parseTags(ctx, *this);

    return true;
}

Napi::Object Metadata::toJs(Napi::Env env) const {
    Napi::Object obj = Napi::Object::New(env);

    auto it = begin();

    while (it != end()) {
        const std::string& key = it->first;

        auto range = equal_range(key);

        // Count values
        size_t count = std::distance(range.first, range.second);

        if (count == 1) {
            obj.Set(
                key,
                Napi::String::New(env, range.first->second)
            );
        } else {
            Napi::Array values = Napi::Array::New(env, count);

            size_t index = 0;
            for (auto valueIt = range.first; valueIt != range.second; ++valueIt) {
                values.Set(
                    index++,
                    Napi::String::New(env, valueIt->second)
                );
            }

            obj.Set(key, values);
        }

        it = range.second;
    }

    if (cover.data.size() > 0) {
        Napi::Object image = Napi::Object::New(env);

        image.Set("mime", Napi::String::New(env, cover.mime));
        image.Set("data", Napi::Buffer<uint8_t>::Copy(env, cover.data.data(), cover.data.size()));
        image.Set("hash", Napi::Number::New(env, static_cast<double>(cover.hash & 0x1FFFFFFFFFFFFFULL)));

        obj.Set("cover", image);
    }

    return obj;
}

std::string CoverImage::encode(CoverImage::Type type) const {
    // std::string mime = getImageMime(data);
    // if (mime.empty())
    //     return {};

    int width = 0;
    int height = 0;

    if (!getImageDimensionsFFmpeg(
            data.data(),
            data.size(),
            width,
            height))
    {
        return {};
    }

    return makeMetadataBlockPicture(
        data,
        mime,
        width,
        height,
        type
    );
}


CoverImage::Type CoverImage::decode(const std::string& base64) {
    return extractCoverFromBase64(base64, *this);
}

void CoverImage::load(const std::string& path) {
    mime = CoverImage::mimeFromPath(path);
    if (!mime.empty())
        data = readFile(path);
}

void CoverImage::load(const uint8_t* data, size_t size, const std::string& mime) {
    this->data.assign(data, data + size);
    this->mime = mime.empty() ? getMime(this->data) : mime;
}

std::string CoverImage::mimeFromPath(const std::string& path) {
    std::filesystem::path p(path);
    std::string ext = p.extension().string();

    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        });

    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";

    return {};
}

int CoverImage::addStream(AVFormatContext* ctx) const {
    return addCoverImageStream(ctx, data);
}

bool CoverImage::addToStream(AVFormatContext* ctx, int streamIndex) const {
    return addCoverImage(ctx, streamIndex, data);
}
