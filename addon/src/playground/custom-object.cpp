#include <playground/custom-object.hpp>

namespace playground {
  Napi::Function CustomObject::GetClass(Napi::Env env) {
    auto methods = {
        InstanceMethod("getValue", &CustomObject::GetValue),
        InstanceMethod("add", &CustomObject::Add),
        InstanceMethod("multiply", &CustomObject::Multiply),
    };
    Napi::Function func = DefineClass(env, "CustomObject", methods);

    return func;
  }

  CustomObject::CustomObject(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<CustomObject>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    int length = info.Length();
    if (length != 1 || !info[0].IsNumber()) {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    }

    _value = info[0].As<Napi::Number>().DoubleValue();
  }

  Napi::Value CustomObject::GetValue(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    return Napi::Number::New(env, _value);
  }

  Napi::Value CustomObject::Add(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    int length = info.Length();
    if (length != 1 || !info[0].IsNumber()) {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    }

    double arg0 = info[0].As<Napi::Number>().DoubleValue();
    _value += arg0;

    return env.Null();
  }

  Napi::Value CustomObject::Multiply(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    int length = info.Length();
    if (length != 1 || !info[0].IsNumber()) {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    }

    double arg0 = info[0].As<Napi::Number>().DoubleValue();
    _value *= arg0;

    return env.Null();
  }
}  // namespace playground
