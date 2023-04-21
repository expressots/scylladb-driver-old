#include <fmt/core.h>

#include <optional>
#include <scylladb_wrapper/cluster/cluster.hpp>
#include <scylladb_wrapper/cluster/session.hpp>

namespace scylladb_wrapper::cluster {
  CassCluster *create_cluster(const char *hosts) {
    CassCluster *cluster = cass_cluster_new();
    cass_cluster_set_contact_points(cluster, hosts);
    return cluster;
  }

  CassError connect_session(CassSession *session, const CassCluster *cluster,
                            std::string *keyspace) {
    CassError rc = CASS_OK;
    CassFuture *future = nullptr;
    if (keyspace != nullptr)
      future = cass_session_connect_keyspace(session, cluster, keyspace->c_str());
    else
      future = cass_session_connect(session, cluster);

    cass_future_wait(future);
    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
      // print_error(future);
    }
    cass_future_free(future);

    return rc;
  }

  Napi::Function Cluster::GetClass(Napi::Env env) {
    auto methods = {
        InstanceMethod("connect", &Cluster::connect),
    };
    Napi::Function func = DefineClass(env, "Cluster", methods);

    return func;
  }

  Cluster::~Cluster() {
    if (cluster != nullptr) {
      cass_cluster_free(cluster);
    }
    if (session != nullptr) {
      cass_session_free(session);
    }
  }

  Cluster::Cluster(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Cluster>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    int length = info.Length();
    if (length > 1) {
      Napi::TypeError::New(env, "Expected at most 1 argument").ThrowAsJavaScriptException();
      return;
    }

    // If the user provided an object parameter we fill the options with the provided values
    if (length == 1) {
      if (!info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected an object as parameter").ThrowAsJavaScriptException();
        return;
      }

      Napi::Object options = info[0].As<Napi::Object>();

      if (options.Has("nodes")) {
        Napi::Value nodes = options.Get("nodes");
        if (!nodes.IsArray()) {
          Napi::TypeError::New(env, "Expected an array of nodes").ThrowAsJavaScriptException();
          return;
        }

        Napi::Array nodesArray = nodes.As<Napi::Array>();
        for (uint32_t i = 0; i < nodesArray.Length(); i++) {
          Napi::Value node = nodesArray.Get(i);
          if (!node.IsString()) {
            Napi::TypeError::New(env, "Expected an array of strings").ThrowAsJavaScriptException();
            return;
          }

          this->nodes.push_back(
              node.As<Napi::String>().Utf8Value());  // FIXME: Only used the first node 🤷
        }
      } else {
        this->nodes.push_back("127.0.0.1");
      }

      if (options.Has("port")) {
        Napi::Value port = options.Get("port");
        if (!port.IsNumber() && !port.IsString()) {
          Napi::TypeError::New(env, "Expected a number or string for port")
              .ThrowAsJavaScriptException();
          return;
        }

        this->port = port.As<Napi::String>().Utf8Value();
      } else {
        this->port = "9042";
      }

      if (options.Has("sslContext")) {
        Napi::Value sslContext = options.Get("sslContext");
        if (!sslContext.IsString()) {
          Napi::TypeError::New(env, "Expected a string for sslContext")
              .ThrowAsJavaScriptException();
          return;
        }

        this->ssl_context = sslContext.As<Napi::String>().Utf8Value();
      } else {
        this->ssl_context = "";  // FIXME: UNUSED
      }

      if (options.Has("username")) {
        Napi::Value username = options.Get("username");
        if (!username.IsString()) {
          Napi::TypeError::New(env, "Expected a string for username").ThrowAsJavaScriptException();
          return;
        }

        this->username = username.As<Napi::String>().Utf8Value();
      } else {
        this->username = "";  // FIXME: UNUSED
      }

      if (options.Has("password")) {
        Napi::Value password = options.Get("password");
        if (!password.IsString()) {
          Napi::TypeError::New(env, "Expected a string for password").ThrowAsJavaScriptException();
          return;
        }

        this->password = password.As<Napi::String>().Utf8Value();
      } else {
        this->password = "";  // FIXME: UNUSED
      }
    }
  }

  Napi::Value Cluster::connect(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    this->session = cass_session_new();
    this->cluster = create_cluster(this->nodes[0].c_str());

    // Optional keyspace string parameter
    std::string *keyspace = nullptr;
    if (info.Length() == 1) {
      if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string for keyspace").ThrowAsJavaScriptException();
        return env.Null();
      }

      keyspace = new std::string(info[0].As<Napi::String>().Utf8Value());
    }

    if (connect_session(this->session, this->cluster, keyspace) != CASS_OK) {
      fmt::print("Unable to connect\n");
      cass_cluster_free(cluster);
      cass_session_free(session);
      Napi::Error::New(env, "Unable to connect").ThrowAsJavaScriptException();
      return env.Null();
    }

    Session *sessionWrapper = new Session(this->session, this->cluster);
    return sessionWrapper->GetClass(info);
  }
}  // namespace scylladb_wrapper::cluster
