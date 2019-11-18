///@file dispatcher.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Example dispatcher.hpp file for use with the example main.cpp file
///@version 0.1.0.0
///@date 11-18-2019
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2019, Jackson Harmer
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
    static nlohmann::json Serialize(const TestStruct& ts)
    {
        nlohmann::json obj_j;
        obj_j["age"] = ts.age;
        obj_j["name"] = std::string(ts.name);
        obj_j["sector"] = ts.sector;
        obj_j["userID"] = ts.userID;
        return obj_j;
    }

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

inline int PrintMyArgs(TestStruct* pts, const int n, const std::string& msg)
{
    std::cout << "age: " << pts->age << '\n';
    std::cout << "name: " << pts->name << '\n';
    std::cout << "sector: " << pts->sector << '\n';
    std::cout << "userID: " << pts->userID << "\n\n";
    std::cout << "n: " << n << '\n';
    std::cout << "msg: " << msg << '\n';

    return 2;
}

inline bool TestMyArgs(TestStruct* pts, const double f)
{
    return pts->age > 4 && f < 5.5;
}

class Dispatcher
{
public:
    std::string Run(const std::string& funcName, const nlohmann::json& obj_j);

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

    template<>
    static nlohmann::json Serialize<TestStruct>(const TestStruct& ts)
    {
        nlohmann::json obj_j;
        obj_j["age"] = ts.age;
        obj_j["name"] = std::string(ts.name);
        obj_j["sector"] = ts.sector;
        obj_j["userID"] = ts.userID;
        return obj_j;
    }

    template<>
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
