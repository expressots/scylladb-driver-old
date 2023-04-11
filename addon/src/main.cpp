#include <napi.h>

#include <playground/custom-object.hpp>
#include <playground/greet-async.hpp>

Napi::Value greet_async(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  auto name = info[0].As<Napi::String>().Utf8Value();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  GreetWorker *worker = new GreetWorker(deferred, name.c_str());
  worker->Queue();

  return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "greetAsync"), Napi::Function::New(env, greet_async));
  exports.Set(Napi::String::New(env, "CustomObject"), playground::CustomObject::GetClass(env));
  return exports;
}

NODE_API_MODULE(addon, Init)
