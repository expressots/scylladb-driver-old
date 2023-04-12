#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>

namespace scylladb_wrapper {

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
    std::string host;

  public:
    static Napi::Function GetClass(Napi::Env env);
    Cluster(const Napi::CallbackInfo &info);

    Napi::Value Connect(const Napi::CallbackInfo &info);
  };
}  // namespace scylladb_wrapper
