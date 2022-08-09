#pragma once

// Standard Library
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

#if defined(__cpp_lib_filesystem)
#  include <filesystem>
#else
#  include <experimental/filesystem>
#endif

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
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
