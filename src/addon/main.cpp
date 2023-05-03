#include <napi.h>
#include <iostream>
#include <bzip2.cpp>

Napi::Value Compress(const Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  const Napi::Object TransformStream = info[0].ToObject();
  cppBZip2 Compress = cppBZip2(true);
  /* Setup */
  if (info[1].IsObject()) {
    const Napi::Object Config = info[1].ToObject();
    if (Config.Get("workFactor").IsNumber()) Compress.setConfig(SetConfig::work_small, Config.Get("workFactor").As<Napi::Number>().Int32Value());
    if (Config.Get("verbosity").IsNumber()) Compress.setConfig(SetConfig::verbosity, Config.Get("verbosity").As<Napi::Number>().Int32Value());
    if (Config.Get("level").IsNumber()) Compress.setConfig(SetConfig::Level, Config.Get("level").As<Napi::Number>().Int32Value());
  }

  const Napi::Value initStatus = Compress.Init(env);
  if (!(initStatus.IsUndefined())) {
    Napi::Error::New(env, initStatus.As<Napi::Object>().Get("message").ToString().Utf8Value().c_str()).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  TransformStream.Set("_transform", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();
    const Napi::Buffer<char> chuck = info[0].As<Napi::Buffer<char>>();
    const Napi::Value Status = Compress.Write(info.Env(), chuck);
    if (!(Status.IsBuffer())) return Callback.Call({Status});
    This.Get("push").As<Napi::Function>().Call(This, {Status});
    return Callback.Call({});
  }));

  TransformStream.Set("_destroy", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Function Callback = info[1].As<Napi::Function>();
    const Napi::Value Error = info[0];
    const Napi::Value Status = Compress.End(info.Env());
    if (!Status.IsUndefined()) return Callback.Call({Status});
    if (!Error.IsUndefined()) return Callback.Call({Error});
    return Callback.Call({});
  }));

  return TransformStream;
}

Napi::Value Descompress(const Napi::CallbackInfo &info) {
  const Napi::Env env = info.Env();
  const Napi::Object TransformStream = info[0].ToObject();
  cppBZip2 Descompress = cppBZip2(false);
  /* Setup */
  if (info[1].IsObject()) {
    const Napi::Object Config = info[1].ToObject();
    if (Config.Get("verbosity").IsNumber()) Descompress.setConfig(SetConfig::verbosity, Config.Get("verbosity").As<Napi::Number>().Int32Value());
    if (Config.Get("small").IsNumber()) Descompress.setConfig(SetConfig::work_small, Config.Get("small").As<Napi::Number>().Int32Value());
  }

  const Napi::Value initStatus = Descompress.Init(env);
  if (!(initStatus.IsUndefined())) {
    initStatus.As<Napi::Error>().ThrowAsJavaScriptException();
    return env.Undefined();
  }

  TransformStream.Set("_transform", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Object This = info.This().As<Napi::Object>();
    const Napi::Function Callback = info[2].As<Napi::Function>();
    const Napi::Buffer<char> chuck = info[0].As<Napi::Buffer<char>>();
    const Napi::Value Status = Descompress.Write(info.Env(), chuck);
    if (!(Status.IsBuffer())) return Callback.Call({Status});
    This.Get("push").As<Napi::Function>().Call(This, {Status});
    return Callback.Call({});
  }));

  TransformStream.Set("_destroy", Napi::Function::New(info.Env(), [&](const Napi::CallbackInfo &info) {
    const Napi::Function Callback = info[1].As<Napi::Function>();
    const Napi::Value Error = info[0];
    const Napi::Value Status = Descompress.End(info.Env());
    if (!Error.IsUndefined()) return Callback.Call({Error});
    if (!Status.IsUndefined()) return Callback.Call({Status});
    return Callback.Call({});
  }));

  return TransformStream;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("Compress", Napi::Function::New(env, Compress));
  exports.Set("Descompress", Napi::Function::New(env, Descompress));
  return exports;
}
NODE_API_MODULE(addon, Init);