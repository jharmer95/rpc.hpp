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

template<>
njson rpc::serialize(const TestObject& obj)
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

template<>
TestObject rpc::deserialize(const njson& serial_obj)
{
    TestObject obj;
    obj.name = serial_obj["name"].get<std::string>();
    obj.age = serial_obj["age"].get<int>();

    std::copy(serial_obj["numbers"].begin(), serial_obj["numbers"].end(), obj.numbers.begin());
    return obj;
}

struct TestObject2 : public TestObject
{
    static njson serialize(const TestObject2& obj)
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

    static TestObject2 deserialize(const njson& serial_obj)
    {
        TestObject2 obj;
        obj.name = serial_obj["name"].get<std::string>();
        obj.age = serial_obj["age"].get<int>();

        std::copy(serial_obj["numbers"].begin(), serial_obj["numbers"].end(), obj.numbers.begin());
        return obj;
    }
};