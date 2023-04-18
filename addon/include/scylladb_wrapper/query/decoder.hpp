#pragma once

#include <napi.h>
#include <scylladb/cassandra.h>

namespace scylladb_wrapper::query {
  Napi::Value decoder(const Napi::CallbackInfo& info, const CassResult* result);
}
