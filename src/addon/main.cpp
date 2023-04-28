#include <napi.h>
extern "C" {
  #include <bzip2/bzlib.h>
}

using namespace Napi;

Object Init(Env env, Object exports) {
  return exports;
}

NODE_API_MODULE(addon, Init);