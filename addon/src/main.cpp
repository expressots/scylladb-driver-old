#include <napi.h>

#include <scylladb_wrapper/cluster/cluster.hpp>
#include <scylladb_wrapper/cluster/session.hpp>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "Cluster"), scylladb_wrapper::cluster::Cluster::GetClass(env));
  exports.Set(Napi::String::New(env, "Session"), Napi::Function::New(env, &scylladb_wrapper::cluster::Session::GetClass));
  return exports;
}

NODE_API_MODULE(addon, Init)
