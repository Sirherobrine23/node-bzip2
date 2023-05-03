#include <napi.h>
extern "C" {
  #include "bzip2/bzlib.h"
}

enum SetConfig {
  Level = 0,
  verbosity = 1,
  work_small = 2
};

class cppBZip2 {
  private:
    bool compress_;
    bz_stream* stream_;
    int level = 9, verbosity = 0, work_small = 0;

  public:
    cppBZip2(bool compress) : compress_(!!compress), stream_(new bz_stream) {}

    void setConfig(SetConfig target, int value) {
      if (target == SetConfig::Level) {
        level = value;
      } else if (target == SetConfig::verbosity) {
        verbosity = value;
      } else if (target == SetConfig::work_small) {
        work_small = value;
      }
    }

    Napi::Value Init(Napi::Env env) {
      int status = compress_ ? BZ2_bzCompressInit(stream_, level, verbosity, work_small) : BZ2_bzDecompressInit(stream_, verbosity, work_small);
      if (status != BZ_OK) {
        if (status == BZ_CONFIG_ERROR) return Napi::Error::New(env, "the library has been mis-compiled").Value();
        else if (status == BZ_MEM_ERROR) return Napi::Error::New(env, "Out of memory").Value();
        else if (status == BZ_PARAM_ERROR) return Napi::Error::New(env, "Invalid params").Value();
        else return Napi::Error::New(env, "Compress init not reached "+status).Value();
      }
      return env.Undefined();
    }

    Napi::Value End(Napi::Env env) {
      int status = compress_ ? BZ2_bzCompressEnd(stream_) : BZ2_bzDecompressEnd(stream_);
      if (status != BZ_OK) {
        if (status == BZ_CONFIG_ERROR) return Napi::Error::New(env, "the library has been mis-compiled").Value();
        else if (status == BZ_MEM_ERROR) return Napi::Error::New(env, "Out of memory").Value();
        else if (status == BZ_PARAM_ERROR) return Napi::Error::New(env, "Invalid params").Value();
        else if (status == BZ_SEQUENCE_ERROR) return Napi::Error::New(env, "Invalid sequence").Value();
        return Napi::Error::New(env, "end not reached "+status).Value();
      }
      return env.Undefined();
    }

    Napi::Value Write(Napi::Env env, Napi::Buffer<char> data) {
      char* src_begin = data.Data();
      char* src_end = src_begin + data.ByteLength();
      char* dest_begin = reinterpret_cast<char*>(malloc(data.ByteLength()+16));
      if (dest_begin == NULL) return Napi::Error::New(env, "Out of memory").Value();
      char* dest_end = dest_begin + data.ByteLength()+16;

      stream_->next_in = const_cast<char*>(src_begin);
      stream_->avail_in = static_cast<unsigned>(src_end - src_begin);
      stream_->next_out = reinterpret_cast<char*>(dest_begin);
      stream_->avail_out = static_cast<unsigned>(dest_end - dest_begin);

      int status = compress_ ? BZ2_bzCompress(stream_, BZ_RUN) : BZ2_bzDecompress(stream_);
      if (compress_ && status != BZ_RUN_OK) {
        free(dest_begin);
        if (status == BZ_OK) return Napi::Error::New(env, "Invalid BZ2_bzCompress config in addon").Value();
        else if (status == BZ_CONFIG_ERROR) return Napi::Error::New(env, "the library has been mis-compiled").Value();
        else if (status == BZ_PARAM_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzCompress params").Value();
        else if (status == BZ_SEQUENCE_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").Value();
        return Napi::Error::New(env, "Compress not reached").Value();
      } else if (!compress_ && status != BZ_OK) {
        free(dest_begin);
        if (status == BZ_DATA_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzDecompress data").Value();
        else if (status == BZ_CONFIG_ERROR) return Napi::Error::New(env, "the library has been mis-compiled").Value();
        else if (status == BZ_PARAM_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzDecompress params").Value();
        else if (status == BZ_SEQUENCE_ERROR) return Napi::Error::New(env, "Invalid BZ2_bzDecompress sequence").Value();
        return Napi::Error::New(env, "Decompress not reached").Value();
      }

      const Napi::Buffer<char> result = Napi::Buffer<char>::New(env, dest_begin, strlen(dest_begin));
      free(dest_begin);
      return result;
    }
};