#include <fmt/core.h>

#include <scylladb_wrapper/cluster.hpp>

namespace scylladb_wrapper {
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
        InstanceMethod("connect", &Cluster::Connect),
    };
    Napi::Function func = DefineClass(env, "Cluster", methods);

    return func;
  }

  Cluster::Cluster(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Cluster>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    int length = info.Length();
    if (length != 1 || !info[0].IsString()) {
      Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    }

    this->host = info[0].As<Napi::String>().Utf8Value();
  }

  Napi::Value Cluster::Connect(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    CassCluster *cluster = NULL;
    auto session = cass_session_new();

    cluster = create_cluster(this->host.c_str());

    if (connect_session(session, cluster) != CASS_OK) {
      cass_cluster_free(cluster);
      cass_session_free(session);

      return Napi::String::New(env, fmt::format("Unable to connect to cluster: {}", this->host));
    }

    /* Build statement and execute query */
    const char *query = "SELECT release_version FROM system.local";
    CassStatement *statement = cass_statement_new(query, 0);

    CassFuture *result_future = cass_session_execute(session, statement);
    if (cass_future_error_code(result_future) == CASS_OK) {
      /* Retrieve result set and get the first row */
      const CassResult *result = cass_future_get_result(result_future);
      const CassRow *row = cass_result_first_row(result);

      if (row) {
        const CassValue *value = cass_row_get_column_by_name(row, "release_version");

        const char *release_version;
        size_t release_version_length;
        cass_value_get_string(value, &release_version, &release_version_length);
        return Napi::String::New(env, fmt::format("Release version: {}", release_version));
      }

      cass_result_free(result);
    }

    cass_session_free(session);
    cass_cluster_free(cluster);

    return Napi::String::New(env, fmt::format("Connected to cluster: {}", this->host));
  }
}  // namespace scylladb_wrapper
