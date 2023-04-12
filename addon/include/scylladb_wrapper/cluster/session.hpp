#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>

namespace scylladb_wrapper::cluster {
  std::vector<std::string> get_string_values_from_result(const CassResult* result);

  class Session {
  private:
    CassSession* session = nullptr;

  public:
    static Napi::Object GetClass(const Napi::CallbackInfo& info);
    Session(CassSession* session);

    Napi::Value execute_sync(const Napi::CallbackInfo& info);

    ~Session();
  };
}  // namespace scylladb_wrapper::cluster
