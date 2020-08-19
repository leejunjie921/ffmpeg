#include "input.h"
#include "decode_video.h"

#undef main
int main() {

  g_InputMedia.Initialize("1.mp4");
  g_DecodeVideo.Start();
  getchar();
  g_DecodeVideo.Stop();
  return 0;
}