///@file rpc.server.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Example implementation of an RPC server
///@version 0.2.4
///@date 03-01-2021
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

#include <asio.hpp>

#include <atomic>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#if defined(RPC_HPP_NJSON_ENABLED)
#    include "rpc_adapters/rpc_njson.hpp"
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
#    include "rpc_adapters/rpc_rapidjson.hpp"
#endif

#if defined(RPC_HPP_BOOST_JSON_ENABLED)
#    include "rpc_adapters/rpc_boost_json.hpp"
#endif

#include "rpc_dispatch_helper.hpp"
#include "test_structs.hpp"

using asio::ip::tcp;

static std::atomic_bool RUNNING = false;

#if defined(RPC_HPP_ENABLE_POINTERS)
constexpr void PtrSum(int* const n1, const int n2)
{
    *n1 += n2;
}

constexpr int AddAllPtr(const int* const vals, const int num_vals)
{
    int sum = 0;

    for (int i = 0; i < num_vals; ++i)
    {
        sum += vals[i];
    }

    return sum;
}

int ReadMessagePtr(TestMessage* const mesg_arr, int* const num_mesgs)
{
    std::ifstream file_in("bus.txt");

    if (!file_in.is_open())
    {
        return 2;
    }

    std::stringstream ss;
    std::string s;
    int i = 0;

    try
    {
        while (file_in >> s)
        {
            if (i < *num_mesgs)
            {
                mesg_arr[i++] = rpc::deserialize<njson_serial_t, TestMessage>(njson::parse(s));
            }
            else
            {
                ss << s << '\n';
            }
        }
    }
    catch (...)
    {
        *num_mesgs = i;
        return 1;
    }

    file_in.close();
    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessagePtr(const TestMessage* const mesg_arr, int* const num_mesgs)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    for (int i = 0; i < *num_mesgs; ++i)
    {
        try
        {
            file_out << rpc::serialize<njson_serial_t, TestMessage>(mesg_arr[i]).dump() << '\n';
        }
        catch (...)
        {
            *num_mesgs = i;
            return 1;
        }
    }

    return 0;
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

void SquareRootPtr(double* const n1, double* const n2, double* const n3, double* const n4,
    double* const n5, double* const n6, double* const n7, double* const n8, double* const n9,
    double* const n10)
{
    *n1 = std::sqrt(*n1);
    *n2 = std::sqrt(*n2);
    *n3 = std::sqrt(*n3);
    *n4 = std::sqrt(*n4);
    *n5 = std::sqrt(*n5);
    *n6 = std::sqrt(*n6);
    *n7 = std::sqrt(*n7);
    *n8 = std::sqrt(*n8);
    *n9 = std::sqrt(*n9);
    *n10 = std::sqrt(*n10);
}

void HashComplexPtr(const ComplexObject* const cx, char* const hashStr)
{
    std::stringstream hash;
    std::array<uint8_t, 12> valsCpy = cx->vals;

    if (cx->flag1)
    {
        std::reverse(valsCpy.begin(), valsCpy.end());
    }

    for (size_t i = 0; i < cx->name.size(); ++i)
    {
        const int acc =
            cx->flag2 ? (cx->name[i] + valsCpy[i % 12]) : (cx->name[i] - valsCpy[i % 12]);
        hash << std::hex << acc;
    }

    auto str = hash.str();
    std::copy(str.begin(), str.end(), hashStr);
    hashStr[str.size()] = '\0';
}
#endif

void KillServer()
{
    RUNNING = false;
}

constexpr int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

size_t StrLen(const std::string& str)
{
    return str.size();
}

std::vector<int> AddOneToEach(std::vector<int> vec)
{
    for (auto& n : vec)
    {
        ++n;
    }

    return vec;
}

void AddOneToEachRef(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

int ReadMessageRef(TestMessage& mesg)
{
    std::ifstream file_in("bus.txt");

    if (!file_in.is_open())
    {
        return 2;
    }

    std::stringstream ss;
    std::string s;

    try
    {
        if (file_in >> s)
        {
            mesg = rpc::deserialize<njson_serial_t, TestMessage>(njson::parse(s));
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

    if (!file_out.is_open())
    {
        return 3;
    }

    file_out << ss.str();
    return 0;
}

int WriteMessageRef(const TestMessage& mesg)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    if (!file_out.is_open())
    {
        return 3;
    }

    try
    {
        file_out << rpc::serialize<njson_serial_t, TestMessage>(mesg).dump() << '\n';
    }
    catch (...)
    {
        return 1;
    }

    return 0;
}

int ReadMessageVec(std::vector<TestMessage>& vec, int& num_mesgs)
{
    std::ifstream file_in("bus.txt");

    if (!file_in.is_open())
    {
        return 2;
    }

    std::stringstream ss;

    std::string s;
    int i = 0;

    try
    {
        while (file_in >> s)
        {
            if (i < num_mesgs)
            {
                vec.push_back(rpc::deserialize<njson_serial_t, TestMessage>(njson::parse(s)));
            }
            else
            {
                ss << s << '\n';
            }
        }
    }
    catch (...)
    {
        num_mesgs = i;
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    if (!file_out.is_open())
    {
        return 3;
    }

    file_out << ss.str();
    return 0;
}

int WriteMessageVec(const std::vector<TestMessage>& vec)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    if (!file_out.is_open())
    {
        return 3;
    }

    for (const auto& mesg : vec)
    {
        try
        {
            file_out << rpc::serialize<njson_serial_t, TestMessage>(mesg).dump() << '\n';
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

uint64_t Fibonacci(const uint64_t number)
{
    return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
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

double Average(const double n1, const double n2, const double n3, const double n4, const double n5,
    const double n6, const double n7, const double n8, const double n9, const double n10)
{
    return (n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10) / 10.00;
}

double StdDev(const double n1, const double n2, const double n3, const double n4, const double n5,
    const double n6, const double n7, const double n8, const double n9, const double n10)
{
    const auto avg = Average(
        n1 * n1, n2 * n2, n3 * n3, n4 * n4, n5 * n5, n6 * n6, n7 * n7, n8 * n8, n9 * n9, n10 * n10);

    return std::sqrt(avg);
}

void SquareRootRef(double& n1, double& n2, double& n3, double& n4, double& n5, double& n6,
    double& n7, double& n8, double& n9, double& n10)
{
    n1 = std::sqrt(n1);
    n2 = std::sqrt(n2);
    n3 = std::sqrt(n3);
    n4 = std::sqrt(n4);
    n5 = std::sqrt(n5);
    n6 = std::sqrt(n6);
    n7 = std::sqrt(n7);
    n8 = std::sqrt(n8);
    n9 = std::sqrt(n9);
    n10 = std::sqrt(n10);
}

template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}

std::vector<uint64_t> RandInt(const uint64_t min, const uint64_t max, const size_t sz = 1000)
{
    std::vector<uint64_t> vec;
    vec.reserve(sz);

    for (size_t i = 0; i < sz; ++i)
    {
        vec.push_back(static_cast<uint64_t>(std::rand()) % (max - min + 1) + min);
    }

    return vec;
}

std::string HashComplex(ComplexObject cx)
{
    std::stringstream hash;

    if (cx.flag1)
    {
        std::reverse(cx.vals.begin(), cx.vals.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        const int acc = cx.flag2 ? cx.name[i] + cx.vals[i % 12] : cx.name[i] - cx.vals[i % 12];
        hash << std::hex << acc;
    }

    return hash.str();
}

void HashComplexRef(ComplexObject& cx, std::string& hashStr)
{
    std::stringstream hash;

    if (cx.flag1)
    {
        std::reverse(cx.vals.begin(), cx.vals.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        const int acc = cx.flag2 ? cx.name[i] + cx.vals[i % 12] : cx.name[i] - cx.vals[i % 12];
        hash << std::hex << acc;
    }

    hashStr = hash.str();
}

#if defined(RPC_HPP_ENABLE_POINTERS)
RPC_DEFAULT_DISPATCH(PtrSum, AddAllPtr, ReadMessagePtr, WriteMessagePtr, FibonacciPtr,
    SquareRootPtr, HashComplexPtr, KillServer, SimpleSum, StrLen, AddOneToEach, AddOneToEachRef,
    ReadMessageRef, WriteMessageRef, ReadMessageVec, WriteMessageVec, ClearBus, Fibonacci,
    FibonacciRef, Average, StdDev, SquareRootRef, AverageContainer<uint64_t>,
    AverageContainer<double>, RandInt, HashComplex, HashComplexRef)
#else
RPC_DEFAULT_DISPATCH(KillServer, SimpleSum, StrLen, AddOneToEach, AddOneToEachRef, ReadMessageRef,
    WriteMessageRef, ReadMessageVec, WriteMessageVec, ClearBus, Fibonacci, FibonacciRef, Average,
    StdDev, SquareRootRef, AverageContainer<uint64_t>, AverageContainer<double>, RandInt,
    HashComplex, HashComplexRef)
#endif

template<typename Serial>
void session(tcp::socket sock)
{
    constexpr auto BUFFER_SZ = 64U * 1024UL;
    std::unique_ptr<char[]> data = std::make_unique<char[]>(BUFFER_SZ);

    try
    {
        while (true)
        {
            asio::error_code error;
            const size_t len = sock.read_some(asio::buffer(data.get(), BUFFER_SZ), error);

            if (error == asio::error::eof)
            {
                break;
            }

            // other error
            if (error)
            {
                throw asio::system_error(error);
            }

            const std::string str(data.get(), data.get() + len);

#if defined(_DEBUG) || !defined(NDEBUG)
            std::cout << "Received: " << str << '\n';
#endif

            auto serial_obj = rpc::serial_adapter<Serial>::from_string(str);
            rpc::server::dispatch<Serial>(serial_obj);
            const auto ret_str = rpc::serial_adapter<Serial>::to_string(serial_obj);

#if defined(_DEBUG) || !defined(NDEBUG)
            std::cout << "Sending: " << ret_str << '\n';
#endif

            write(sock, asio::buffer(ret_str, ret_str.size()));
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception in thread: " << ex.what() << '\n';
    }
}

template<typename Serial>
struct port
{
};

#if defined(RPC_HPP_NJSON_ENABLED)
template<>
struct port<njson_serial_t>
{
    constexpr static uint16_t value = 5000U;
};
#endif

#if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
template<>
struct port<generic_serial_t>
{
    constexpr static uint16_t value = 5001U;
};
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
template<>
struct port<rpdjson_serial_t>
{
    constexpr static uint16_t value = 5002U;
};
#endif

#if defined(RPC_HPP_BOOST_JSON_ENABLED)
template<>
struct port<bjson_serial_t>
{
    constexpr static uint16_t value = 5003U;
};
#endif

template<typename Serial>
inline constexpr uint16_t port_v = port<Serial>::value;

template<typename Serial>
[[noreturn]] void server(asio::io_context& io_context)
{
    while (true)
    {
        tcp::acceptor acc(io_context, tcp::endpoint(tcp::v4(), port_v<Serial>));
        session<Serial>(acc.accept());
    }
}

int main()
{
    try
    {
        asio::io_context io_context;
        RUNNING = true;

#if defined(RPC_HPP_NJSON_ENABLED)
        std::thread(server<njson_serial_t>, std::ref(io_context)).detach();
        std::cout << "Running njson server on port " << port_v<njson_serial_t> << "...\n";
#    if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
        std::thread(server<generic_serial_t>, std::ref(io_context)).detach();
        std::cout << "Running nlohmann/serial_type server on port "
                  << port_v<generic_serial_t> << "...\n";
#    endif
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
        std::thread(server<rpdjson_serial_t>, std::ref(io_context)).detach();
        std::cout << "Running rapidjson server on port " << port_v<rpdjson_serial_t> << "...\n";
#endif

#if defined(RPC_HPP_BOOST_JSON_ENABLED)
        std::thread(server<bjson_serial_t>, std::ref(io_context)).detach();
        std::cout << "Running Boost.JSON server on port " << port_v<bjson_serial_t> << "...\n";
#endif

        while (RUNNING)
        {
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
        return 1;
    }
}
