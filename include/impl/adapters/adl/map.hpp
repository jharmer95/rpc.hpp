#ifndef RPC_HPP_ADAPTERS_ADL_MAP_HPP
#define RPC_HPP_ADAPTERS_ADL_MAP_HPP

#include "../serializer.hpp"

#include <map>

template<typename Adapter, typename Key, typename T, typename Compare, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::map<Key, T, Compare, Alloc>& val)
{
    ser.as_map("", val);
}

template<typename Adapter, typename Key, typename T, typename Compare, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::map<Key, T, Compare, Alloc>& val)
{
    ser.as_map("", val);
}

template<typename Adapter, typename Key, typename T, typename Compare, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::multimap<Key, T, Compare, Alloc>& val)
{
    ser.as_multimap("", val);
}

template<typename Adapter, typename Key, typename T, typename Compare, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser,
    std::multimap<Key, T, Compare, Alloc>& val)
{
    ser.as_multimap("", val);
}

#endif
