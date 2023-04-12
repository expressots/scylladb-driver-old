#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>
#include <vector>

namespace scylladb_wrapper::cluster {

  struct Basic_ {
    cass_bool_t bln;
    cass_float_t flt;
    cass_double_t dbl;
    cass_int32_t i32;
    cass_int64_t i64;
  };

  typedef struct Basic_ Basic;  // Not necessary <---

  CassCluster *create_cluster(const char *hosts);
  CassError connect_session(CassSession *session, const CassCluster *cluster);
  CassError execute_query(CassSession *session, const char *query);
  CassError insert_into_basic(CassSession *session, const char *key, const Basic *basic);
  CassError select_from_basic(CassSession *session, const char *key, Basic *basic);

  class Cluster : public Napi::ObjectWrap<Cluster> {
  private:
    std::vector<std::string> nodes;
    std::string port;
    std::string ssl_context;
    std::string username;
    std::string password;

  public:
    static Napi::Function GetClass(Napi::Env env);
    Cluster(const Napi::CallbackInfo &info);

    Napi::Value connect(const Napi::CallbackInfo &info);
  };
}  // namespace scylladb_wrapper
