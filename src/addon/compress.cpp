#include <napi.h>
#include <iostream>
extern "C" {
  #include <bzip2/bzlib_private.h>
  #include <bzip2/bzlib.h>
}

enum Sets {
  blockSize100k = 0,
  workFactor = 1,
  verbose = 2,
};

class Compression {
  private:
    int bzCompressStatus, blockSize100k = 9, verbosity = 0, workFactor = 0;
    bz_stream strm;
  public:
  Compression() {
    strm.bzalloc = NULL;
    strm.bzfree = NULL;
    strm.opaque = NULL;
  }

  void set(Sets target, int value) {
    if (target == Sets::blockSize100k) {
      if (!(value >= 1 && value <= 9)) value = 9;
      blockSize100k = value;
    } else if (target == Sets::verbose) {
      if (!(value >= 0 && value <= 6)) value = 0;
      verbosity = value;
    } else if (target == Sets::workFactor) {
      if (!(value >= 0 && value <= 250)) value = 0;
      workFactor = value;
    }
  }

  Napi::Value init(Napi::Env env) {
    std::cerr << "Init compress, level " << blockSize100k << ", verbosity " << verbosity << ", workFactor " << workFactor << std::endl;
    bzCompressStatus = BZ2_bzCompressInit(&strm, blockSize100k, verbosity, workFactor);
    if (bzCompressStatus != BZ_OK) {
      if (bzCompressStatus == BZ_CONFIG_ERROR) return Napi::Error::New(env, "the library has been mis-compiled").Value();
      else if (bzCompressStatus == BZ_PARAM_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value();
      else return Napi::Error::New(env, "Compress init not reached "+bzCompressStatus).Value();
    }
    strm.avail_in = 0;
    return env.Undefined();
  }

  bool process(Napi::Env env, Napi::Buffer<char*> chuck, Napi::Object This, Napi::Function Callback) {
    // Input
    strm.avail_in = chuck.ByteLength();
    strm.next_in = (char*)chuck.Data();

    // Out
    strm.avail_out = strm.avail_in + 16;
    char* bzBuffer = (char*)malloc(strm.avail_out);
    if (bzBuffer == NULL) return Callback.Call({Napi::Error::New(env, "malloc failed").Value()});
    strm.next_out = bzBuffer;

    // Compress
    bzCompressStatus = BZ2_bzCompress(&strm, BZ_RUN);
    if (bzCompressStatus != BZ_RUN_OK) {
      free(bzBuffer);
      if (bzCompressStatus == BZ_OK) return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress config in addon").Value()});
      else if (bzCompressStatus == BZ_CONFIG_ERROR) return Callback.Call({Napi::Error::New(env, "the library has been mis-compiled").Value()});
      else if (bzCompressStatus == BZ_PARAM_ERROR) return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value()});
      else if (bzCompressStatus == BZ_SEQUENCE_ERROR) return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").Value()});

      std::cout << "Error compressing, code: " << bzCompressStatus << std::endl;
      Callback.Call({Napi::Error::New(env, "Compress not reached").Value()});

      return false;
    }

    /* push Buffer to stream */
    This.Get("push").As<Napi::Function>().Call(This, {Napi::Buffer<char>::New(env, bzBuffer, strlen(bzBuffer))});

    // free memory
    free(bzBuffer);

    // Call callback to next Buffer
    Callback.Call({});

    return Napi::Boolean::New(env, true);
  }

  Napi::Value end(Napi::Env env, Napi::Value error, Napi::Function Callback) {
    if (!(error.IsUndefined() || error.IsNull())) {
      bzCompressStatus = BZ2_bzCompressEnd(&strm);
      if (bzCompressStatus != BZ_OK) {
        if (bzCompressStatus == BZ_CONFIG_ERROR) {
          return Callback.Call({Napi::Error::New(env, "the library has been mis-compiled").Value()});
        } else if (bzCompressStatus == BZ_PARAM_ERROR) {
          return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value()});
        } else if (bzCompressStatus == BZ_SEQUENCE_ERROR) {
          return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").Value()});
        } else {
          return Callback.Call({Napi::Error::New(env, "Compress end not reached").Value()});
        }
      }
      return Callback.Call({});
    }
    return Callback({error});
  }
};

Napi::Value Compress(const Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  const Napi::Object TransformStream = info[0].ToObject();
  Compression Compress;
  /* Setup */
  if (info[1].IsObject()) {
    const Napi::Object Config = info[1].ToObject();
    if (Config.Get("level").IsNumber()) Compress.set(Sets::blockSize100k, Config.Get("level").As<Napi::Number>().Int32Value());
    if (Config.Get("verbosity").IsNumber()) Compress.set(Sets::verbose, Config.Get("verbosity").As<Napi::Number>().Int32Value());
    if (Config.Get("workFactor").IsNumber()) Compress.set(Sets::workFactor, Config.Get("workFactor").As<Napi::Number>().Int32Value());
  }

  // Init bz_stream
  const Napi::Value initStatus = Compress.init(env);
  if (!(initStatus.IsUndefined())) {
    initStatus.As<Napi::Error>().ThrowAsJavaScriptException();
    return env.Undefined();
  }

  TransformStream.Set("_transform", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();
    const Napi::Buffer<char*> chuck = info[0].As<Napi::Buffer<char*>>();

    Compress.process(info.Env(), chuck, This, Callback);
  }));

  TransformStream.Set("_destroy", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Value Error = info[0];
    const Napi::Function Callback = info[1].As<Napi::Function>();
    Compress.end(info.Env(), Error, Callback);
  }));

  return TransformStream;
}