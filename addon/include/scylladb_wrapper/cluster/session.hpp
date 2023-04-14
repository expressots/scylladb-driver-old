#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>

namespace scylladb_wrapper::cluster {
  std::vector<std::string> get_string_values_from_result(const CassResult* result);

  class Session {
  private:
    CassSession* session;

  public:
    Napi::Object GetClass(const Napi::CallbackInfo& info);
    Session(CassSession* session) : session(session) {}

    Napi::Value execute_sync(const Napi::CallbackInfo& info);

    ~Session();

  private:
    static void on_execute_complete(CassFuture* future, void* data);
  };
}  // namespace scylladb_wrapper::cluster
