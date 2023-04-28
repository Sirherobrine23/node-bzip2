#include <napi.h>
#include <compress.cpp>
#include <decompress.cpp>
using namespace Napi;

Object Init(Env env, Object exports) {
  exports.Set("Compress", Function::New(env, Compress));
  exports.Set("Descompress", Function::New(env, Descompress));
  return exports;
}

NODE_API_MODULE(addon, Init);