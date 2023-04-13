#pragma once

#include <napi.h>

#include <functional>

namespace NodePP {
  template <typename ClassType, typename FunctionType>
  Napi::Function MemberFunction(Napi::Env env, ClassType *instance, FunctionType function) {
    return Napi::Function::New(
        env, [=](const Napi::CallbackInfo &info) { return (instance->*function)(info); });
  }
}  // namespace NodePP
