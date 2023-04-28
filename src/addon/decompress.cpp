#include <napi.h>
extern "C" {
  #include <bzip2/bzlib.h>
}

Napi::Value Descompress(Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  return Napi::String::New(env, "from Addon");
}