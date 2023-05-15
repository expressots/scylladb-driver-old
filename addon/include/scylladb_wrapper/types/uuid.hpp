#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>

namespace scylladb_wrapper::uuid {
  class UUIDv4 : public Napi::ObjectWrap<UUIDv4> {
  private:
    uint64_t _part1;
    uint64_t _part2;

    Napi::FunctionReference *_constructor;

  public:
    // ------------- C++ -------------
    static Napi::Function GetClass(Napi::Env env);
    UUIDv4(const Napi::CallbackInfo &info);

    static Napi::Object Wrap(Napi::Env env, uint64_t part1, uint64_t part2);

    CassUuid to_cass_uuid() const;
    static bool is_instance_of(const Napi::Object &object);

    const std::string to_string_cpp() const;
    // -------------------------------

    // ------------- JS --------------
    static Napi::Value random(const Napi::CallbackInfo &info);
    static Napi::Value from_string(const Napi::CallbackInfo &info);
    Napi::Value to_string(const Napi::CallbackInfo &info);
    // -------------------------------
  };
}  // namespace scylladb_wrapper::uuid
