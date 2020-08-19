#include "decode_video.h"

#include "input.h"

PATTERN_SINGLETON_IMPLEMENT(DecodeVideo)

DecodeVideo::DecodeVideo() : decodeRun_(false) {}
DecodeVideo::~DecodeVideo() {}

int DecodeVideo::Start() {
  DecodeInitial();
  threadvideodecode_ = std::thread(&DecodeVideo::ThreadVideoDecode, this);
  printf("start success!\n");
  return ALG_SUCCESS;
}

int DecodeVideo::Stop() {
  decodeRun_ = false;
  if (threadvideodecode_.joinable()) {
    threadvideodecode_.join();
  }
  printf("stop success!\n");
  return ALG_SUCCESS;
}

int DecodeVideo::DecodeInitial() {
  auto mediastate = g_InputMedia.GetMediaState();
  videostate_->videoindex = av_find_best_stream(
      mediastate->pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  printf("index %d\n", videostate_->videoindex);
  if (videostate_->videoindex == -1) {
    printf("fail to find best video stream!\n");
    return ALG_FAIL;
  }
  videostate_->pCodecCtx =
      mediastate->pFormatCtx->streams[videostate_->videoindex]->codec;

  videostate_->pCodec = avcodec_find_decoder(videostate_->pCodecCtx->codec_id);
  if (videostate_->pCodec == NULL) {
    printf("video Codec not found.\n");
    return ALG_FAIL;
  }
  if (avcodec_open2(videostate_->pCodecCtx, videostate_->pCodec, NULL) < 0) {
    printf("Could not open codec.\n");
    return ALG_FAIL;
  }
  printf("decode %d\n", videostate_->pCodecCtx->codec_id);

  videostate_->pFrame = av_frame_alloc();
  videostate_->pFrameYUV = av_frame_alloc();
  videostate_->out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(
      videostate_->pCodecCtx->pix_fmt, videostate_->pCodecCtx->width,
      videostate_->pCodecCtx->height, 1));

  av_image_fill_arrays(
      videostate_->pFrameYUV->data, videostate_->pFrameYUV->linesize,
      videostate_->out_buffer, videostate_->pCodecCtx->pix_fmt,
      videostate_->pCodecCtx->width, videostate_->pCodecCtx->height, 1);

  videostate_->packet = (AVPacket *)av_malloc(sizeof(AVPacket));

  videostate_->img_convert_ctx = sws_getContext(
      videostate_->pCodecCtx->width, videostate_->pCodecCtx->height,
      videostate_->pCodecCtx->pix_fmt, 960, 720, AV_PIX_FMT_YUV420P,
      SWS_BICUBIC, NULL, NULL, NULL);
  mediastate_ = mediastate;
  decodeRun_ = true;
  return ALG_SUCCESS;
}

void DecodeVideo::ThreadVideoDecode() {
  while (decodeRun_) {
    while (av_read_frame(mediastate_->pFormatCtx, videostate_->packet) >= 0) {
      if (videostate_->videoindex == videostate_->packet->stream_index) {
        // printf("index %d\n", videostate_->videoindex);
      }
      av_packet_unref(videostate_->packet);
    }
  }
}
