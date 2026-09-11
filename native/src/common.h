#pragma once


#include <napi.h>

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <cstdint>

extern "C" {

struct AVStream;
struct AVFormatContext;

}

std::vector<uint8_t> readFile(const std::string& path);

bool getImageDimensionsFFmpeg(const uint8_t* data, size_t size, int& width, int& height);
bool getDimensionsFromStream(AVStream* stream, int& width, int& height);

int  addCoverImageStream(AVFormatContext* ctx, const std::vector<uint8_t>& buf);
int  addCoverImageStream(AVStream* stream, const std::vector<uint8_t>& buf);
bool addCoverImage(AVFormatContext* ctx, int streamIndex, const std::vector<uint8_t>& buf);

struct CoverImage {

    enum Type {
        INVALID = -1,
        OTHER = 0,
        FILE_ICON = 1,
        OTHER_ICON = 2,
        FRONT_COVER = 3,
        BACK_COVER = 4,
        LEAFLET = 5,
        MEDIA = 6,
        LEAD_ARTIST = 7,
        ARTIST = 8,
        CONDUCTOR = 9,
        BAND = 10,
        COMPOSER = 11,
        LYRICIST = 12,
        RECORDING_LOCATION = 13,
        DURING_RECORDING = 14,
        DURING_PERFORMANCE = 15,
        VIDEO_CAPTURE = 16,
        FISH = 17,
        ILLUSTRATION = 18,
        BAND_LOGO = 19,
        PUBLISHER_LOGO = 20
    };

    std::string mime;
    std::vector<uint8_t> data;
    uint64_t hash;

    bool empty() const { return data.empty(); }

    std::string encode(Type = FRONT_COVER) const;
    Type        decode(const std::string&);

    void load(const std::string& path);
    void load(const uint8_t* data, size_t size, const std::string& mime = {});

    int  addStream(AVFormatContext*) const;
    bool addToStream(AVFormatContext*, int streamIndex) const;

    static std::string mimeFromPath(const std::string& path);
};

struct Metadata : std::multimap<std::string, std::string> {

    CoverImage cover;

    Metadata() = default;
    Metadata(AVFormatContext*);

    bool parse(AVFormatContext*);

    Napi::Object toJs(Napi::Env env) const;
};