#ifndef RPC_HPP_ADAPTERS_ADL_UNORDERED_SET_HPP
#define RPC_HPP_ADAPTERS_ADL_UNORDERED_SET_HPP

#include "../serializer.hpp"

#include <unordered_set>

template<typename Adapter, typename Key, typename Hash, typename KeyEqual, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::unordered_set<Key, Hash, KeyEqual, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename Key, typename Hash, typename KeyEqual, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser,
    std::unordered_set<Key, Hash, KeyEqual, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename Key, typename Hash, typename KeyEqual, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser,
    const std::unordered_multiset<Key, Hash, KeyEqual, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename Key, typename Hash, typename KeyEqual, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser,
    std::unordered_multiset<Key, Hash, KeyEqual, Alloc>& val)
{
    ser.as_array("", val);
}

#endif
