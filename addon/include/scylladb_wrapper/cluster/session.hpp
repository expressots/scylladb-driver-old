#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>

namespace scylladb_wrapper::cluster {
  Napi::Value get_string_values_from_result(const Napi::CallbackInfo& info, const CassResult* result);

  class Session {
  private:
    CassSession* session;
    CassCluster* cluster;

  public:
    Napi::Object GetClass(const Napi::CallbackInfo& info);
    Session(CassSession* session, CassCluster* cluster) : session(session), cluster(cluster) {}

    Napi::Value execute_sync(const Napi::CallbackInfo& info);
    Napi::Value set_keyspace(const Napi::CallbackInfo& info);

    ~Session();

  private:
    static void on_execute_complete(CassFuture* future, void* data);
  };
}  // namespace scylladb_wrapper::cluster
