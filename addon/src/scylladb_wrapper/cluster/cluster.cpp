#include <fmt/core.h>

#include <scylladb_wrapper/cluster/cluster.hpp>
#include <scylladb_wrapper/cluster/session.hpp>

namespace scylladb_wrapper::cluster {
  CassCluster *create_cluster(const char *hosts) {
    CassCluster *cluster = cass_cluster_new();
    cass_cluster_set_contact_points(cluster, hosts);
    return cluster;
  }

  CassError connect_session(CassSession *session, const CassCluster *cluster) {
    CassError rc = CASS_OK;
    CassFuture *future = cass_session_connect(session, cluster);

    cass_future_wait(future);
    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
      // print_error(future);
    }
    cass_future_free(future);

    return rc;
  }

  CassError execute_query(CassSession *session, const char *query) {
    CassError rc = CASS_OK;
    CassFuture *future = NULL;
    CassStatement *statement = cass_statement_new(query, 0);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
      // print_error(future);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
  }

  CassError insert_into_basic(CassSession *session, const char *key, const Basic *basic) {
    CassError rc = CASS_OK;
    CassStatement *statement = NULL;
    CassFuture *future = NULL;
    const char *query
        = "INSERT INTO examples.basic (key, bln, flt, dbl, i32, "
          "i64) VALUES (?, ?, ?, ?, ?, ?);";

    statement = cass_statement_new(query, 6);

    cass_statement_bind_string(statement, 0, key);
    cass_statement_bind_bool(statement, 1, basic->bln);
    cass_statement_bind_float(statement, 2, basic->flt);
    cass_statement_bind_double(statement, 3, basic->dbl);
    cass_statement_bind_int32(statement, 4, basic->i32);
    cass_statement_bind_int64(statement, 5, basic->i64);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
      // print_error(future);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
  }

  CassError select_from_basic(CassSession *session, const char *key, Basic *basic) {
    CassError rc = CASS_OK;
    CassStatement *statement = NULL;
    CassFuture *future = NULL;
    const char *query = "SELECT * FROM examples.basic WHERE key = ?";

    statement = cass_statement_new(query, 1);

    cass_statement_bind_string(statement, 0, key);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
      // print_error(future);
    } else {
      const CassResult *result = cass_future_get_result(future);
      CassIterator *iterator = cass_iterator_from_result(result);

      if (cass_iterator_next(iterator)) {
        const CassRow *row = cass_iterator_get_row(iterator);
        cass_value_get_bool(cass_row_get_column(row, 1), &basic->bln);
        cass_value_get_double(cass_row_get_column(row, 2), &basic->dbl);
        cass_value_get_float(cass_row_get_column(row, 3), &basic->flt);
        cass_value_get_int32(cass_row_get_column(row, 4), &basic->i32);
        cass_value_get_int64(cass_row_get_column(row, 5), &basic->i64);
      }

      cass_result_free(result);
      cass_iterator_free(iterator);
    }

    cass_future_free(future);
    cass_statement_free(statement);

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

    if (connect_session(this->session, this->cluster) != CASS_OK) {
      fmt::print("Unable to connect\n");
      cass_cluster_free(cluster);
      cass_session_free(session);
      Napi::Error::New(env, "Unable to connect").ThrowAsJavaScriptException();
      return env.Null();
    }

    Session *sessionWrapper = new Session(this->session);
    return sessionWrapper->GetClass(info);
  }
}  // namespace scylladb_wrapper::cluster
