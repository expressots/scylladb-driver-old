#include <fmt/core.h>

#include <nodepp/object-member-function.hpp>
#include <nodepp/promise-worker.hpp>
#include <scylladb_wrapper/cluster/session.hpp>

namespace scylladb_wrapper::cluster {

  Napi::Object Session::GetClass(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object session_object = Napi::Object::New(env);

    session_object.Set(Napi::String::New(env, "executeSync"),
                       NodePP::MemberFunction(env, this, &Session::execute_sync));

    return session_object;
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
      auto results = scylladb_wrapper::cluster::get_string_values_from_result(result);

      // Return the array of strings to the user
      auto array = Napi::Array::New(env, results.size());
      for (size_t i = 0; i < results.size(); i++) {
        array.Set(i, results[i]);
      }

      cass_result_free(result);
      cass_future_free(result_future);
      return array;
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

  std::vector<std::string> get_string_values_from_result(const CassResult* result) {
    std::vector<std::string> values;

    CassIterator* row_iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(row_iterator)) {
      const CassRow* row = cass_iterator_get_row(row_iterator);
      size_t column_count = cass_result_column_count(result);

      for (size_t i = 0; i < column_count; ++i) {
        const CassValue* column_value = cass_row_get_column(row, i);

        // Process the column_value based on its type
        // For example, if it's a text column:
        auto type = cass_value_type(column_value);

        switch (type) {
          case CASS_VALUE_TYPE_TEXT: {
            const char* text;
            size_t text_length;
            cass_value_get_string(column_value, &text, &text_length);
            values.push_back(std::string(text, text_length));
            break;
          }
          case CASS_VALUE_TYPE_UUID: {
            CassUuid uuid;
            cass_value_get_uuid(column_value, &uuid);
            char uuid_str[CASS_UUID_STRING_LENGTH];
            cass_uuid_string(uuid, uuid_str);
            values.push_back(std::string(uuid_str));
            break;
          }
          case CASS_VALUE_TYPE_VARCHAR: {
            const char* text;
            size_t text_length;
            cass_value_get_string(column_value, &text, &text_length);
            values.push_back(std::string(text, text_length));
            break;
          }
          default:
            fmt::print("Unknown type: {}\n", type);
            break;
        }
      }
    }

    cass_iterator_free(row_iterator);

    return values;
  }
}  // namespace scylladb_wrapper::cluster
