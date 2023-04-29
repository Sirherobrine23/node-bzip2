#include <napi.h>
#include <iostream>
extern "C" {
  #include <bzip2/bzlib.h>
  #include <bzip2/bzlib_private.h>
}

using namespace std;

// Compress stream in node addon
Napi::Value Compress(const Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  // Tranform stream
  const Napi::Object TransformStream = info[0].ToObject();

  /* Gzip configs */
  int level = 1, verbosity = 0, workFactor = 30;

  /* Setup */
  if (info[1].IsObject()) {
    const Napi::Object Config = info[1].ToObject();
    if (Config.Get("level").IsNumber()) {
      const Napi::Number compressLevel = Config.Get("level").As<Napi::Number>();
      level = compressLevel.Int32Value();
    }

    if (Config.Get("verbosity").IsNumber()) {
      const Napi::Number Ver = Config.Get("verbosity").As<Napi::Number>();
      verbosity = Ver.Int32Value();
    }

    if (Config.Get("workFactor").IsNumber()) {
      const Napi::Number Ver = Config.Get("workFactor").As<Napi::Number>();
      workFactor = Ver.Int32Value();
    }
  }

  /* Compression sets */
  int bzCompressStatus;
  bz_stream bzStream = bz_stream({});
  bzStream.next_in = NULL;
  bzStream.avail_in = 0U;
  bzStream.avail_out = 0U;
  bzStream.bzalloc = NULL;
  bzStream.bzfree = NULL;
  bzStream.opaque = NULL;
  if ((bzCompressStatus = BZ2_bzCompressInit(&bzStream, level, verbosity, workFactor)) != BZ_OK) {
    const char* Message = "Error code: " + (char)bzCompressStatus;
    if (bzCompressStatus == BZ_CONFIG_ERROR) Message = "the library has been mis-compiled";
    else if (bzCompressStatus == BZ_PARAM_ERROR) Message = "Invalid BZ2_bzCompressInit config";
    std::cerr << Message << "\n";
    Napi::Error::New(env, Message).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  // Set tranform Buffer
  TransformStream.Set("_transform", Napi::Function::New(env, [&](const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const Napi::Buffer<char> chuck = info[0].As<Napi::Buffer<char>>();
    ((EState*)bzStream.state)->strm = &bzStream;
    bzStream.next_in = chuck.Data();
    bzStream.avail_in = strlen(bzStream.next_in);
    printf("%d\n", ((EState*)bzStream.state)->mode);

    if ((bzCompressStatus = BZ2_bzCompress(&bzStream, BZ_RUN)) != BZ_OK) {
      const char* Message = "Error code: " + bzCompressStatus;
      if (bzCompressStatus == BZ_CONFIG_ERROR) Message = "the library has been mis-compiled";
      else if (bzCompressStatus == BZ_PARAM_ERROR) Message = "Invalid BZ2_bzCompress config";
      std::cerr << Message << "\n";
      Napi::Error::New(env, Message).ThrowAsJavaScriptException();
      return env.Undefined();
    }
    printf("%d\n", ((EState*)bzStream.state)->mode);


    /* push Buffer to stream */
    const Napi::Object This = info.This().As<Napi::Object>();
    This.Get("push").As<Napi::Function>().Call(This, {
      Napi::Buffer<char*>::New(env, &bzStream.next_out, sizeof(bzStream.next_out))
    });

    // Call callback to next Buffer
    return info[2].As<Napi::Function>().Call({});
  }));

  // set final stream
  TransformStream.Set("_final", Napi::Function::New(env, [&](const Napi::CallbackInfo &info) {
    const Napi::Function Callback = info[0].As<Napi::Function>();
    printf("%d\n", ((EState*)bzStream.state)->mode);

    bzCompressStatus = BZ2_bzCompressEnd(&bzStream);
    if (bzCompressStatus != BZ_OK) {
      const char* Message = "Error code: " + bzCompressStatus;
      if (bzCompressStatus == BZ_CONFIG_ERROR) Message = "the library has been mis-compiled";
      else if (bzCompressStatus == BZ_PARAM_ERROR) Message = "Invalid BZ2_bzCompressEnd config";
      std::cerr << Message << "\n";
      Napi::Error::New(env, Message).ThrowAsJavaScriptException();
      return env.Undefined();
    }

    std::cerr << ((EState*)bzStream.state)->mode << "\n";
    return Callback.Call({});
  }));

  // Return stream
  return TransformStream;
}