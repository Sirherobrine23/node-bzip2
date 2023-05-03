#include <napi.h>
#include <iostream>
extern "C" {
  #include <bzip2/bzlib_private.h>
  #include <bzip2/bzlib.h>
}

enum Sets {
  verbosity = 0,
  small = 1,
};

class Descompression {
  private:
    int bzCompressStatus, verbosity = 3, small;
    bz_stream strm;
  public:
  Descompression() {
    strm.bzalloc = NULL;
    strm.bzfree = NULL;
    strm.opaque = NULL;
  }
  void set(Sets target, int value) {
    if (target == Sets::verbosity) {
      if (!(value >= 1 && value <= 9)) value = 9;
      verbosity = value;
    } else if (target == Sets::small) {
      small = value;
    }
  }

  Napi::Value init(Napi::Env env) {
    std::cerr << "Init decompress, Verbosity: " << verbosity << ", Small: " << small << std::endl;
    bzCompressStatus = BZ2_bzDecompressInit(&strm, verbosity, small);
    if (bzCompressStatus != BZ_OK) {
      if (bzCompressStatus == BZ_CONFIG_ERROR) return Napi::Error::New(env, "the library has been mis-compiled").Value();
      else if (bzCompressStatus == BZ_PARAM_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzDecompressInit params").Value();
      else return Napi::Error::New(env, "decompress init not reached "+bzCompressStatus).Value();
    }
    strm.avail_in = 0;
    return env.Undefined();
  }

  bool process(Napi::Env env, Napi::Buffer<char*> chuck, Napi::Object This, Napi::Function Callback) {
    Callback.Call({});
  }

  bool end(Napi::Env env, Napi::Value error, Napi::Function Callback) {
    Callback.Call({error});
  }
};

Napi::Value Descompress(const Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  const Napi::Object TransformStream = info[0].ToObject();
  Descompression Descompress;
  /* Setup */
  if (info[1].IsObject()) {
    const Napi::Object Config = info[1].ToObject();
    if (Config.Get("verbosity").IsNumber()) Descompress.set(Sets::verbosity, Config.Get("verbosity").As<Napi::Number>().Int32Value());
    if (Config.Get("small").IsNumber()) Descompress.set(Sets::small, Config.Get("small").As<Napi::Number>().Int32Value());
  }

  const Napi::Value initStatus = Descompress.init(env);
  if (!(initStatus.IsUndefined())) {
    initStatus.As<Napi::Error>().ThrowAsJavaScriptException();
    return env.Undefined();
  }

  TransformStream.Set("_transform", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();
    const Napi::Buffer<char*> chuck = info[0].As<Napi::Buffer<char*>>();

    Descompress.process(info.Env(), chuck, This, Callback);
  }));

  TransformStream.Set("_destroy", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Value Error = info[0];
    const Napi::Function Callback = info[1].As<Napi::Function>();
    Descompress.end(info.Env(), Error, Callback);
  }));

}