#pragma once

#include "Singleton.h"
#include "common.h"

class DecodeVideo {
  PATTERN_SINGLETON_DECLARE(DecodeVideo);

 public:
  int Start();
  int Stop();

 private:
  int DecodeInitial();
  void ThreadVideoDecode();

 private:
  std::thread threadvideodecode_;
  std::atomic<bool> decodeRun_;
  VideoStateSptr videostate_ = std::make_shared<VideoState>();
  MediaStateSptr mediastate_;
};

#define g_DecodeVideo (*DecodeVideo::Instance())
