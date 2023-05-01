#include <napi.h>
#include <iostream>
#include <cpp_bzip2/bzlib.hh>
#include <cpp_bzip2/bzlib_private.hh>

// Compress stream in node addon
Napi::Value Compress(const Napi::CallbackInfo &info) {
  // Tranform stream
  const Napi::Object TransformStream = info[0].ToObject();

  /* Gzip configs */
  int bzCompressStatus, level = 1, verbosity = 0, workFactor = 30;

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
  bz_stream bzStream;

  bool Runn = false;
  // Set tranform Buffer
  TransformStream.Set("_transform", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();

    if (!Runn) {
      Runn = true;
      bzStream.bzalloc = NULL;
      bzStream.bzfree = NULL;
      bzStream.opaque = NULL;
      bzStream.next_in = NULL;
      bzStream.avail_in = 0;
      bzStream.next_out = NULL;
      bzStream.avail_out = 0;

      std::cout << "Init compress,  level " << level << ", verbosity " << verbosity << ", workFactor " << workFactor << std::endl;
      bzCompressStatus = BZ2_bzCompressInit(&bzStream, level, verbosity, workFactor);
      if (bzCompressStatus != BZ_OK) {
        if (bzCompressStatus == BZ_CONFIG_ERROR) return Callback.Call({Napi::Error::New(info.Env(), "the library has been mis-compiled").Value()});
        else if (bzCompressStatus == BZ_PARAM_ERROR) return Callback.Call({Napi::Error::New(info.Env(), "Invalid BZ2_bzCompress params").Value()});
        else return Callback.Call({Napi::Error::New(info.Env(), "Compress init not reached "+bzCompressStatus).Value()});
      }

    }

    // Initial
    const Napi::Buffer<char*> chuck = info[0].As<Napi::Buffer<char*>>();
    bzStream.avail_in = chuck.ByteLength();
    bzStream.next_in = (char*)chuck.Data();

    // Out
    char* bzBuffer = (char*)malloc(bzStream.avail_in + 16);
    bzStream.next_out = bzBuffer;
    bzStream.avail_out = chuck.ByteLength() + 16;

    ((EState*)bzStream.state)->strm = &bzStream;

    bzCompressStatus = BZ2_bzCompress(&bzStream, BZ_RUN);
    if (bzCompressStatus != BZ_RUN_OK) {
      free(bzBuffer);
      if (bzCompressStatus == BZ_OK) return Callback.Call({Napi::Error::New(info.Env(), "Invalid BZ2_bzCompress config in addon").Value()});
      else if (bzCompressStatus == BZ_CONFIG_ERROR) return Callback.Call({Napi::Error::New(info.Env(), "the library has been mis-compiled").Value()});
      else if (bzCompressStatus == BZ_PARAM_ERROR) return Callback.Call({Napi::Error::New(info.Env(), "Invalid BZ2_bzCompress params").Value()});
      else if (bzCompressStatus == BZ_SEQUENCE_ERROR) return Callback.Call({Napi::Error::New(info.Env(), "Invalid BZ2_bzCompress sequence").Value()});

      std::cout << "Error compressing, code: " << bzCompressStatus << std::endl;
      return Callback.Call({Napi::Error::New(info.Env(), "Compress not reached").Value()});
    }

    /* push Buffer to stream */
    This.Get("push").As<Napi::Function>().Call(This, {
      Napi::Buffer<char>::New(info.Env(), bzBuffer, strlen(bzBuffer))
    });

    // free memory
    free(bzBuffer);

    // Call callback to next Buffer
    return Callback.Call({});
  }));

  // set final stream
  TransformStream.Set("_destroy", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const Napi::Function Callback = info[1].As<Napi::Function>();

    if (!info[0].IsObject()) {
      // bzCompressStatus = BZ2_bzCompress(&bzStream, BZ_FINISH);
      // if (bzCompressStatus != BZ_FINISH_OK) {
      //   if (bzCompressStatus == BZ_CONFIG_ERROR) {
      //     return Callback.Call({Napi::Error::New(env, "the library has been mis-compiled").Value()});
      //   } else if (bzCompressStatus == BZ_PARAM_ERROR) {
      //     return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress end params").Value()});
      //   } else if (bzCompressStatus == BZ_SEQUENCE_ERROR) {
      //     return Callback.Call({Napi::Error::New(env, "Invalid BZ2_bzCompress end sequence").Value()});
      //   } else {
      //     return Callback.Call({Napi::Error::New(env, "Compress finish not reached").Value()});
      //   }
      // }

      bzCompressStatus = BZ2_bzCompressEnd(&bzStream);
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
    } else return Callback({info[0].As<Napi::Object>()});

  }));

  // Return stream
  return TransformStream;
}