#include <napi.h>
#include <iostream>
#include <cpp_bzip2/bzlib_private.hh>
#include <cpp_bzip2/bzlib.hh>
#include <cpp_bzip2/bzlib.cpp>

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

  Napi::Value compressInit(Napi::Env env) {
    std::cerr << "Init compress, level " << blockSize100k << ", verbosity " << verbosity << ", workFactor " << workFactor << std::endl;
    if (!bz_config_ok()) return Napi::Error::New(env, "bz_config_ok failed").Value();
    if (blockSize100k < 1 || blockSize100k > 9) return Napi::Error::New(env, "blockSize100k must be between 1 and 9").Value();
    if (workFactor < 0 || workFactor > 250) return Napi::Error::New(env, "workFactor must be between 0 and 250").Value();
    if (workFactor == 0) workFactor = 30;
    strm.bzalloc = default_bzalloc;
    strm.bzfree = default_bzfree;

    EState* s = new EState;
    s->arr1 = NULL;
    s->arr2 = NULL;
    s->ftab = NULL;

    Int32 n = 100000 * blockSize100k;
    s->arr1 = (UInt32 *)malloc(n * sizeof(UInt32));
    s->arr2 = (UInt32 *)malloc((n+BZ_N_OVERSHOOT) * sizeof(UInt32));
    s->ftab = (UInt32 *)malloc(65537 * sizeof(UInt32));

    if (s->arr1 == NULL || s->arr2 == NULL || s->ftab == NULL) {
      if (s->arr1 != NULL) free(s->arr1);
      if (s->arr2 != NULL) free(s->arr2);
      if (s->ftab != NULL) free(s->ftab);
      if (s       != NULL) free(s);
      return Napi::Error::New(env, "malloc failed, MemoryError").Value();
    }

    s->blockNo           = 0;
    s->state             = BZ_S_INPUT;
    s->mode              = BZ_M_RUNNING;
    s->combinedCRC       = 0;
    s->blockSize100k     = blockSize100k;
    s->nblockMAX         = 100000 * blockSize100k - 19;
    s->verbosity         = verbosity;
    s->workFactor        = workFactor;

    s->block             = (UChar*)s->arr2;
    s->mtfv              = (UInt16*)s->arr1;
    s->zbits             = NULL;
    s->ptr               = (UInt32*)s->arr1;

    strm.total_in_lo32  = 0;
    strm.total_in_hi32  = 0;
    strm.total_out_lo32 = 0;
    strm.total_out_hi32 = 0;
    strm.state          = s;
    ((EState *)strm.state)->strm = (bz_stream*)&strm;
    init_RL(s);
    prepare_new_block(s);
    return env.Undefined();
  }

  bool compressData(Napi::Env env, Napi::Buffer<char*> chuck, Napi::Object This, Napi::Function Callback) {
    // Input
    strm.avail_in = chuck.ByteLength();
    strm.next_in = (char*)chuck.Data();

    // Out
    char* bzBuffer = (char*)malloc(strm.avail_in + 16);
    strm.next_out = bzBuffer;
    strm.avail_out = chuck.ByteLength() + 16;

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
    This.Get("push").As<Napi::Function>().Call(This, {
      Napi::Buffer<char>::New(env, bzBuffer, strlen(bzBuffer))
    });

    // free memory
    free(bzBuffer);

    // Call callback to next Buffer
    Callback.Call({});

    return Napi::Boolean::New(env, true);
  }

  Napi::Value compressEnd(Napi::Env env, Napi::Value error, Napi::Function Callback) {
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
  const Napi::Value initStatus = Compress.compressInit(env);
  if (!(initStatus.IsUndefined() || initStatus.IsNull())) {
    initStatus.As<Napi::Error>().ThrowAsJavaScriptException();
    return env.Undefined();
  }

  TransformStream.Set("_transform", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();
    const Napi::Buffer<char*> chuck = info[0].As<Napi::Buffer<char*>>();

    Compress.compressData(info.Env(), chuck, This, Callback);
  }));

  TransformStream.Set("_destroy", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Value Error = info[0];
    const Napi::Function Callback = info[1].As<Napi::Function>();
    Compress.compressEnd(info.Env(), Error, Callback);
  }));

  return TransformStream;
}