#include <napi.h>

#include <scylladb_wrapper/cluster/cluster.hpp>
#include <scylladb_wrapper/types/uuid.hpp>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("Cluster", scylladb_wrapper::cluster::Cluster::GetClass(env));
  exports.Set("UUIDv4", scylladb_wrapper::uuid::UUIDv4::GetClass(env));
  return exports;
}

NODE_API_MODULE(addon, Init)
