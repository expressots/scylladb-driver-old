#include <napi.h>

#include <scylladb_wrapper/cluster/cluster.hpp>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "Cluster"), scylladb_wrapper::cluster::Cluster::GetClass(env));
  return exports;
}

NODE_API_MODULE(addon, Init)
