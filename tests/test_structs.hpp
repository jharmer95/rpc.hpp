///@file test_structs.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Structures/classes for use with rpc.hpp unit tests
///@version 0.2.0.0
///@date 09-09-2020
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020, Jackson Harmer
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

#include <algorithm>
#include <array>
#include <string>

struct TestObject
{
    std::string name;
    int age{};
    std::array<int, 4> numbers{};
};

#if defined(RPC_HPP_NJSON_ENABLED)
template<>
inline njson rpc::serialize(const TestObject& val)
{
    njson obj_j;
    obj_j["name"] = val.name;
    obj_j["age"] = val.age;
    obj_j["numbers"] = njson::array();

    for (const auto& n : val.numbers)
    {
        obj_j["numbers"].push_back(n);
    }

    return obj_j;
}

#    if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
template<>
inline TestObject rpc::deserialize(const njson& serial_obj)
{
    TestObject obj;
    obj.name = serial_obj["name"].get<std::string>();
    obj.age = serial_obj["age"].get<int>();

    std::copy(serial_obj["numbers"].begin(), serial_obj["numbers"].end(), obj.numbers.begin());
    return obj;
}

template<>
inline generic_serial_t rpc::serialize(const TestObject& val)
{
    return to_func(rpc::serialize<njson, TestObject>(val));
}

template<>
inline TestObject rpc::deserialize(const generic_serial_t& serial_obj)
{
    return rpc::deserialize<njson, TestObject>(from_func(serial_obj));
}
#    endif
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
template<>
inline rpdjson_doc rpc::serialize(const TestObject& val)
{
    rpdjson_doc d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    rpdjson_val name_v;
    name_v.SetString(val.name.c_str(), alloc);
    d.AddMember("name", name_v, alloc);

    rpdjson_val age_v;
    age_v.SetInt(val.age);
    d.AddMember("age", age_v, alloc);

    rpdjson_val numbers_v;
    numbers_v.SetArray();

    for (const auto& n : val.numbers)
    {
        numbers_v.PushBack(n, alloc);
    }

    d.AddMember("numbers", numbers_v, alloc);
    return d;
}

template<>
inline TestObject rpc::deserialize(const rpdjson_doc& serial_obj)
{
    TestObject obj;
    const auto name_v = serial_obj.FindMember("name");
    obj.name = std::string(name_v->value.GetString(), name_v->value.GetStringLength());

    const auto age_v = serial_obj.FindMember("age");
    obj.age = age_v->value.GetInt();

    const auto numbers_v = serial_obj.FindMember("numbers");
    const auto& arr = numbers_v->value.GetArray();

    for (unsigned i = 0; i < 4; ++i)
    {
        obj.numbers[i] = arr[i].GetInt();
    }

    return obj;
}
#endif
