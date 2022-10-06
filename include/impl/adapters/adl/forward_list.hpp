#ifndef RPC_HPP_ADAPTERS_ADL_FORWARD_LIST_HPP
#define RPC_HPP_ADAPTERS_ADL_FORWARD_LIST_HPP

#include "../serializer.hpp"

#include <forward_list>

template<typename Adapter, typename T, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::forward_list<T, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename T, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::forward_list<T, Alloc>& val)
{
    ser.as_array("", val);
}

#endif
