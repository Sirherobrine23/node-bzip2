#include <napi.h>
#include <iostream>
#include <cpp_bzip2/bzlib.hh>
#include <cpp_bzip2/bzlib_private.hh>

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
  bz_stream* bzStream = new bz_stream;
  bzStream->bzalloc = NULL;
  bzStream->bzfree = NULL;
  bzStream->opaque = NULL;

  bzCompressStatus = BZ2_bzCompressInit(bzStream, level, verbosity, workFactor);
  if (bzCompressStatus != BZ_OK) {
    if (bzCompressStatus == BZ_CONFIG_ERROR) {
      Napi::Error::New(env, "the library has been mis-compiled").ThrowAsJavaScriptException();
    } else if (bzCompressStatus == BZ_PARAM_ERROR) {
      Napi::Error::New(env, "Invalid BZ2_bzCompress params").ThrowAsJavaScriptException();
    } else {
      Napi::Error::New(env, "Compress not reached").ThrowAsJavaScriptException();
    }
    return env.Undefined();
  }

  // Set tranform Buffer
  TransformStream.Set("_transform", Napi::Function::New(env, [&](const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();

    // Set Running
    bzStream->state = new EState;
    ((EState*)bzStream->state)->state = BZ_S_INPUT;
    ((EState*)bzStream->state)->mode = BZ_M_RUNNING;

    /// Initial
    const Napi::Buffer<char*> chuck = info[0].As<Napi::Buffer<char*>>();
    bzStream->avail_in = chuck.ByteLength();
    bzStream->next_in = (char*)chuck.Data();

    // Out
    char* bzBuffer = {0};
    bzStream->next_out = bzBuffer;
    bzStream->avail_out = chuck.ByteLength() + 16;
    ((EState*)bzStream->state)->strm = bzStream;

    bzCompressStatus = BZ2_bzCompress(bzStream, BZ_Action::Run);
    if (!(bzCompressStatus == BZ_RUN_OK || bzCompressStatus == BZ_OK)) {
      std::cout << bzCompressStatus << "\n\n";
      if (bzCompressStatus == BZ_CONFIG_ERROR) return Callback.Call({Napi::Error::New(env, "the library has been mis-compiled").Value()});
      else if (bzCompressStatus == BZ_PARAM_ERROR) return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value()});
      else if (bzCompressStatus == BZ_SEQUENCE_ERROR) return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").Value()});
      else return Callback.Call({Napi::Error::New(env, "Compress not reached").Value()});
    }

    std::cout << "Push\n";

    /* push Buffer to stream */
    This.Get("push").As<Napi::Function>().Call(This, {
      Napi::Buffer<char>::New(env, bzBuffer, strlen(bzBuffer))
    });

    // Call callback to next Buffer
    return Callback.Call({});
  }));

  // set final stream
  TransformStream.Set("_final", Napi::Function::New(env, [&](const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const Napi::Function Callback = info[0].As<Napi::Function>();

    bzCompressStatus = BZ2_bzCompress(bzStream, BZ_Action::Finish);
    if (bzCompressStatus != BZ_FINISH_OK) {
      if (bzCompressStatus == BZ_CONFIG_ERROR) {
        return Callback.Call({Napi::Error::New(env, "the library has been mis-compiled").Value()});
      } else if (bzCompressStatus == BZ_PARAM_ERROR) {
        return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value()});
      } else if (bzCompressStatus == BZ_SEQUENCE_ERROR) {
        return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").Value()});
      } else {
        return Callback.Call({Napi::Error::New(env, "Compress not reached").Value()});
      }
    }

    bzCompressStatus = BZ2_bzCompressEnd(bzStream);
    if (bzCompressStatus != BZ_OK) {
      if (bzCompressStatus == BZ_CONFIG_ERROR) {
        return Callback.Call({Napi::Error::New(env, "the library has been mis-compiled").Value()});
      } else if (bzCompressStatus == BZ_PARAM_ERROR) {
        return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value()});
      } else if (bzCompressStatus == BZ_SEQUENCE_ERROR) {
        return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").Value()});
      } else {
        return Callback.Call({Napi::Error::New(env, "Compress not reached").Value()});
      }
    }
    return Callback.Call({});
  }));

  // Return stream
  return TransformStream;
}