#ifndef RPC_HPP_ADAPTERS_ADL_STRING_HPP
#define RPC_HPP_ADAPTERS_ADL_STRING_HPP

#include "../serializer.hpp"

#include <string>
#include <string_view>

template<typename Adapter>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, false>& ser, const std::string_view val)
{
    ser.as_string("", val);
}

template<typename Adapter>
void serialize(rpc_hpp::adapters::serializer_base<Adapter, true>& ser, std::string& val)
{
    ser.as_string("", val);
}

#endif
