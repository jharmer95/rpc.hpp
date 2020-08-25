///@file bench_funcs.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Functions for benchmarking rpc
///@version 0.1.0.0
///@date 02-07-2020
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

#include "rpc.hpp"
#include "rpc_adapters/rpc_njson.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct TestMessage
{
    bool Flag1{};
    bool Flag2{};
    int ID{};
    int Data[256]{};
    uint8_t DataSize{};

    [[nodiscard]] bool operator==(const TestMessage& other) const noexcept
    {
        if (Flag1 != other.Flag1 || Flag2 != other.Flag2 || ID != other.ID
            || DataSize != other.DataSize)
        {
            return false;
        }

        return memcmp(Data, other.Data, DataSize) == 0;
    }
};

template<>
inline njson rpc::serialize(const TestMessage& mesg) RPC_HPP_EXCEPT
{
    njson obj_j;
    obj_j["ID"] = mesg.ID;
    obj_j["Flag1"] = mesg.Flag1;
    obj_j["Flag2"] = mesg.Flag2;
    obj_j["DataSize"] = mesg.DataSize;

    if (mesg.DataSize > 0)
    {
        std::vector<int> tmpVec(mesg.Data, mesg.Data + mesg.DataSize);
        obj_j["Data"] = tmpVec;
    }
    else
    {
        obj_j["Data"] = njson::array();
    }

    return obj_j;
}

template<>
inline TestMessage rpc::deserialize(const njson& obj_j) RPC_HPP_EXCEPT
{
    TestMessage mesg;
    mesg.ID = obj_j["ID"].get<int>();
    mesg.Flag1 = obj_j["Flag1"].get<bool>();
    mesg.Flag2 = obj_j["Flag2"].get<bool>();
    mesg.DataSize = obj_j["DataSize"].get<uint8_t>();
    const auto sData = obj_j["Data"].get<std::vector<int>>();
    std::copy(sData.begin(), sData.begin() + mesg.DataSize, mesg.Data);
    return mesg;
}

int ReadMessages(TestMessage* mesgBuf, int* numMesgs)
{
    std::ifstream file_in("bus.txt");
    std::stringstream ss;

    std::string s;
    int i = 0;

    try
    {
        while (file_in >> s)
        {
            if (i < *numMesgs)
            {
                mesgBuf[i++] = rpc::deserialize<njson, TestMessage>(njson::parse(s));
            }
            else
            {
                ss << s << '\n';
            }
        }
    }
    catch (...)
    {
        *numMesgs = i;
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessages(TestMessage* mesgBuf, int* numMesgs)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    for (int i = 0; i < *numMesgs; ++i)
    {
        try
        {
            file_out << rpc::serialize<njson, TestMessage>(mesgBuf[i]).dump() << '\n';
        }
        catch (...)
        {
            *numMesgs = i;
            return 1;
        }
    }

    return 0;
}

int ReadMessageRef(TestMessage& mesg)
{
    std::ifstream file_in("bus.txt");
    std::stringstream ss;

    std::string s;

    try
    {
        if (file_in >> s)
        {
            mesg = rpc::deserialize<njson, TestMessage>(njson::parse(s));
        }
        while (file_in >> s)
        {
            ss << s << '\n';
        }
    }
    catch (...)
    {
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessageRef(const TestMessage& mesg)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    try
    {
        file_out << rpc::serialize<njson, TestMessage>(mesg).dump() << '\n';
    }
    catch (...)
    {
        return 1;
    }

    return 0;
}

int ReadMessageVec(std::vector<TestMessage>& vec, int& numMesgs)
{
    std::ifstream file_in("bus.txt");
    std::stringstream ss;

    std::string s;
    int i = 0;

    try
    {
        while (file_in >> s)
        {
            if (i < numMesgs)
            {
                vec.push_back(rpc::deserialize<njson, TestMessage>(njson::parse(s)));
            }
            else
            {
                ss << s << '\n';
            }
        }
    }
    catch (...)
    {
        numMesgs = i;
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessageVec(const std::vector<TestMessage>& vec)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    for (const auto& mesg : vec)
    {
        try
        {
            file_out << rpc::serialize<njson, TestMessage>(mesg).dump() << '\n';
        }
        catch (...)
        {
            return 1;
        }
    }

    return 0;
}

void ClearBus()
{
    std::ofstream("bus.txt");
}

uint64_t Fibonacci(uint64_t number)
{
    return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}

void FibonacciPtr(uint64_t* number)
{
    if (*number < 2)
    {
        *number = 1;
    }
    else
    {
        uint64_t n1 = *number - 1;
        uint64_t n2 = *number - 2;
        FibonacciPtr(&n1);
        FibonacciPtr(&n2);
        *number = n1 + n2;
    }
}

void FibonacciRef(uint64_t& number)
{
    if (number < 2)
    {
        number = 1;
    }
    else
    {
        uint64_t n1 = number - 1;
        uint64_t n2 = number - 2;
        FibonacciRef(n1);
        FibonacciRef(n2);
        number = n1 + n2;
    }
}

struct Complex
{
public:
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};
};

template<>
inline njson rpc::serialize(const Complex& cx) RPC_HPP_EXCEPT
{
    njson obj_j;
    obj_j["id"] = cx.id;
    obj_j["name"] = cx.name;
    obj_j["flag1"] = cx.flag1;
    obj_j["flag2"] = cx.flag2;
    obj_j["vals"] = cx.vals;

    return obj_j;
}

template<>
inline Complex rpc::deserialize(const njson& obj_j) RPC_HPP_EXCEPT
{
    Complex cx;
    cx.id = obj_j["id"].get<int>();
    cx.name = obj_j["name"].get<std::string>();
    cx.flag1 = obj_j["flag1"].get<bool>();
    cx.flag2 = obj_j["flag2"].get<bool>();
    auto vec = obj_j["vals"].get<std::vector<uint8_t>>();

    if (vec.size() > 12)
    {
        std::copy(vec.begin(), vec.begin() + 12, cx.vals.begin());
    }
    else
    {
        std::copy(vec.begin(), vec.end(), cx.vals.begin());
    }

    return cx;
}

std::string HashComplex(Complex cx)
{
    std::stringstream hash;

    if (cx.flag1)
    {
        std::reverse(cx.vals.begin(), cx.vals.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        int acc = cx.flag2 ? (cx.name[i] + cx.vals[i % 12]) : (cx.name[i] - cx.vals[i % 12]);
        hash << std::hex << acc;
    }

    return hash.str();
}

void HashComplexPtr(const Complex* cx, char* hashStr)
{
    std::stringstream hash;
    std::array<uint8_t, 12> valsCpy = cx->vals;

    if (cx->flag1)
    {
        std::reverse(valsCpy.begin(), valsCpy.end());
    }

    for (size_t i = 0; i < cx->name.size(); ++i)
    {
        int acc = cx->flag2 ? (cx->name[i] + valsCpy[i % 12]) : (cx->name[i] - valsCpy[i % 12]);
        hash << std::hex << acc;
    }

    auto str = hash.str();
    std::copy(str.begin(), str.end(), hashStr);
}

void HashComplexRef(Complex& cx, std::string& hashStr)
{
    std::stringstream hash;

    if (cx.flag1)
    {
        std::reverse(cx.vals.begin(), cx.vals.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        int acc = cx.flag2 ? (cx.name[i] + cx.vals[i % 12]) : (cx.name[i] - cx.vals[i % 12]);
        hash << std::hex << acc;
    }

    hashStr = hash.str();
}

double Average(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10)
{
    return (n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10) / 10.00;
}

double StdDev(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10)
{
    auto avg = Average(
        n1 * n1, n2 * n2, n3 * n3, n4 * n4, n5 * n5, n6 * n6, n7 * n7, n8 * n8, n9 * n9, n10 * n10);
    return sqrt(avg);
}

void SquareRootPtr(double* n1, double* n2, double* n3, double* n4, double* n5, double* n6,
    double* n7, double* n8, double* n9, double* n10)
{
    *n1 = sqrt(*n1);
    *n2 = sqrt(*n2);
    *n3 = sqrt(*n3);
    *n4 = sqrt(*n4);
    *n5 = sqrt(*n5);
    *n6 = sqrt(*n6);
    *n7 = sqrt(*n7);
    *n8 = sqrt(*n8);
    *n9 = sqrt(*n9);
    *n10 = sqrt(*n10);
}

void SquareRootRef(double& n1, double& n2, double& n3, double& n4, double& n5, double& n6,
    double& n7, double& n8, double& n9, double& n10)
{
    n1 = sqrt(n1);
    n2 = sqrt(n2);
    n3 = sqrt(n3);
    n4 = sqrt(n4);
    n5 = sqrt(n5);
    n6 = sqrt(n6);
    n7 = sqrt(n7);
    n8 = sqrt(n8);
    n9 = sqrt(n9);
    n10 = sqrt(n10);
}

template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}

std::vector<uint64_t> RandInt(uint64_t min, uint64_t max, size_t sz = 1000)
{
    std::vector<uint64_t> vec;
    vec.reserve(sz);

    for (size_t i = 0; i < sz; ++i)
    {
        vec.push_back(static_cast<uint64_t>(std::rand()) % (max - min + 1) + min);
    }

    return vec;
}

template<typename Serial>
rpc::func_result<Serial> rpc::dispatch(const func_call<Serial>& fc)
{
    const auto func_name = fc.get_func_name();

    if (func_name == "WriteMessages")
    {
        return rpc::run_callback(WriteMessages, fc);
    }

    if (func_name == "WriteMessageRef")
    {
        return rpc::run_callback(WriteMessageRef, fc);
    }

    if (func_name == "WriteMessageVec")
    {
        return rpc::run_callback(WriteMessageVec, fc);
    }

    if (func_name == "ReadMessages")
    {
        return rpc::run_callback(ReadMessages, fc);
    }

    if (func_name == "ReadMessageRef")
    {
        return rpc::run_callback(ReadMessageRef, fc);
    }

    if (func_name == "ReadMessageVec")
    {
        return rpc::run_callback(ReadMessageVec, fc);
    }

    if (func_name == "Fibonacci")
    {
        return rpc::run_callback(Fibonacci, fc);
    }

    if (func_name == "FibonacciPtr")
    {
        return rpc::run_callback(FibonacciPtr, fc);
    }

    if (func_name == "FibonacciRef")
    {
        return rpc::run_callback(FibonacciRef, fc);
    }

    if (func_name == "HashComplex")
    {
        return rpc::run_callback(HashComplex, fc);
    }

    if (func_name == "HashComplexPtr")
    {
        return rpc::run_callback(HashComplexPtr, fc);
    }

    if (func_name == "HashComplexRef")
    {
        return rpc::run_callback(HashComplexRef, fc);
    }

    if (func_name == "Average")
    {
        return rpc::run_callback(Average, fc);
    }

    if (func_name == "StdDev")
    {
        return rpc::run_callback(StdDev, fc);
    }

    if (func_name == "SquareRootPtr")
    {
        return rpc::run_callback(SquareRootPtr, fc);
    }

    if (func_name == "SquareRootRef")
    {
        return rpc::run_callback(SquareRootRef, fc);
    }

    if (func_name == "AverageContainer<double>")
    {
        return rpc::run_callback(AverageContainer<double>, fc);
    }

    if (func_name == "AverageContainer<uint64_t>")
    {
        return rpc::run_callback(AverageContainer<uint64_t>, fc);
    }

    if (func_name == "RandInt")
    {
        return rpc::run_callback(RandInt, fc);
    }

    throw std::runtime_error("RPC error: Called function: \"" + func_name + "\" not found!");
}
