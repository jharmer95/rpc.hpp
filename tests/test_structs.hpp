///@file test_structs.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Structures/classes for use with rpc.hpp unit tests
///@version 0.3.3
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2021, Jackson Harmer
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

#if defined(RPC_HPP_ENABLE_JSON)
#    if defined(RPC_HPP_JSON_USE_NJSON)
#        include "rpc_adapters/rpc_njson.hpp"
#    endif
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>

using njson = nlohmann::json;

struct TestMessage
{
    bool flag1{};
    bool flag2{};
    int id{};
    int data[256]{};
    uint8_t data_sz{};

    [[nodiscard]] bool operator==(const TestMessage& other) const noexcept
    {
        if (flag1 != other.flag1 || flag2 != other.flag2 || id != other.id
            || data_sz != other.data_sz)
        {
            return false;
        }

        return std::memcmp(data, other.data, data_sz) == 0;
    }
};

struct ComplexObject
{
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};
};

#if defined(RPC_HPP_JSON_USE_NJSON)
template<>
template<>
inline njson rpc::adapters::njson_adapter::serialize(const TestMessage& val)
{
    njson obj_j;
    obj_j["flag1"] = val.flag1;
    obj_j["flag2"] = val.flag2;
    obj_j["id"] = val.id;
    obj_j["data"] = njson::array();
    obj_j["data_sz"] = val.data_sz;

    for (uint8_t i = 0; i < val.data_sz; ++i)
    {
        obj_j["data"].push_back(val.data[i]);
    }

    return obj_j;
}

template<>
template<>
inline TestMessage rpc::adapters::njson_adapter::deserialize(const njson& serial_obj)
{
    TestMessage mesg;
    mesg.flag1 = serial_obj["flag1"].get<bool>();
    mesg.flag2 = serial_obj["flag2"].get<bool>();
    mesg.id = serial_obj["id"].get<int>();
    mesg.data_sz = serial_obj["data_sz"].get<uint8_t>();
    std::copy_n(serial_obj["data"].begin(), mesg.data_sz, mesg.data);
    return mesg;
}

template<>
template<>
inline njson rpc::adapters::njson_adapter::serialize(const ComplexObject& val)
{
    njson obj_j;
    obj_j["id"] = val.id;
    obj_j["name"] = val.name;
    obj_j["flag1"] = val.flag1;
    obj_j["flag2"] = val.flag2;
    obj_j["vals"] = val.vals;

    return obj_j;
}

template<>
template<>
inline ComplexObject rpc::adapters::njson_adapter::deserialize(const njson& serial_obj)
{
    ComplexObject cx;
    cx.id = serial_obj["id"].get<int>();
    cx.name = serial_obj["name"].get<std::string>();
    cx.flag1 = serial_obj["flag1"].get<bool>();
    cx.flag2 = serial_obj["flag2"].get<bool>();
    const auto& vals = serial_obj["vals"];

    if (vals.size() > 12)
    {
        std::copy_n(vals.begin(), 12, cx.vals.begin());
    }
    else
    {
        std::copy(vals.begin(), vals.end(), cx.vals.begin());
    }

    return cx;
}
#endif
