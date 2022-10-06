#ifndef RPC_HPP_ADAPTERS_ADL_PAIR_HPP
#define RPC_HPP_ADAPTERS_ADL_PAIR_HPP

#include "../serializer.hpp"

#include <utility>

template<typename Adapter, typename T1, typename T2>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::pair<T1, T2>& val)
{
    ser.as_tuple("", val);
}

template<typename Adapter, typename T1, typename T2>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::pair<T1, T2>& val)
{
    ser.as_tuple("", val);
}

#endif
