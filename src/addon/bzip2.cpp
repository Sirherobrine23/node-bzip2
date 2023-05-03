#include <napi.h>
extern "C" {
  #include "bzip2/bzlib.h"
}

struct params {
  int level;
  int verbosity;
  int work_small;
};


class BZip2 {
  private:
    bool ready_, compress_;
    params params_; bz_stream* stream_;

  public:
    BZip2(const params& p, bool compress) : stream_(new bz_stream), ready_(false), params_(p), compress_(compress) {}
    ~BZip2() {delete static_cast<bz_stream*>(stream_);}

    Napi::Value Init(Napi::Env env) {
      bz_stream* s = static_cast<bz_stream*>(stream_);
      int status = compress_ ? BZ2_bzCompressInit(s, params_.level, params_.verbosity, params_.work_small) : BZ2_bzDecompressInit(s, params_.verbosity, params_.work_small);
      if (status != BZ_OK) {
        if (status == BZ_CONFIG_ERROR) Napi::Error::New(env, "the library has been mis-compiled").ThrowAsJavaScriptException();
        else if (status == BZ_MEM_ERROR) Napi::Error::New(env, "Out of memory").ThrowAsJavaScriptException();
        else if (status == BZ_PARAM_ERROR) Napi::Error::New(env, "Invalid BZ2_bzCompress params").ThrowAsJavaScriptException();
        else Napi::Error::New(env, "Compress init not reached "+status).ThrowAsJavaScriptException();
        return env.Undefined();
      }
      ready_ = true;
      return env.Undefined();
    }

    Napi::Value End(Napi::Env env) {
      ready_ = false;
      bz_stream* s = static_cast<bz_stream*>(stream_);
      int status = compress_ ? BZ2_bzCompressEnd(s) : BZ2_bzDecompressEnd(s);
      if (status != BZ_OK) {

      }
      return env.Undefined();
    }

    Napi::Value Write(Napi::Env env, Napi::Buffer<char> data) {
      if (!ready_) {
        Napi::Error::New(env, "BZip2 not initialized").ThrowAsJavaScriptException();
        return env.Undefined();
      }
    }
};