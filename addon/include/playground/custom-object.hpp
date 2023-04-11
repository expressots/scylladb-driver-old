#pragma once

#include <napi.h>

namespace playground {
  class CustomObject : public Napi::ObjectWrap<CustomObject> {
  private:
    double _value;

  public:
    static Napi::Function GetClass(Napi::Env env);
    CustomObject(const Napi::CallbackInfo &info);

    Napi::Value GetValue(const Napi::CallbackInfo &info);
    Napi::Value Add(const Napi::CallbackInfo &info);
    Napi::Value Multiply(const Napi::CallbackInfo &info);
  };
}  // namespace playground
