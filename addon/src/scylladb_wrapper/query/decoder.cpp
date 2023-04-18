#include <fmt/core.h>

#include <scylladb_wrapper/query/decoder.hpp>

namespace scylladb_wrapper::query {
  std::string getStringFromColumnValue(const CassValue* column_value) {
    const char* text;
    size_t text_length;
    cass_value_get_string(column_value, &text, &text_length);
    return std::string(text, text_length);
  }

  Napi::Value decoder(const Napi::CallbackInfo& info, const CassResult* result) {
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

        auto type = cass_value_type(column_value);

        switch (type) {
          case CASS_VALUE_TYPE_TEXT:
          case CASS_VALUE_TYPE_VARCHAR: {
            std::string value = getStringFromColumnValue(column_value);
            column.Set(Napi::String::New(env, column_name), Napi::String::New(env, value));
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
          case CASS_VALUE_TYPE_INT: {
            cass_int32_t value;
            cass_value_get_int32(column_value, &value);
            column.Set(Napi::String::New(env, column_name), Napi::Number::New(env, value));
            break;
          }
          case CASS_VALUE_TYPE_BIGINT: {
            cass_int64_t value;
            cass_value_get_int64(column_value, &value);
            column.Set(Napi::String::New(env, column_name), Napi::BigInt::New(env, value));
            break;
          }
          case CASS_VALUE_TYPE_FLOAT: {
            cass_float_t value;
            cass_value_get_float(column_value, &value);
            column.Set(Napi::String::New(env, column_name), Napi::Number::New(env, value));
            break;
          }
          case CASS_VALUE_TYPE_DOUBLE: {
            cass_double_t value;
            cass_value_get_double(column_value, &value);
            column.Set(Napi::String::New(env, column_name), Napi::Number::New(env, value));
            break;
          }
          case CASS_VALUE_TYPE_TIMESTAMP: {
            cass_int64_t value;
            cass_value_get_int64(column_value, &value);
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
}  // namespace scylladb_wrapper::query