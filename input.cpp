#include "input.h"

PATTERN_SINGLETON_IMPLEMENT(InputMedia)

InputMedia::InputMedia() {}
InputMedia::~InputMedia() {}

int InputMedia::Initialize(const char*filepath) {

  av_register_all();
  avformat_network_init();
  mediastate_->pFormatCtx = avformat_alloc_context();

  if (avformat_open_input(&mediastate_->pFormatCtx, filepath, NULL, NULL) !=
      0) {
    printf("Couldn't open input stream.\n");
    return ALG_FAIL;
  }
  if (avformat_find_stream_info(mediastate_->pFormatCtx, NULL) < 0) {
    printf("Couldn't find stream information.\n");
    return ALG_FAIL;
  }
  av_dump_format(mediastate_->pFormatCtx, 0, filepath, 0);

  printf("nb %d\n", mediastate_->pFormatCtx->nb_streams);
  return ALG_SUCCESS;
}
