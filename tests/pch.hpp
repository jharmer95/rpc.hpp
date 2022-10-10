#pragma once

// Standard Library
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <forward_list>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// 3rd Party
#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <bitsery/bitsery.h>
#  include <bitsery/adapter/buffer.h>
#  include <bitsery/ext/std_map.h>
#  include <bitsery/ext/std_optional.h>
#  include <bitsery/ext/std_set.h>
#  include <bitsery/ext/std_tuple.h>
#  include <bitsery/traits/array.h>
#  include <bitsery/traits/core/traits.h>
#  include <bitsery/traits/deque.h>
#  include <bitsery/traits/forward_list.h>
#  include <bitsery/traits/list.h>
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
