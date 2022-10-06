#ifndef RPC_HPP_ADAPTERS_ADL_OPTIONAL_HPP
#define RPC_HPP_ADAPTERS_ADL_OPTIONAL_HPP

#include "../serializer.hpp"

#include <optional>

template<typename Adapter, typename T>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::optional<T>& val)
{
    ser.as_optional("", val);
}

template<typename Adapter, typename T>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::optional<T>& val)
{
    ser.as_optional("", val);
}

#endif
