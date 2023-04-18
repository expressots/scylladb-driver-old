#include <fmt/core.h>

#include <nodepp/object-member-function.hpp>
#include <nodepp/promise-worker.hpp>
#include <scylladb_wrapper/cluster/session.hpp>
#include <scylladb_wrapper/query/decoder.hpp>

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
      auto results = scylladb_wrapper::query::decoder(info, result);

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

}  // namespace scylladb_wrapper::cluster
