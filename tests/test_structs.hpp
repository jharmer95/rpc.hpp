#pragma once

#include <algorithm>
#include <array>
#include <string>

struct TestObject
{
    std::string name;
    int age;
    std::array<int, 4> numbers;
};

#if defined(RPC_HPP_NJSON_ENABLED)
template<>
inline njson rpc::serialize(const TestObject& obj)
{
    njson obj_j;
    obj_j["name"] = obj.name;
    obj_j["age"] = obj.age;
    obj_j["numbers"] = njson::array();

    for (const auto& n : obj.numbers)
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
inline generic_serial_t rpc::serialize(const TestObject& obj)
{
    return to_func(rpc::serialize<njson, TestObject>(obj));
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
inline rpdjson_doc rpc::serialize(const TestObject& obj)
{
    rpdjson_doc d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    rpdjson_val name_v;
    name_v.SetString(obj.name.c_str(), alloc);
    d.AddMember("name", name_v, alloc);

    rpdjson_val age_v;
    age_v.SetInt(obj.age);
    d.AddMember("age", age_v, alloc);

    rpdjson_val numbers_v;
    numbers_v.SetArray();

    for (const auto& n : obj.numbers)
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
    obj.name = name_v->value.GetString();

    const auto age_v = serial_obj.FindMember("age");
    obj.age = age_v->value.GetInt();

    const auto numbers_v = serial_obj.FindMember("numbers");
    const auto& arr = numbers_v->value.GetArray();

    for (unsigned i = 0; i < 4; ++i)
    {
        obj.numbers[i++] = arr[i].GetInt();
    }

    return obj;
}
#endif
