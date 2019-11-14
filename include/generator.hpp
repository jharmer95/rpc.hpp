#pragma once

#include <type_traits>

namespace rpc
{
template <typename T>
struct is_generator : public std::false_type {};

template <typename T>
inline constexpr bool is_generator_v = is_generator<T>::value;
} // namespace rpc
