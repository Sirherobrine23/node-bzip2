#include <napi.h>
#include <compress.cpp>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("Compress", Napi::Function::New(env, Compress));
  return exports;
}

NODE_API_MODULE(addon, Init);