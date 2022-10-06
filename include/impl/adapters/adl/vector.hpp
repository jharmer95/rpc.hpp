#ifndef RPC_HPP_ADAPTERS_ADL_VECTOR_HPP
#define RPC_HPP_ADAPTERS_ADL_VECTOR_HPP

#include "../serializer.hpp"

#include <vector>

template<typename Adapter, typename T, typename Alloc>
void serialize(
    rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::vector<T, Alloc>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, typename T, typename Alloc>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::vector<T, Alloc>& val)
{
    ser.as_array("", val);
}

#endif
