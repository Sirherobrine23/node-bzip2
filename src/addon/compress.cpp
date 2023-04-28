#include <napi.h>
extern "C" {
  #include <bzip2/bzlib.h>
}

// Compress stream in node addon
Napi::Value Compress(Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();

  // Level compression
  const Napi::Number compressLevel = info[0].ToObject().Get("level").As<Napi::Number>();
  int32_t level = 2;
  if (compressLevel.IsNumber()) {
    if (!(compressLevel.Int32Value() > 0 && compressLevel.Int32Value() < 10)) {
      Napi::TypeError::New(env, "Invalid Level compress").ThrowAsJavaScriptException();
      return env.Undefined();
    } else level = compressLevel.Int32Value();
  } else level = 1;

  // Tranform stream
  const Napi::Object TransformStream = info[1].ToObject();
  const Napi::Function pushBuffer = TransformStream.Get("push").As<Napi::Function>();

  // Set tranform Buffer
  TransformStream.Set("_transform", Napi::Function::New(env, [&](Napi::CallbackInfo &info) {
    const Napi::Buffer<int32_t> Chuck = info[0].As<Napi::Buffer<int32_t>>();
    const Napi::String encoding = info[0].As<Napi::String>();
    const Napi::FunctionReference Callback = info[0].As<Napi::FunctionReference>();

    // push Buffer to stream
    pushBuffer.Call({});

    // Call callback to next Buffer
    Callback.Call({});
  }));

  // Return stream
  return TransformStream;
}