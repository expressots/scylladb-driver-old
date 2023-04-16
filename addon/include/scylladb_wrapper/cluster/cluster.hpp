#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>
#include <vector>

namespace scylladb_wrapper::cluster {

  CassCluster *create_cluster(const char *hosts);
  CassError connect_session(CassSession *session, const CassCluster *cluster);

  class Cluster : public Napi::ObjectWrap<Cluster> {
  private:
    std::vector<std::string> nodes;
    std::string port;
    std::string ssl_context;
    std::string username;
    std::string password;

    CassCluster* cluster;
    CassSession* session;

  public:
    static Napi::Function GetClass(Napi::Env env);
    Cluster(const Napi::CallbackInfo &info);
    ~Cluster();

    Napi::Value connect(const Napi::CallbackInfo &info);
  };
}  // namespace scylladb_wrapper
