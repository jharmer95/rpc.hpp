#ifndef RPC_HPP_ADAPTERS_ADL_SET_HPP
#define RPC_HPP_ADAPTERS_ADL_SET_HPP

#include "../serializer.hpp"

#include <set>

template<typename Adapter, typename Key, typename Compare, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::set<Key, Compare, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename Key, typename Compare, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::set<Key, Compare, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename Key, typename Compare, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::multiset<Key, Compare, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename Key, typename Compare, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::multiset<Key, Compare, Alloc>& val)
{
    ser.as_array("", val);
}

#endif
