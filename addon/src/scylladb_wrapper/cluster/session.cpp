#include <fmt/core.h>
#include <scylladb/cassandra.h>

#include <nodepp/object-member-function.hpp>
#include <nodepp/promise-worker.hpp>
#include <regex>
#include <scylladb_wrapper/cluster/session.hpp>
#include <scylladb_wrapper/types/uuid.hpp>

#define CHECK_CASS_ERROR(rc, type)                                                 \
  do {                                                                             \
    if (rc != CASS_OK) {                                                           \
      Napi::TypeError::New(env, fmt::format("Invalid operation on type {}", type)) \
          .ThrowAsJavaScriptException();                                           \
      return env.Null();                                                           \
    }                                                                              \
  } while (0)

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
    if (query.empty()) {
      Napi::TypeError::New(env, "Query cannot be empty").ThrowAsJavaScriptException();
      return env.Null();
    }

    // There's a second parameter for the query parameters, which is an Array of values
    // We'll use this to bind the values to the query
    // Check if there is a second parameter and if it's an array
    Napi::Array params;
    if (info.Length() == 2) {
      if (!info[1].IsArray()) {
        Napi::TypeError::New(env, "Expected a array for the second parameter")
            .ThrowAsJavaScriptException();
        return env.Null();
      }

      params = info[1].As<Napi::Array>();
    }

    CassStatement* statement = cass_statement_new(query.c_str(), params ? params.Length() : 0);

    // Bind the values to the query
    if (params) {
      for (uint32_t i = 0; i < params.Length(); ++i) {
        Napi::Value param = params.Get(i);

        if (param.IsString()) {
          std::string param_string = param.As<Napi::String>().Utf8Value();
          CHECK_CASS_ERROR(cass_statement_bind_string(statement, i, param_string.c_str()),
                           "string");
        } else if (param.IsNumber()) {
          double param_number = param.As<Napi::Number>().DoubleValue();
          CHECK_CASS_ERROR(cass_statement_bind_double(statement, i, param_number), "number");
        } else if (param.IsBoolean()) {
          bool param_bool = param.As<Napi::Boolean>().Value();
          cass_bool_t cass_bool = param_bool ? cass_true : cass_false;
          CHECK_CASS_ERROR(cass_statement_bind_bool(statement, i, cass_bool), "boolean");
        } else if (param.IsObject()) {
          using namespace scylladb_wrapper::uuid;
          Napi::Object obj = param.As<Napi::Object>();

          if (UUIDv4::is_instance_of(obj)) {
            std::string uuid = Napi::ObjectWrap<UUIDv4>::Unwrap(obj)->to_string_cpp();

            CassUuid cass_uuid;
            CHECK_CASS_ERROR(cass_uuid_from_string(uuid.c_str(), &cass_uuid), "UUID");
            CHECK_CASS_ERROR(cass_statement_bind_uuid(statement, i, cass_uuid), "UUID");
          }

        } else {
          Napi::TypeError::New(env, "Unsupported parameter type").ThrowAsJavaScriptException();
          return env.Null();
        }
      }
    }

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

      fmt::print("Query executed successfully\n");

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
          case CASS_VALUE_TYPE_ASCII: {
            const char* ascii;
            size_t ascii_length;
            cass_value_get_string(column_value, &ascii, &ascii_length);
            column.Set(Napi::String::New(env, column_name),
                       Napi::String::New(env, std::string(ascii, ascii_length)));
            break;
          }
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

  void print_uuid(const CassUuid* uuid) {
    std::uint32_t time_high = static_cast<std::uint32_t>(uuid->time_and_version >> 32);
    std::uint16_t time_low = static_cast<std::uint16_t>(uuid->time_and_version >> 16);
    std::uint16_t time_mid = static_cast<std::uint16_t>(uuid->time_and_version);
    std::uint16_t clock_seq = static_cast<std::uint16_t>(uuid->clock_seq_and_node >> 48);
    std::uint64_t node = uuid->clock_seq_and_node & 0x0000FFFFFFFFFFFF;

    std::string uuid_str = fmt::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}", time_high, time_mid,
                                       time_low, clock_seq, node);

    fmt::print("UUID: {}\n", uuid_str);
  }
}  // namespace scylladb_wrapper::cluster
