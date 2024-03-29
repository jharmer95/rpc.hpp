#pragma once

// Standard Library
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// 3rd Party
#include <asio.hpp>

#if defined(RPC_HPP_ENABLE_BITSERY)
#    include <bitsery/bitsery.h>
#    include <bitsery/adapter/buffer.h>
#    include <bitsery/ext/std_tuple.h>
#    include <bitsery/traits/array.h>
#    include <bitsery/traits/string.h>
#    include <bitsery/traits/vector.h>
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include <boost/json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#    include <nlohmann/json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include <rapidjson/document.h>
#    include <rapidjson/stringbuffer.h>
#    include <rapidjson/writer.h>
#endif

#if defined(RPC_HPP_BENCH_GRPC)
#    include <grpc/grpc.h>
#    include <grpcpp/channel.h>
#    include <grpcpp/client_context.h>
#    include <grpcpp/create_channel.h>
#    include <grpcpp/security/credentials.h>
#    include "grpc/benchmark.grpc.pb.h"
#endif

#if defined(RPC_HPP_BENCH_RPCLIB)
#    include <rpc/client.h>
#endif
