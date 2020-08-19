#pragma once
#include "Singleton.h"
#include "common.h"

class InputMedia {
  PATTERN_SINGLETON_DECLARE(InputMedia);

 public:
  int Initialize(const char* filepath);
  MediaStateSptr& GetMediaState() { return mediastate_; }

 private:
  MediaStateSptr mediastate_ = std::make_shared<MediaState>();
 
};
#define g_InputMedia (*InputMedia::Instance())

//class JJPlayer {
//  PATTERN_SINGLETON_DECLARE(JJPlayer);
//
// public:
//  //int Start();
//  //int Stop();
//
// private:
//  PacketQueue audioq;
//  struct SwrContext *audio_convert_ctx{nullptr};
//  std::atomic<bool> thread_exit{false};
//  std::atomic<bool> thread_pause{false};
//  std::atomic<bool> quit{false};
//};
//
//#define g_JJPlayer (*JJPlayer::Instance())
