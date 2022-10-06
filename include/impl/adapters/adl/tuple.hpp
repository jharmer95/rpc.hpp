#ifndef RPC_HPP_ADAPTERS_ADL_TUPLE_HPP
#define RPC_HPP_ADAPTERS_ADL_TUPLE_HPP

#include "../serializer.hpp"

#include <tuple>

template<typename Adapter, typename... Args>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::tuple<Args...>& val)
{
    ser.as_tuple("", val);
}

template<typename Adapter, typename... Args>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::tuple<Args...>& val)
{
    ser.as_tuple("", val);
}

#endif
