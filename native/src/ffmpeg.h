#pragma once

extern "C" {

#include <libavformat/avformat.h>
#include <libavformat/url.h>  
#include <libavformat/demux.h>  
#include <libavcodec/avcodec.h>
#include <libavcodec/bsf.h>
#include <libavutil/md5.h>
#include <libavutil/mem.h>
#include <libavutil/dict.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/avassert.h>

}

#ifdef ADDON_TRACE

//#define log(fmt, ...) printf("# " fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define log(fmt, ...) printf("# " fmt "\n", ##__VA_ARGS__)
#define logerr(fmt, ...) fprintf(stderr, "# " fmt "\n", ##__VA_ARGS__)

#else
#define log(...)
#define logerr(...)
#endif

void init_ffmpeg();
