#ifndef RPC_HPP_ADAPTERS_ADL_UNORDERED_MAP_HPP
#define RPC_HPP_ADAPTERS_ADL_UNORDERED_MAP_HPP

#include "../serializer.hpp"

#include <unordered_map>

template<typename Adapter, typename Key, typename T, typename Hash, typename KeyEqual,
    typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& val)
{
    ser.as_map("", val);
}

template<typename Adapter, typename Key, typename T, typename Hash, typename KeyEqual,
    typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser,
    std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& val)
{
    ser.as_map("", val);
}

template<typename Adapter, typename Key, typename T, typename Hash, typename KeyEqual,
    typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::unordered_multimap<Key, T, Hash, KeyEqual, Alloc>& val)
{
    ser.as_multimap("", val);
}

template<typename Adapter, typename Key, typename T, typename Hash, typename KeyEqual,
    typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser,
    std::unordered_multimap<Key, T, Hash, KeyEqual, Alloc>& val)
{
    ser.as_multimap("", val);
}

#endif
