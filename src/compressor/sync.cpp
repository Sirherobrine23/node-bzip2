#include <napi.h>
extern "C" {
  #include <bzlib.h>
}

Napi::Value SyncCompress(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  int bzCompressStatus, level = 1, verbosity = 0, workFactor = 30;
  if (!(info[0].IsBuffer())) {
    Napi::Error::New(env, "First argument must be a Buffer").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info[1].IsObject()) {
    const Napi::Object Config = info[1].ToObject();
    if (Config.Get("level").IsNumber()) level = Config.Get("level").ToNumber().Int32Value();
    if (Config.Get("verbosity").IsNumber()) verbosity = Config.Get("verbosity").ToNumber().Int32Value();
    if (Config.Get("workFactor").IsNumber()) workFactor = Config.Get("workFactor").ToNumber().Int32Value();
  }

  /* Buffer data */
  const Napi::Buffer<char> data = info[0].As<Napi::Buffer<char>>();
  bz_stream* stream_ = new bz_stream({});
  stream_->bzalloc = NULL;
  stream_->bzfree = NULL;
  stream_->opaque = NULL;

  bzCompressStatus = BZ2_bzCompressInit(stream_, level, verbosity, workFactor);
  if (bzCompressStatus != BZ_OK) {
    if (bzCompressStatus == BZ_CONFIG_ERROR) {
      Napi::Error::New(info.Env(), "the library has been mis-compiled").ThrowAsJavaScriptException();
    } else if (bzCompressStatus == BZ_PARAM_ERROR) {
      Napi::Error::New(info.Env(), "Invalid BZ2_bzCompress params").ThrowAsJavaScriptException();
    } else {
      std::string err = "BZ2_bzCompressInit, Error code: ";
      Napi::Error::New(env, err.append(std::to_string(bzCompressStatus)).c_str()).ThrowAsJavaScriptException();
    }
    return env.Undefined();
  }

  size_t dataSize = data.ByteLength();
  int out = dataSize + (dataSize / 100) + 600;
  char* dest_begin = new char[out];

  char* src_begin = data.Data();
  stream_->avail_in = data.ByteLength();
  stream_->next_in = src_begin;
  stream_->next_out = dest_begin;
  stream_->avail_out = out;

  bzCompressStatus = BZ2_bzCompress(stream_, BZ_FINISH);
  if (!(bzCompressStatus == BZ_FINISH_OK || bzCompressStatus == BZ_STREAM_END)) {
    delete[] dest_begin;
    if (bzCompressStatus == BZ_OK) Napi::Error::New(env, "Invalid BZ2_bzCompress config in addon").ThrowAsJavaScriptException();
    else if (bzCompressStatus == BZ_CONFIG_ERROR) Napi::Error::New(env, "the library has been mis-compiled").ThrowAsJavaScriptException();
    else if (bzCompressStatus == BZ_PARAM_ERROR) Napi::Error::New(env, "Invalid BZ2_bzCompress params").ThrowAsJavaScriptException();
    else if (bzCompressStatus == BZ_SEQUENCE_ERROR) Napi::Error::New(env, "Invalid BZ2_bzCompress sequence").ThrowAsJavaScriptException();
    else {
      std::string err = "BZ2_bzCompress, Error code: ";
      Napi::Error::New(env, err.append(std::to_string(bzCompressStatus)).c_str()).ThrowAsJavaScriptException();
    }
    return env.Undefined();
  }
  BZ2_bzCompressEnd(stream_);

  const Napi::Buffer<char> result = Napi::Buffer<char>::New(env, dest_begin, stream_->avail_out);
  delete[] dest_begin;

  delete stream_;
  return result;
}