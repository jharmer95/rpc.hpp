///@file test_structs.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Structures/classes for use with rpc.hpp unit tests
///@version 0.2.3
///@date 02-24-2021
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

#if defined(RPC_HPP_NJSON_ENABLED)
#    include "rpc_adapters/rpc_njson.hpp"
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
#    include "rpc_adapters/rpc_rapidjson.hpp"
#endif

#if defined(RPC_HPP_BOOST_JSON_ENABLED)
#    include "rpc_adapters/rpc_boost_json.hpp"
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>

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

#if defined(RPC_HPP_NJSON_ENABLED)
template<>
inline njson rpc::serialize<njson_serial_t>(const TestMessage& val)
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
inline TestMessage rpc::deserialize<njson_serial_t>(const njson& serial_obj)
{
    TestMessage mesg;
    mesg.flag1 = serial_obj["flag1"].get<bool>();
    mesg.flag2 = serial_obj["flag2"].get<bool>();
    mesg.id = serial_obj["id"].get<int>();
    mesg.data_sz = serial_obj["data_sz"].get<uint8_t>();
    std::copy(serial_obj["data"].begin(), serial_obj["data"].begin() + mesg.data_sz, mesg.data);
    return mesg;
}

template<>
inline njson rpc::serialize<njson_serial_t>(const ComplexObject& val)
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
inline ComplexObject rpc::deserialize<njson_serial_t>(const njson& serial_obj)
{
    ComplexObject cx;
    cx.id = serial_obj["id"].get<int>();
    cx.name = serial_obj["name"].get<std::string>();
    cx.flag1 = serial_obj["flag1"].get<bool>();
    cx.flag2 = serial_obj["flag2"].get<bool>();
    const auto& vals = serial_obj["vals"];

    if (vals.size() > 12)
    {
        std::copy(vals.begin(), vals.begin() + 12, cx.vals.begin());
    }
    else
    {
        std::copy(vals.begin(), vals.end(), cx.vals.begin());
    }

    return cx;
}

#    if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
template<>
inline byte_vec rpc::serialize<generic_serial_t>(const TestMessage& val)
{
    return to_func(rpc::serialize<njson_serial_t, TestMessage>(val));
}

template<>
inline TestMessage rpc::deserialize<generic_serial_t>(const byte_vec& serial_obj)
{
    return rpc::deserialize<njson_serial_t, TestMessage>(from_func(serial_obj));
}

template<>
inline byte_vec rpc::serialize<generic_serial_t>(const ComplexObject& val)
{
    return to_func(rpc::serialize<njson_serial_t, ComplexObject>(val));
}

template<>
inline ComplexObject rpc::deserialize<generic_serial_t>(const byte_vec& serial_obj)
{
    return rpc::deserialize<njson_serial_t, ComplexObject>(from_func(serial_obj));
}
#    endif
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
template<>
inline rpdjson_doc rpc::serialize<rpdjson_serial_t>(const TestMessage& val)
{
    rpdjson_doc d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    rpdjson_val flag1_v;
    flag1_v.SetBool(val.flag1);
    d.AddMember("flag1", flag1_v, alloc);

    rpdjson_val flag2_v;
    flag2_v.SetBool(val.flag2);
    d.AddMember("flag2", flag2_v, alloc);

    rpdjson_val id_v;
    id_v.SetInt(val.id);
    d.AddMember("id", id_v, alloc);

    rpdjson_val data_sz_v;
    data_sz_v.SetUint(val.data_sz);
    d.AddMember("data_sz", data_sz_v, alloc);

    rpdjson_val data_v;
    data_v.SetArray();

    for (uint8_t i = 0; i < val.data_sz; ++i)
    {
        data_v.PushBack(val.data[i], alloc);
    }

    d.AddMember("data", data_v, alloc);
    return d;
}

template<>
inline TestMessage rpc::deserialize<rpdjson_serial_t>(const rpdjson_val& serial_obj)
{
    TestMessage obj;
    const auto flag1_v = serial_obj.FindMember("flag1");
    obj.flag1 = flag1_v->value.GetBool();

    const auto flag2_v = serial_obj.FindMember("flag2");
    obj.flag2 = flag2_v->value.GetBool();

    const auto id_v = serial_obj.FindMember("id");
    obj.id = id_v->value.GetInt();

    const auto data_sz_v = serial_obj.FindMember("data_sz");
    obj.data_sz = static_cast<uint8_t>(data_sz_v->value.GetUint());

    const auto data_v = serial_obj.FindMember("data");
    const auto& arr = data_v->value.GetArray();

    for (unsigned i = 0; i < obj.data_sz; ++i)
    {
        obj.data[i] = arr[i].GetInt();
    }

    return obj;
}

template<>
inline rpdjson_doc rpc::serialize<rpdjson_serial_t>(const ComplexObject& val)
{
    rpdjson_doc d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    rpdjson_val id_v;
    id_v.SetInt(val.id);
    d.AddMember("id", id_v, alloc);

    rpdjson_val name_v;
    name_v.SetString(val.name.c_str(), alloc);
    d.AddMember("name", name_v, alloc);

    rpdjson_val flag1_v;
    flag1_v.SetBool(val.flag1);
    d.AddMember("flag1", flag1_v, alloc);

    rpdjson_val flag2_v;
    flag2_v.SetBool(val.flag2);
    d.AddMember("flag2", flag2_v, alloc);

    rpdjson_val vals_v;
    vals_v.SetArray();

    for (uint8_t i = 0; i < 12; ++i)
    {
        vals_v.PushBack(val.vals[i], alloc);
    }

    d.AddMember("vals", vals_v, alloc);
    return d;
}

template<>
inline ComplexObject rpc::deserialize<rpdjson_serial_t>(const rpdjson_val& serial_obj)
{
    ComplexObject obj;

    const auto id_v = serial_obj.FindMember("id");
    obj.id = id_v->value.GetInt();

    const auto name_v = serial_obj.FindMember("name");
    obj.name = std::string(name_v->value.GetString(), name_v->value.GetStringLength());

    const auto flag1_v = serial_obj.FindMember("flag1");
    obj.flag1 = flag1_v->value.GetBool();

    const auto flag2_v = serial_obj.FindMember("flag2");
    obj.flag2 = flag2_v->value.GetBool();

    const auto vals_v = serial_obj.FindMember("vals");
    const auto& arr = vals_v->value.GetArray();

    for (unsigned i = 0; i < 12; ++i)
    {
        obj.vals[i] = static_cast<uint8_t>(arr[i].GetUint());
    }

    return obj;
}
#endif

#if defined(RPC_HPP_BOOST_JSON_ENABLED)
template<>
inline bjson_val rpc::serialize<bjson_serial_t>(const TestMessage& val)
{
    bjson_obj obj_j;
    obj_j["flag1"] = val.flag1;
    obj_j["flag2"] = val.flag2;
    obj_j["id"] = val.id;
    obj_j["data"] = bjson::array{};
    auto& data = obj_j["data"].as_array();
    obj_j["data_sz"] = val.data_sz;

    for (uint8_t i = 0; i < val.data_sz; ++i)
    {
        data.push_back(val.data[i]);
    }

    return obj_j;
}

template<>
inline TestMessage rpc::deserialize<bjson_serial_t>(const bjson_val& serial_obj)
{
    TestMessage mesg;
    mesg.flag1 = serial_obj.at("flag1").get_bool();
    mesg.flag2 = serial_obj.at("flag2").get_bool();
    mesg.id = static_cast<int>(serial_obj.at("id").get_int64());
    mesg.data_sz = static_cast<uint8_t>(serial_obj.at("data_sz").get_int64());
    const auto& data = serial_obj.at("data").as_array();

    for (uint8_t i = 0; i < mesg.data_sz; ++i)
    {
        mesg.data[i] = static_cast<int>(data[i].get_int64());
    }

    return mesg;
}

template<>
inline bjson_val rpc::serialize<bjson_serial_t>(const ComplexObject& val)
{
    bjson_obj obj_j;
    obj_j["id"] = val.id;
    obj_j["name"] = val.name;
    obj_j["flag1"] = val.flag1;
    obj_j["flag2"] = val.flag2;
    bjson::array arr;

    for (const auto v : val.vals)
    {
        arr.push_back(v);
    }

    obj_j["vals"] = std::move(arr);

    return obj_j;
}

template<>
inline ComplexObject rpc::deserialize<bjson_serial_t>(const bjson_val& serial_obj)
{
    ComplexObject cx;
    cx.id = static_cast<int>(serial_obj.at("id").get_int64());
    cx.name = serial_obj.at("name").get_string().c_str();
    cx.flag1 = serial_obj.at("flag1").get_bool();
    cx.flag2 = serial_obj.at("flag2").get_bool();
    const auto& vals = serial_obj.at("vals").as_array();

    if (vals.size() > 12)
    {
        for (size_t i = 0; i < 12; ++i)
        {
            cx.vals[i] = static_cast<uint8_t>(vals[i].get_int64());
        }
    }
    else
    {
        for (size_t i = 0; i < vals.size(); ++i)
        {
            cx.vals[i] = static_cast<uint8_t>(vals[i].get_int64());
        }
    }

    return cx;
}
#endif
