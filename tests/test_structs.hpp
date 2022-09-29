///@file test_structs.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Structures/classes for use with rpc.hpp unit tests
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#pragma once

#include <rpc.hpp>

#if defined(RPC_HPP_BENCH_RPCLIB)
#  include <rpc/client.h>
#endif

struct ComplexObject
{
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};

#if defined(RPC_HPP_BENCH_RPCLIB)
    MSGPACK_DEFINE_ARRAY(id, name, flag1, flag2, vals)
#endif
};

template<typename S>
void serialize(rpc_hpp::adapters::generic_serializer<S>& ser, ComplexObject& cx_obj)
{
    ser.as_int("id", cx_obj.id);
    ser.as_string("name", cx_obj.name);
    ser.as_bool("flag1", cx_obj.flag1);
    ser.as_bool("flag2", cx_obj.flag2);
    ser.as_array("val", cx_obj.vals);
}

template<typename T>
struct ValueRange
{
    static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type");

    // TODO: Allow using serialize as a member function
    // template<typename Adapter, bool Deserialize>
    // void serialize(rpc_hpp::adapters::serializer<Adapter, Deserialize>& ser)
    // {
    //     if constexpr (std::is_floating_point_v<T>)
    //     {
    //         ser.as_float("min", min);
    //         ser.as_float("max", max);
    //     }
    //     else
    //     {
    //         ser.as_int("min", min);
    //         ser.as_int("max", max);
    //     }
    // }

    T min;
    T max;

#if defined(RPC_HPP_BENCH_RPCLIB)
    MSGPACK_DEFINE_ARRAY(min, max)
#endif
};

template<typename S, typename T>
void serialize(rpc_hpp::adapters::generic_serializer<S>& ser, ValueRange<T>& range)
{
    if constexpr (std::is_floating_point_v<T>)
    {
        ser.as_float("min", range.min);
        ser.as_float("max", range.max);
    }
    else
    {
        ser.as_int("min", range.min);
        ser.as_int("max", range.max);
    }
}
