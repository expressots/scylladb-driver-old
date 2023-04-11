#pragma once

#include <fmt/format.h>
#include <napi.h>

#include <playground/promise-worker.hpp>
#include <thread>

class GreetWorker : public playground::PromiseWorker {
public:
  GreetWorker(Napi::Promise::Deferred const &d, const char *name)
      : PromiseWorker(d, "GreetWorker"), name(name) {}

  void Execute() override { std::this_thread::sleep_for(std::chrono::seconds(1)); }

  void Resolve(Napi::Promise::Deferred const &deferred) override {
    deferred.Resolve(Napi::String::New(Env(), fmt::format("Hello, {}!\nUsing lib {{fmt}} in C++", name)));
  }

  void Reject(Napi::Promise::Deferred const &deferred, Napi::Error const &error) override {
    deferred.Reject(error.Value());
  }

private:
  std::string name;
};
