#include <napi.h>
#include "compressor/sync.cpp"
// #include "compressor/async.cpp"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("sync", Napi::Function::New(env, SyncCompress));
  // exports.Set("async", Napi::Function::New(env, CallAsyncCompress));
  return exports;
}
NODE_API_MODULE(node_bzlib, Init)