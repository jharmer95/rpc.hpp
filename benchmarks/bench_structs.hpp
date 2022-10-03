#pragma once

#include <array>
#include <cstdint>
#include <string>

struct ComplexObject
{
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};

#ifdef MSGPACK_DEFINE_ARRAY
    MSGPACK_DEFINE_ARRAY(id, name, flag1, flag2, vals)
#endif
};

#ifdef RPC_HPP
template<typename S>
void serialize(rpc_hpp::adapters::generic_serializer<S>& ser, ComplexObject& cx_obj)
{
    ser.as_int("id", cx_obj.id);
    ser.as_string("name", cx_obj.name);
    ser.as_bool("flag1", cx_obj.flag1);
    ser.as_bool("flag2", cx_obj.flag2);
    ser.as_array("val", cx_obj.vals);
}
#endif
