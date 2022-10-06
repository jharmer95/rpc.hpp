#ifndef RPC_HPP_ADAPTERS_ADL_ARRAY_HPP
#define RPC_HPP_ADAPTERS_ADL_ARRAY_HPP

#include "../serializer.hpp"

#include <array>
#include <cstddef>

template<typename Adapter, typename T, size_t N>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::array<T, N>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename T, size_t N>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::array<T, N>& val)
{
    ser.as_array("", val);
}

#endif
