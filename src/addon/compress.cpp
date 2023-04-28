#include <napi.h>
#include <iostream>
extern "C" {
  #include <bzip2/bzlib.h>
}

using namespace std;

// Compress stream in node addon
Napi::Value Compress(const Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  // Tranform stream
  const Napi::Object TransformStream = info[0].ToObject();

  /* Gzip configs */
  int level = 1;

  /* Setup */
  if (info[1].IsObject()) {
    const Napi::Number compressLevel = info[0].ToObject().Get("level").As<Napi::Number>();
    if (compressLevel.IsNumber()) {
      if (!(compressLevel.Int32Value() > 0 && compressLevel.Int32Value() < 10)) {
        Napi::TypeError::New(env, "Invalid Level compress").ThrowAsJavaScriptException();
        return env.Undefined();
      } else level = compressLevel.Int32Value();
    }
  }

  /* Compression sets */
  int bzCompressStatus;
  bz_stream bzStream = bz_stream({});
  bzStream.bzalloc = NULL;
  bzStream.opaque = NULL;
  bzStream.bzfree = NULL;
  if ((bzCompressStatus = BZ2_bzCompressInit(&bzStream, level, 0, 0)) != BZ_OK) {
    const char* Message = "Undefined bzconfig error, error code: " + bzCompressStatus;
    if (bzCompressStatus == BZ_CONFIG_ERROR) Message = "the library has been mis-compiled";
    else if (bzCompressStatus == BZ_PARAM_ERROR) Message = "Invalid parameters in Addon";
    Napi::TypeError::New(env, Message).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  // Set tranform Buffer
  TransformStream.Set("_transform", Napi::Function::New(env, [&](const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const Napi::Buffer<const char> chuck = info[0].As<Napi::Buffer<const char>>();
    bzStream.next_in = (char*)chuck.Data();
    bzStream.avail_in = strlen(bzStream.next_in);

    if ((bzCompressStatus = BZ2_bzCompress(&bzStream, BZ_RUN)) != BZ_RUN_OK) {
      const char* Message = "Undefined bzconfig error, error code: " + bzCompressStatus;
      if (bzCompressStatus == BZ_CONFIG_ERROR) Message = "the library has been mis-compiled";
      else if (bzCompressStatus == BZ_PARAM_ERROR) Message = "Invalid bzStream config";
      cout << Message << "\n";
      Napi::Error::New(env, Message).ThrowAsJavaScriptException();
      return env.Undefined();
    }

    /* push Buffer to stream */
    const Napi::String PushEncoding = Napi::String::New(env, "binary");

    const Napi::Object This = info.This().As<Napi::Object>();
    This.Get("push").As<Napi::Function>().Call(This, {Napi::Buffer<char>::Copy(env, bzStream.next_out, sizeof(bzStream.next_out))});

    // Call callback to next Buffer
    return info[2].As<Napi::Function>().Call({});
  }));

  // set final stream
  TransformStream.Set("_final", Napi::Function::New(env, [&](const Napi::CallbackInfo &info) {
    const Napi::Function Callback = info[0].As<Napi::Function>();
    bzStream.avail_in = 0;
    bzStream.next_in = NULL;
    bzCompressStatus = BZ2_bzCompressEnd(&bzStream);

    if (!(bzCompressStatus == BZ_FINISH_OK || bzCompressStatus == BZ_STREAM_END)) {
      Napi::Error::New(info.Env(), "Cannot finish bzip2 compactation").ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }

    return Callback.Call({});
  }));

  // Return stream
  return TransformStream;
}