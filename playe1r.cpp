/**
 * 实现音视频播放器
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */

#include "player.h"
#include "common.h"


int thread_exit = 0;
int thread_pause = 0;
int quit = 0;
PacketQueue audioq;

void packet_queue_init(PacketQueue *q) {
  memset(q, 0, sizeof(PacketQueue));
  q->mutex = SDL_CreateMutex();
  q->cond = SDL_CreateCond();
}

int audio_decode_frame(AVCodecContext *acodec_ctx, uint8_t *audio_buf,
                       int buf_size) {}

void audio_callback(void *userdata, Uint8 *stream, int len) {
  AVCodecContext *acodec_ctx = (AVCodecContext *)userdata;
  int len1, audio_size;
  static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
  static unsigned int audio_buf_size = 0;
  static unsigned int audio_buf_index = 0;

  while (len > 0) {
    audio_size = audio_decode_frame(acodec_ctx, audio_buf, sizeof(audio_buf));
    if (audio_buf_index >= audio_buf_size) {
      if (audio_size < 0) {
        audio_buf_size = 1024;
        memset(audio_buf, 0, audio_buf_size);
      } else {
        audio_buf_size = audio_size;
      }
      audio_buf_index = 0;
    }
    len1 = audio_buf_size - audio_buf_index;
    if(len1 > len) len1 = len;
    memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
    len -= len1;
    stream += len1;
    audio_buf_index += len1;
  }
}

int sfp_refresh_thread(void *args) {
  thread_exit = 0;
  thread_pause = 0;

  while (!thread_exit) {
    if (!thread_pause) {
      SDL_Event event;
      event.type = SFM_REFRESH_EVENT;
      SDL_PushEvent(&event);
    }
    SDL_Delay(40);
  }
  thread_exit = 0;
  thread_pause = 0;
  SDL_Event event;
  event.type = SFM_BREAK_EVENT;
  SDL_PushEvent(&event);
  return 0;
}

int main(int argc, char *argv[]) {
  

  av_register_all();
  avformat_network_init();
  pFormatCtx = avformat_alloc_context();

  if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
    printf("Couldn't open input stream.\n");
    goto __fail;
  }
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
    printf("Couldn't find stream information.\n");
    goto __fail;
  }
  for (i = 0; i < pFormatCtx->nb_streams; i++) {
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
        videoindex < 0) {
      videoindex = i;
    }
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
        audioindex < 0) {
      audioindex = i;
    }
  }
  if (videoindex == -1) {
    printf("Didn't find a video stream!\n");
    goto __fail;
  }
  if (audioindex == -1) {
    printf("Didn't find a auidio stream!\n");
    goto __fail;
  }

  // audio 解码转码
  aCodecCtx = pFormatCtx->streams[audioindex]->codec;
  aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
  if (aCodec == NULL) {
    printf("audio Codec not found.\n");
    goto __fail;
  }
  wanted_spec.freq = aCodecCtx->sample_rate;
  wanted_spec.format = AUDIO_S16SYS;
  wanted_spec.channels = aCodecCtx->channels;
  wanted_spec.silence = 0;
  wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
  wanted_spec.callback = audio_callback;
  wanted_spec.userdata = aCodecCtx;

  if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
    printf("fial to open audio device!\n");
    goto __fail;
  }
  avcodec_open2(aCodecCtx, aCodec, NULL);
  packet_queue_init(&audioq);

  in_channel_layout = av_get_default_channel_layout(aCodecCtx->channels);
  out_channel_layout = in_channel_layout;

  audio_convert_ctx = swr_alloc();
  if (audio_convert_ctx) {
    swr_alloc_set_opts(audio_convert_ctx, out_channel_layout, AV_SAMPLE_FMT_S16,
                       aCodecCtx->sample_rate, in_channel_layout,
                       aCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
  }
  swr_init(audio_convert_ctx);
  SDL_PauseAudio(0);

  // video 解码 rescale
  pCodecCtx = pFormatCtx->streams[videoindex]->codec;
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if (pCodec == NULL) {
    printf("video Codec not found.\n");
    goto __fail;
  }
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    printf("Could not open codec.\n");
    goto __fail;
  }

  pFrame = av_frame_alloc();
  pFrameYUV = av_frame_alloc();
  out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(
      pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1));
  av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                       pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                       1);

  packet = (AVPacket *)av_malloc(sizeof(AVPacket));
  // Output Info-----------------------------
  printf("--------------- File Information ----------------\n");
  av_dump_format(pFormatCtx, 0, filepath, 0);
  printf("-------------------------------------------------\n");
  img_convert_ctx = sws_getContext(
      pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 960, 720,
      AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

#if OUTPUT_YUV420P
  fp_yuv = fopen("output.yuv", "wb+");
#endif

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    printf("Could not initialize SDL - %s\n", SDL_GetError());
    goto __fail;
  }

  screen_w = pCodecCtx->width;
  screen_h = pCodecCtx->height;
  // SDL 2.0 Support for multiple windows
  screen = SDL_CreateWindow("Simplest ffmpeg player's Window",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            screen_w, screen_h,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!screen) {
    printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
    goto __fail;
  }

  sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
  if (!sdlRenderer) {
    printf("SDL: could not create render - exiting:%s\n", SDL_GetError());
    goto __fail;
  }

  // IYUV: Y + U + V  (3 planes)
  // YV12: Y + V + U  (3 planes)
  sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV,
                                 SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width,
                                 pCodecCtx->height);
  if (!sdlTexture) {
    printf("SDL: could not create texture - exiting:%s\n", SDL_GetError());
    goto __fail;
  }
  sdlRect[0].x = 0;
  sdlRect[0].y = 0;
  sdlRect[0].w = screen_w / 2;
  sdlRect[0].h = screen_h / 2;

  sdlRect[1].x = screen_w / 2;
  sdlRect[1].y = screen_h / 2;
  sdlRect[1].w = screen_w / 2;
  sdlRect[1].h = screen_h / 2;

  sdlRect[2].x = screen_w / 2;
  sdlRect[2].y = 0;
  sdlRect[2].w = screen_w / 2;
  sdlRect[2].h = screen_h / 2;

  sdlRect[3].x = 0;
  sdlRect[3].y = screen_h / 2;
  sdlRect[3].w = screen_w / 2;
  sdlRect[3].h = screen_h / 2;

  if (!SDL_RenderDrawRects(sdlRenderer, sdlRect, 4)) {
    printf("success!\n");
  }

  tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
  // SDL End----------------------
  while (1) {
    SDL_WaitEvent(&event);
    if (event.type == SFM_REFRESH_EVENT) {
      while (1) {
        if (av_read_frame(pFormatCtx, packet) < 0) {
          thread_exit = 1;
        }
        if (packet->stream_index == videoindex) {
          break;
        }
      }
      if (avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet) < 0) {
        printf("failde to decode!\n");
        goto __fail;
      }
      if (got_picture) {
        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
        SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0],
                          pFrameYUV->linesize[0]);
        SDL_RenderClear(sdlRenderer);

        SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect[0], &sdlRect[0]);
        SDL_RenderPresent(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect[0], &sdlRect[1]);
        SDL_RenderPresent(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect[0], &sdlRect[2]);
        SDL_RenderPresent(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect[0], &sdlRect[3]);
        SDL_RenderPresent(sdlRenderer);
      }
      av_free_packet(packet);
    } else if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_SPACE) {
        thread_pause = !thread_pause;
      } else if (event.key.keysym.sym == SDLK_ESCAPE) {
        thread_exit = 1;
      }
    } else if (event.type == SDL_QUIT) {
      thread_exit = 1;
    } else if (event.type == SFM_BREAK_EVENT) {
      break;
    }
  }

__fail:
  if (img_convert_ctx) {
    sws_freeContext(img_convert_ctx);
  }
  SDL_Quit();
  av_frame_free(&pFrameYUV);
  av_frame_free(&pFrame);
  avcodec_close(pCodecCtx);
  avformat_close_input(&pFormatCtx);

  return 0;
}