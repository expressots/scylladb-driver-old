#include <fmt/core.h>

#include <nodepp/object-member-function.hpp>
#include <nodepp/promise-worker.hpp>
#include <scylladb_wrapper/cluster/session.hpp>

namespace scylladb_wrapper::cluster {

  Napi::Object Session::GetClass(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object session_object = Napi::Object::New(env);

    // Freeze and seal it.

    session_object.Set(Napi::String::New(env, "executeSync"),
                       NodePP::MemberFunction(env, this, &Session::execute_sync));
    
    session_object.Set(Napi::String::New(env, "setKeyspace"),
                       NodePP::MemberFunction(env, this, &Session::set_keyspace));

    return session_object;
  }

  Napi::Value Session::set_keyspace(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check if argument exists and is a string
    if (info.Length() < 1 || !info[0].IsString()) {
      Napi::TypeError::New(env, "String argument expected").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    std::string keyspace_name = info[0].ToString().Utf8Value();

    // Check if session and cluster instances exist
    if (!session || !cluster) {
      Napi::TypeError::New(env, "Session and cluster instances must be created first")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    // Use cass_session_connect_keyspace function to set the session keyspace
    CassFuture* rc = cass_session_connect_keyspace(session, cluster, keyspace_name.c_str());
    auto error = cass_future_error_code(rc);
    if (error != CASS_OK) {
      const char* message;
      size_t message_length;
      cass_future_error_message(rc, &message, &message_length);
      Napi::TypeError::New(env, std::string(message, message_length)).ThrowAsJavaScriptException();
      return env.Null();
    }

    return env.Null();
  }

  Napi::Value Session::execute_sync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    // Get the string from the first parameter
    std::string query = info[0].As<Napi::String>().Utf8Value();

    CassStatement* statement = cass_statement_new(query.c_str(), 0);
    CassFuture* result_future = cass_session_execute(this->session, statement);
    cass_statement_free(statement);

    auto result_code = cass_future_error_code(result_future);

    if (result_code == CASS_OK) {
      /* Retrieve result set and get the first row */
      const CassResult* result = cass_future_get_result(result_future);
      auto results = scylladb_wrapper::cluster::get_string_values_from_result(info, result);

      Napi::Array resultsArray = results.As<Napi::Array>();

      cass_result_free(result);
      cass_future_free(result_future);
      return resultsArray;
    } else {
      const char* message;
      size_t message_length;
      cass_future_error_message(result_future, &message, &message_length);

      fmt::print("Error: {}\n", message);
      fmt::print("Error code: {}\n", cass_error_desc(result_code));

      cass_future_free(result_future);
      return env.Null();
    }
  }

  Session::~Session() {
    if (session != nullptr) {
      cass_session_free(session);
    }
  }

  Napi::Value get_string_values_from_result(const Napi::CallbackInfo& info,
                                            const CassResult* result) {
    Napi::Env env = info.Env();

    Napi::Array valuesArray = Napi::Array::New(env);

    CassIterator* row_iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(row_iterator)) {
      const CassRow* row = cass_iterator_get_row(row_iterator);
      size_t column_count = cass_result_column_count(result);

      Napi::Object column = Napi::Object::New(env);

      for (size_t i = 0; i < column_count; ++i) {
        const CassValue* column_value = cass_row_get_column(row, i);

        const char* column_name;
        size_t column_name_length;
        cass_result_column_name(result, i, &column_name, &column_name_length);

        // Process the column_value based on its type
        // For example, if it's a text column:
        auto type = cass_value_type(column_value);

        switch (type) {
          case CASS_VALUE_TYPE_TEXT: {
            const char* text;
            size_t text_length;
            cass_value_get_string(column_value, &text, &text_length);
            column.Set(Napi::String::New(env, column_name),
                       Napi::String::New(env, std::string(text, text_length)));
            break;
          }
          case CASS_VALUE_TYPE_BOOLEAN: {
            cass_bool_t value;
            cass_value_get_bool(column_value, &value);
            column.Set(Napi::String::New(env, column_name), Napi::Boolean::New(env, value));
            break;
          }
          case CASS_VALUE_TYPE_UUID: {
            CassUuid uuid;
            cass_value_get_uuid(column_value, &uuid);
            char uuid_str[CASS_UUID_STRING_LENGTH];
            cass_uuid_string(uuid, uuid_str);
            column.Set(Napi::String::New(env, column_name),
                       Napi::String::New(env, std::string(uuid_str)));
            break;
          }
          case CASS_VALUE_TYPE_VARCHAR: {
            const char* text;
            size_t text_length;
            cass_value_get_string(column_value, &text, &text_length);
            column.Set(Napi::String::New(env, column_name),
                       Napi::String::New(env, std::string(text, text_length)));
            break;
          }
          case CASS_VALUE_TYPE_INT: {
            cass_int32_t value;
            cass_value_get_int32(column_value, &value);
            column.Set(Napi::String::New(env, column_name), Napi::Number::New(env, value));
            break;
          }
          default:
            fmt::print("Unknown type: {}\n", static_cast<int>(type));
            break;
        }
      }

      valuesArray.Set(valuesArray.Length(), column);
    }

    cass_iterator_free(row_iterator);

    return valuesArray;
  }
}  // namespace scylladb_wrapper::cluster
