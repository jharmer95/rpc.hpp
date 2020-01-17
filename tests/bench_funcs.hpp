#pragma once

#include "rpc.hpp"
#include "adapters/rpc_njson.hpp"

#include <algorithm>
#include <array>
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

    // [[nodiscard]] static nlohmann::json Serialize(const TestMessage& mesg)
    // {
    //     nlohmann::json obj_j;
    //     obj_j["ID"] = mesg.ID;
    //     obj_j["Flag1"] = mesg.Flag1;
    //     obj_j["Flag2"] = mesg.Flag2;
    //     obj_j["DataSize"] = mesg.DataSize;

    //     if (mesg.DataSize > 0)
    //     {
    //         std::vector<int> tmpVec(mesg.Data, mesg.Data + mesg.DataSize);
    //         obj_j["Data"] = tmpVec;
    //     }
    //     else
    //     {
    //         obj_j["Data"] = nlohmann::json::array();
    //     }

    //     return obj_j;
    // }

    // [[nodiscard]] static TestMessage DeSerialize(const nlohmann::json& obj_j)
    // {
    //     TestMessage mesg;
    //     mesg.ID = obj_j["ID"].get<int>();
    //     mesg.Flag1 = obj_j["Flag1"].get<bool>();
    //     mesg.Flag2 = obj_j["Flag2"].get<bool>();
    //     mesg.DataSize = obj_j["DataSize"].get<uint8_t>();
    //     const auto sData = obj_j["Data"].get<std::vector<int>>();
    //     std::copy(sData.begin(), sData.begin() + mesg.DataSize, mesg.Data);
    //     return mesg;
    // }
};

template<>
[[nodiscard]] nlohmann::json rpc::Serialize(const TestMessage& mesg)
{
    nlohmann::json obj_j;
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
        obj_j["Data"] = nlohmann::json::array();
    }

    return obj_j;
}

template<>
[[nodiscard]] TestMessage rpc::DeSerialize(const nlohmann::json& obj_j)
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
                mesgBuf[i++] = rpc::DeSerialize<TestMessage, njson::json>(njson::json::parse(s));
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
            file_out << rpc::Serialize<TestMessage, njson::json>(mesgBuf[i]).dump() << '\n';
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
    int i = 0;

    try
    {
        if (file_in >> s)
        {
            mesg = rpc::DeSerialize<TestMessage, njson::json>(njson::json::parse(s));
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
        file_out << rpc::Serialize<TestMessage, njson::json>(mesg).dump() << '\n';
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
                vec.push_back(rpc::DeSerialize<TestMessage, njson::json>(njson::json::parse(s)));
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
            file_out << rpc::Serialize<TestMessage, njson::json>(mesg).dump() << '\n';
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
    int id;
    std::string name;
    bool flag1;
    bool flag2;
    std::array<uint8_t, 12> vals;
};

template<>
[[nodiscard]] njson::json rpc::Serialize(const Complex& cx)
{
    njson::json obj_j;
    obj_j["id"] = cx.id;
    obj_j["name"] = cx.name;
    obj_j["flag1"] = cx.flag1;
    obj_j["flag2"] = cx.flag2;
    obj_j["vals"] = cx.vals;

    return obj_j;
}

template<>
[[nodiscard]] Complex rpc::DeSerialize(const njson::json& obj_j)
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

long double Average(long double n1, long double n2, long double n3, long double n4, long double n5, long double n6, long double n7, long double n8, long double n9, long double n10)
{
    return (n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10) / 10.00L;
}

long double StdDev(long double n1, long double n2, long double n3, long double n4, long double n5, long double n6, long double n7, long double n8, long double n9, long double n10)
{
    auto avg = Average(n1 * n1, n2 * n2, n3 * n3, n4 * n4, n5 * n5, n6 * n6, n7 * n7, n8 * n8, n9 * n9, n10 * n10);
    return sqrtl(avg);
}

void SquareRootPtr(long double* n1, long double* n2, long double* n3, long double* n4, long double* n5, long double* n6, long double* n7, long double* n8, long double* n9, long double* n10)
{
    *n1 = sqrtl(*n1);
    *n2 = sqrtl(*n2);
    *n3 = sqrtl(*n3);
    *n4 = sqrtl(*n4);
    *n5 = sqrtl(*n5);
    *n6 = sqrtl(*n6);
    *n7 = sqrtl(*n7);
    *n8 = sqrtl(*n8);
    *n9 = sqrtl(*n9);
    *n10 = sqrtl(*n10);
}

void SquareRootRef(long double& n1, long double& n2, long double& n3, long double& n4, long double& n5, long double& n6, long double& n7, long double& n8, long double& n9, long double& n10)
{
    n1 = sqrtl(n1);
    n2 = sqrtl(n2);
    n3 = sqrtl(n3);
    n4 = sqrtl(n4);
    n5 = sqrtl(n5);
    n6 = sqrtl(n6);
    n7 = sqrtl(n7);
    n8 = sqrtl(n8);
    n9 = sqrtl(n9);
    n10 = sqrtl(n10);
}

template <typename T>
long double AverageContainer(const std::vector<T>& vec)
{
    long double sum = std::accumulate(vec.begin(), vec.end(), 0.00L);
    return sum / vec.size();
}

std::vector<int> RandInt(int min, int max, size_t sz = 1000UL)
{
    std::vector<int> vec;
    vec.reserve(sz);

    for (size_t i = 0; i < sz; ++i)
    {
        vec.push_back(std::rand() % (max - min + 1) + min);
    }

    return vec;
}

template<typename T_Serial>
std::string rpc::dispatch(const std::string& funcName, const T_Serial& obj)
{
    if (funcName == "WriteMessages")
    {
        return rpc::RunCallBack(obj, WriteMessages);
    }

    if (funcName == "WriteMessageRef")
    {
        return rpc::RunCallBack(obj, WriteMessageRef);
    }

    if (funcName == "WriteMessageVec")
    {
        return rpc::RunCallBack(obj, WriteMessageVec);
    }

    if (funcName == "ReadMessages")
    {
        return rpc::RunCallBack(obj, ReadMessages);
    }

    if (funcName == "ReadMessageRef")
    {
        return rpc::RunCallBack(obj, ReadMessageRef);
    }

    if (funcName == "ReadMessageVec")
    {
        return rpc::RunCallBack(obj, ReadMessageVec);
    }

    if (funcName == "Fibonacci")
    {
        return rpc::RunCallBack(obj, Fibonacci);
    }

    if (funcName == "FibonacciPtr")
    {
        return rpc::RunCallBack(obj, FibonacciPtr);
    }

    if (funcName == "FibonacciRef")
    {
        return rpc::RunCallBack(obj, FibonacciRef);
    }

    if (funcName == "HashComplex")
    {
        return rpc::RunCallBack(obj, HashComplex);
    }

    if (funcName == "HashComplexPtr")
    {
        return rpc::RunCallBack(obj, HashComplexPtr);
    }

    if (funcName == "HashComplexRef")
    {
        return rpc::RunCallBack(obj, HashComplexRef);
    }

    if (funcName == "Average")
    {
        return rpc::RunCallBack(obj, Average);
    }

    if (funcName == "StdDev")
    {
        return rpc::RunCallBack(obj, StdDev);
    }

    if (funcName == "SquareRootPtr")
    {
        return rpc::RunCallBack(obj, SquareRootPtr);
    }

    if (funcName == "SquareRootRef")
    {
        return rpc::RunCallBack(obj, SquareRootRef);
    }

    if (funcName == "AverageContainer<long double>")
    {
        return rpc::RunCallBack(obj, AverageContainer<long double>);
    }

    if (funcName == "AverageContainer<uint64_t>")
    {
        return rpc::RunCallBack(obj, AverageContainer<uint64_t>);
    }

    if (funcName == "RandInt")
    {
        return rpc::RunCallBack(obj, RandInt);
    }

    throw std::runtime_error("RPC error: Called function: \"" + funcName + "\" not found!");
}
