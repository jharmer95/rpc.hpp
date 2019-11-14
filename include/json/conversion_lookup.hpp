#pragma once

#include "nlohmann/json/json.hpp"
#include "generator.hpp"

#include <cstdint>
#include <functional>
#include <map>
#include <type_traits>

namespace njson = nlohmann;

namespace rpc::json
{
template <typename T>
using JSON_Generator = std::function<njson::json(T)>;

struct is_generator<JSON_Generator> : public std::true_type {};
} // namespace rpc::json
