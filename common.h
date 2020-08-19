#pragma once
#include <cstdio>
#include <iostream>
#include <atomic>
#include <set>
#include <string>
#include <thread>
#include <memory>

#include "Singleton.h"
#include "Types.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libsdl2/SDL.h"
#include "libsdl2/SDL_audio.h"
#include "libsdl2/SDL_events.h"
#include "libsdl2/SDL_render.h"
#include "libsdl2/SDL_video.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
};

#define ALG_FAIL -1
#define ALG_SUCCESS 0

typedef struct PacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  SDL_mutex *mutex;
  SDL_cond *cond;
} PacketQueue;

struct MediaState {
  MediaState() {}
  ~MediaState() {
    if (pFormatCtx) {
      avformat_close_input(&pFormatCtx);
    }
  }
  AVFormatContext *pFormatCtx{nullptr};
};
using MediaStateUptr = std::unique_ptr<MediaState>;
using MediaStateSptr = std::shared_ptr<MediaState>;

struct VideoState {
  VideoState() {}
  ~VideoState() {
    if (pCodecCtx) {
      avcodec_close(pCodecCtx);
    }
    if (pFrame) {
      av_frame_free(&pFrame);
    }
    if (pFrameYUV) {
      av_frame_free(&pFrameYUV);
    }
    if (packet) {
      av_packet_free(&packet);
    }
    if (img_convert_ctx) {
      sws_freeContext(img_convert_ctx);
    }
    if (out_buffer) {
      av_freep(&out_buffer);
    }
  }
  std::string filename;
  int videoindex{-1};
  AVCodecContext *pCodecCtx{nullptr};
  AVCodec *pCodec{nullptr};
  AVFrame *pFrame{nullptr};
  AVFrame *pFrameYUV{nullptr};
  unsigned char *out_buffer{nullptr};
  AVPacket *packet{nullptr};
  struct SwsContext *img_convert_ctx{nullptr};
};
using VideoStateUptr = std::unique_ptr<VideoState>;
using VideoStateSptr = std::shared_ptr<VideoState>;

struct AudioState {
  AudioState() {}
  ~AudioState() {
    if (aCodecCtx) {
      avcodec_close(aCodecCtx);
    }
    if (audio_convert_ctx) {
      swr_close(audio_convert_ctx);
    }
  }
  AVCodecContext *aCodecCtx{nullptr};
  AVCodec *aCodec{nullptr};
  struct SwrContext *audio_convert_ctx{nullptr};

  int audioindex{-1};
  int64_t in_channel_layout{0};
  int64_t out_channel_layout{0};
};
using AudioStateUptr = std::unique_ptr<AudioState>;
using AudioStateSptr = std::shared_ptr<AudioState>;

struct SdlState {
  SdlState() {}
  ~SdlState() {}
  int screen_w{0};
  int screen_h{0};
  SDL_Window *screen{nullptr};
  SDL_Renderer *sdlRenderer{nullptr};
  SDL_Texture *sdlTexture{nullptr};
  SDL_Thread *tid{nullptr};
};
using SdlStateUptr = std::unique_ptr<SdlState>;
using SdlStateSptr = std::shared_ptr<SdlState>;


