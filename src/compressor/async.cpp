#include <napi.h>
#include <thread>
extern "C" {
  #include <bzlib.h>
}

class AsyncCompress : public Napi::AsyncWorker {
  public:
    AsyncCompress(Function& callback, Napi::Object& config) : AsyncWorker(callback), config(config) {}
    ~AsyncCompress() {}

  // This code will be executed on the worker thread
  void Execute() override {
    // Need to simulate cpu heavy task
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  void OnOK() override {
    Napi::HandleScope scope(Env());
    Callback().Call({Env().Null(), Napi::String::New(Env(), config)});
  }

  private:
    Napi::Object config;
};

Napi::Value CallAsyncCompress(const Napi::CallbackInfo& info) {
  // You need to validate the arguments here.
  std::string in = info[0].As<Napi::String>();
  Napi::Function cb = info[1].As<Napi::Function>();
  EchoWorker* wk = new AsyncCompress(cb, in);
  wk->Queue();
  return info.Env().Undefined();
}