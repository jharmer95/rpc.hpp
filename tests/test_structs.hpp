///@file test_structs.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Structures/classes for use with rpc.hpp unit tests
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

#if defined(RPC_HPP_ENABLE_BITSERY)
#    include <rpc_adapters/rpc_bitsery.hpp>

using rpc::adapters::bitsery_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include <rpc_adapters/rpc_boost_json.hpp>

using rpc::adapters::boost_json_adapter;
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#    include <rpc_adapters/rpc_njson.hpp>

using rpc::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include <rpc_adapters/rpc_rapidjson.hpp>

using rpc::adapters::rapidjson_adapter;
#endif

#include <algorithm>
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
};

#if defined(RPC_HPP_ENABLE_BITSERY)
template<typename S>
void serialize(S& s, ComplexObject& val)
{
    s.value4b(val.id);
    s.text1b(val.name, 255);
    s.value1b(val.flag1);
    s.value1b(val.flag2);
    s.container1b(val.vals);
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
template<>
template<>
inline rpc::adapters::boost_json::value_t boost_json_adapter::serialize(const ComplexObject& val)
{
    rpc::adapters::boost_json::object_t obj_j;
    obj_j["id"] = val.id;
    obj_j["name"] = val.name;
    obj_j["flag1"] = val.flag1;
    obj_j["flag2"] = val.flag2;
    boost::json::array arr;

    for (const auto v : val.vals)
    {
        arr.push_back(v);
    }

    obj_j["vals"] = std::move(arr);

    return obj_j;
}

template<>
template<>
inline ComplexObject boost_json_adapter::deserialize(
    const rpc::adapters::boost_json::value_t& serial_obj)
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

#if defined(RPC_HPP_ENABLE_NJSON)
template<>
template<>
inline rpc::adapters::njson::njson_t njson_adapter::serialize(const ComplexObject& val)
{
    adapters::njson::njson_t obj_j;
    obj_j["id"] = val.id;
    obj_j["name"] = val.name;
    obj_j["flag1"] = val.flag1;
    obj_j["flag2"] = val.flag2;
    obj_j["vals"] = val.vals;

    return obj_j;
}

template<>
template<>
inline ComplexObject njson_adapter::deserialize(const adapters::njson::njson_t& serial_obj)
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

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
template<>
template<>
inline rpc::adapters::rapidjson::doc_t rapidjson_adapter::serialize(const ComplexObject& val)
{
    adapters::rapidjson::doc_t d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    adapters::rapidjson::value_t id_v;
    id_v.SetInt(val.id);
    d.AddMember("id", id_v, alloc);

    adapters::rapidjson::value_t name_v;
    name_v.SetString(val.name.c_str(), alloc);
    d.AddMember("name", name_v, alloc);

    adapters::rapidjson::value_t flag1_v;
    flag1_v.SetBool(val.flag1);
    d.AddMember("flag1", flag1_v, alloc);

    adapters::rapidjson::value_t flag2_v;
    flag2_v.SetBool(val.flag2);
    d.AddMember("flag2", flag2_v, alloc);

    adapters::rapidjson::value_t vals_v;
    vals_v.SetArray();

    for (uint8_t i = 0; i < 12; ++i)
    {
        vals_v.PushBack(val.vals[i], alloc);
    }

    d.AddMember("vals", vals_v, alloc);
    return d;
}

template<>
template<>
inline ComplexObject rapidjson_adapter::deserialize(const adapters::rapidjson::doc_t& serial_obj)
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
