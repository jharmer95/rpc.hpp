#pragma once

// Standard Library
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// 3rd Party
#include <asio.hpp>

#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <bitsery/bitsery.h>
#  include <bitsery/adapter/buffer.h>
#  include <bitsery/ext/std_tuple.h>
#  include <bitsery/traits/array.h>
#  include <bitsery/traits/string.h>
#  include <bitsery/traits/vector.h>
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  include <boost/json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#  include <nlohmann/json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  include <rapidjson/document.h>
#  include <rapidjson/stringbuffer.h>
#  include <rapidjson/writer.h>
#endif