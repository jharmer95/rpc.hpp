#pragma once

#include <nlohmann/json/json.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct TestStruct
{
    int age{};
    int sector{};
    unsigned long userID{};
    char name[255]{};

    // TODO: Get templated Serialize/Deserialize working
    // static nlohmann::json Serialize(const TestStruct& ts)
    // {
    //     nlohmann::json obj_j;
    //     obj_j["age"] = ts.age;
    //     obj_j["name"] = std::string(ts.name);
    //     obj_j["sector"] = ts.sector;
    //     obj_j["userID"] = ts.userID;

    //     return obj_j;
    // }

    // TODO: move array stuff to the library side so that return type is just TestStruct
    static std::vector<TestStruct> DeSerialize(const nlohmann::json& obj_j)
    {
        std::vector<TestStruct> vec;
        const size_t sz = obj_j.is_array() ? obj_j.size() : 1;

        for (size_t i = 0; i < sz; ++i)
        {
            TestStruct ts;
            ts.age = obj_j["age"].get<int>();
            const auto nmStr = obj_j["name"].get<std::string>();
            std::copy(nmStr.begin(), nmStr.end(), ts.name);
            ts.sector = obj_j["sector"].get<int>();
            ts.userID = obj_j["userID"].get<unsigned long>();
            vec.push_back(ts);
        }

        return vec;
    }
};

int PrintMyArgs(TestStruct* pts, const int n, const std::string& msg)
{
    std::cout << "age: " << pts->age << '\n';
    std::cout << "name: " << pts->name << '\n';
    std::cout << "sector: " << pts->sector << '\n';
    std::cout << "userID: " << pts->userID << "\n\n";
    std::cout << "n: " << n << '\n';
    std::cout << "msg: " << msg << '\n';

    return 2;
}

bool TestMyArgs(TestStruct* pts, const double f)
{
    return pts->age > 4 && f < 5.5;
}

class Dispatcher// : public rpc::IDispatcher
{
public:
    auto GetFunction(const std::string& sv) const
    {
        if (sv == "PrintMyArgs")
        {
            return m_printArgs;
        }
    }

    template<typename T>
    static nlohmann::json Serialize(const T& obj)
    {
        throw std::logic_error("Type has not been provided with a Serialize method!");
    }

    template<typename T>
    static std::vector<T> DeSerialize(const nlohmann::json& obj_j)
    {
        throw std::logic_error("Type has not been provided with a DeSerialize method!");
    }

    template <>
    static nlohmann::json Serialize<TestStruct>(const TestStruct& ts)
    {
        nlohmann::json obj_j;
        obj_j["age"] = ts.age;
        obj_j["name"] = std::string(ts.name);
        obj_j["sector"] = ts.sector;
        obj_j["userID"] = ts.userID;

        return obj_j;
    }

    template <>
    static std::vector<TestStruct> DeSerialize<TestStruct>(const nlohmann::json& obj_j)
    {
        std::vector<TestStruct> vec;
        const size_t sz = obj_j.is_array() ? obj_j.size() : 1;

        for (size_t i = 0; i < sz; ++i)
        {
            TestStruct ts;
            ts.age = obj_j["age"].get<int>();
            const auto nmStr = obj_j["name"].get<std::string>();
            std::copy(nmStr.begin(), nmStr.end(), ts.name);
            ts.sector = obj_j["sector"].get<int>();
            ts.userID = obj_j["userID"].get<unsigned long>();
            vec.push_back(ts);
        }

        return vec;
    }

private:
    std::function<int(TestStruct*, int, std::string)> m_printArgs = PrintMyArgs;
    std::function<bool(TestStruct*, double)> m_testArgs = TestMyArgs;
};
