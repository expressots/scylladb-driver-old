#include <napi.h>

#include <scylladb_wrapper/cluster.hpp>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "Cluster"), scylladb_wrapper::Cluster::GetClass(env));
  return exports;
}

NODE_API_MODULE(addon, Init)
