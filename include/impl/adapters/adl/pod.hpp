#ifndef RPC_HPP_ADAPTERS_ADL_POD_HPP
#define RPC_HPP_ADAPTERS_ADL_POD_HPP

#include "../serializer.hpp"

#include <type_traits>

// Overloads for common types
template<typename Adapter>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const bool val)
{
    ser.as_bool("", val);
}

template<typename Adapter>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, bool& val)
{
    ser.as_bool("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && (!std::is_same_v<T, bool>)), bool> = true>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_int("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && (!std::is_same_v<T, bool>)), bool> = true>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_int("", val);
}

template<typename Adapter, typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_float("", val);
}

template<typename Adapter, typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_float("", val);
}

#endif
