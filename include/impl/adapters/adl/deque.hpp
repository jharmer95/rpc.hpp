#ifndef RPC_HPP_ADAPTERS_ADL_DEQUE_HPP
#define RPC_HPP_ADAPTERS_ADL_DEQUE_HPP

#include "../serializer.hpp"

#include <deque>

template<typename Adapter, typename T, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::deque<T, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename T, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::deque<T, Alloc>& val)
{
    ser.as_array("", val);
}

#endif
