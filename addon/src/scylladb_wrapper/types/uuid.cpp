#include <fmt/core.h>

#include <iomanip>
#include <iostream>
#include <random>
#include <scylladb_wrapper/types/uuid.hpp>
#include <sstream>

namespace scylladb_wrapper::uuid {
  Napi::Function UUIDv4::GetClass(Napi::Env env) {
    auto methods = {
        StaticMethod("random", &UUIDv4::random),
        StaticMethod("fromString", &UUIDv4::from_string),
        InstanceMethod("toString", &UUIDv4::to_string),
    };
    Napi::Function func = DefineClass(env, "UUIDv4", methods);
    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    // Create a persistent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    *constructor = Napi::Persistent(func);

    // supress destructor
    constructor->SuppressDestruct();

    // Store the constructor as the add-on instance data. This will allow this
    // add-on to support multiple instances of itself running on multiple worker
    // threads, as well as multiple instances of itself running in different
    // contexts on the same thread.
    //
    // By default, the value set on the environment here will be destroyed when
    // the add-on is unloaded using the `delete` operator, but it is also
    // possible to supply a custom deleter.
    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return func;
  }

  Napi::Object UUIDv4::Wrap(Napi::Env env, uint64_t part1, uint64_t part2) {
    Napi::EscapableHandleScope scope(env);

    Napi::FunctionReference* constructor = env.GetInstanceData<Napi::FunctionReference>();
    Napi::Function ctor = constructor->Value();
    Napi::Object obj = ctor.New({
        Napi::BigInt::New(env, part1),
        Napi::BigInt::New(env, part2),
    });

    obj.Set("uuidType", Napi::String::New(env, "UUIDv4"));

    return scope.Escape(napi_value(obj)).ToObject();
  }

  UUIDv4::UUIDv4(const Napi::CallbackInfo& info) : Napi::ObjectWrap<UUIDv4>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!info.IsConstructCall()) {
      Napi::TypeError::New(env, "Use the new operator to create instances of this type.")
          .ThrowAsJavaScriptException();
    }

    bool* lossless = new bool();
    *lossless = false;
    _part1 = info[0].As<Napi::BigInt>().Uint64Value(lossless);
    _part2 = info[1].As<Napi::BigInt>().Uint64Value(lossless);

    // Check if the third parameters if of the type of this ObjectWrap class <UUIDv4>
  }

  Napi::Value UUIDv4::random(const Napi::CallbackInfo& info) {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<uint64_t> dist;

    auto _part1 = dist(rng);
    auto _part2 = dist(rng);

    // Set the version bits (0100) and the variant bits (10)
    _part1 = (_part1 & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    _part2 = (_part2 & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    // TODO: This wrap method introduces a unnecessary wrap of data, we may create a ISSUE at
    // node-addon-api to see if they can guide us to a better solution later. This new solution may
    // not introduce breaking changes, so the users of this library will not be affected.
    return Wrap(info.Env(), _part1, _part2);
  }

  Napi::Value UUIDv4::from_string(const Napi::CallbackInfo& info) {
    std::string uuid_str = info[0].As<Napi::String>().Utf8Value();

    if (uuid_str.size() != 36 || uuid_str[8] != '-' || uuid_str[13] != '-' || uuid_str[18] != '-'
        || uuid_str[23] != '-') {
      throw std::invalid_argument("Invalid UUID string format.");
    }

    std::stringstream ss(uuid_str);
    uint32_t p1a;
    uint16_t p1b, p1c;
    uint16_t p2a;
    uint64_t p2b;

    char dash;
    ss >> std::hex >> p1a >> dash >> p1b >> dash >> p1c >> dash >> p2a >> dash >> p2b;

    if (ss.fail()) {
      throw std::invalid_argument("Invalid UUID string format.");
    }

    auto _part1 = (static_cast<uint64_t>(p1a) << 32) | (static_cast<uint64_t>(p1b) << 16) | p1c;
    auto _part2 = (static_cast<uint64_t>(p2a) << 48) | p2b;

    return Wrap(info.Env(), _part1, _part2);
  }

  Napi::Value UUIDv4::to_string(const Napi::CallbackInfo& info) {
    std::ostringstream oss;

    oss << std::hex << std::setw(8) << std::setfill('0') << ((_part1 >> 32) & 0xFFFFFFFFULL) << '-';
    oss << std::hex << std::setw(4) << std::setfill('0') << ((_part1 >> 16) & 0xFFFFULL) << '-';
    oss << std::hex << std::setw(4) << std::setfill('0') << (_part1 & 0xFFFFULL) << '-';
    oss << std::hex << std::setw(4) << std::setfill('0') << ((_part2 >> 48) & 0xFFFFULL) << '-';
    oss << std::hex << std::setw(12) << std::setfill('0') << (_part2 & 0xFFFFFFFFFFFFULL);

    return Napi::String::New(info.Env(), oss.str());
  }

  const std::string UUIDv4::to_string_cpp() const {
    std::ostringstream oss;

    oss << std::hex << std::setw(8) << std::setfill('0') << ((_part1 >> 32) & 0xFFFFFFFFULL) << '-';
    oss << std::hex << std::setw(4) << std::setfill('0') << ((_part1 >> 16) & 0xFFFFULL) << '-';
    oss << std::hex << std::setw(4) << std::setfill('0') << (_part1 & 0xFFFFULL) << '-';
    oss << std::hex << std::setw(4) << std::setfill('0') << ((_part2 >> 48) & 0xFFFFULL) << '-';
    oss << std::hex << std::setw(12) << std::setfill('0') << (_part2 & 0xFFFFFFFFFFFFULL);

    return oss.str();
  }

  CassUuid UUIDv4::to_cass_uuid() const {
    CassUuid uuid;
    uuid.time_and_version = _part1;
    uuid.clock_seq_and_node = _part2;
    return uuid;
  }

  bool UUIDv4::is_instance_of(const Napi::Object& object) {
    return object.Has("uuidType")
           && object.Get("uuidType").As<Napi::String>().Utf8Value() == "UUIDv4";
  }
}  // namespace scylladb_wrapper::uuid
