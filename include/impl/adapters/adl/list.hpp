#ifndef RPC_HPP_ADAPTERS_ADL_LIST_HPP
#define RPC_HPP_ADAPTERS_ADL_LIST_HPP

#include "../serializer.hpp"

#include <list>

template<typename Adapter, typename T, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::list<T, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename T, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::list<T, Alloc>& val)
{
    ser.as_array("", val);
}

#endif
