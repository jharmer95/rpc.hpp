///@file rpc.server.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of an RPC server for testing
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
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

//#define RPC_HPP_ENABLE_SERVER_CACHE
#define RPC_HPP_ENABLE_CALLBACKS

#include "rpc.server.hpp"
#include "../static_funcs.hpp"

#if defined(RPC_HPP_ENABLE_NJSON)
#  include <rpc_adapters/rpc_njson.hpp>

using rpc_hpp::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  include <rpc_adapters/rpc_rapidjson.hpp>

using rpc_hpp::adapters::rapidjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  include <rpc_adapters/rpc_boost_json.hpp>

using rpc_hpp::adapters::boost_json_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <rpc_adapters/rpc_bitsery.hpp>

using rpc_hpp::adapters::bitsery_adapter;

constexpr uint64_t bitsery_adapter::config::max_func_name_size = 30;
constexpr uint64_t bitsery_adapter::config::max_string_size = 2'048;
constexpr uint64_t bitsery_adapter::config::max_container_size = 1'000;
#endif

#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>

#if defined(RPC_HPP_ENABLE_SERVER_CACHE)
#  if defined(__cpp_lib_filesystem)
#    include <filesystem>

namespace filesystem = std::filesystem;
#  else
#    include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;
#  endif

#  include <fstream>
#  include <utility>
#endif

std::atomic_bool RUNNING{ false };

[[noreturn]] void ThrowError() noexcept(false)
{
    throw std::runtime_error("THIS IS A TEST ERROR!");
}

// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
void KillServer() noexcept
{
    puts("\nShutting down from remote KillServer call...");
    RUNNING = false;
}

// cached
size_t StrLen(const std::string& str)
{
    return str.size();
}

// cached
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

int CountChars(const std::string& str, char c)
{
    return static_cast<int>(
        std::count_if(str.begin(), str.end(), [c](const char x) { return x == c; }));
}

void AddOne(size_t& n)
{
    n += 1;
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

// cached
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

std::vector<uint64_t> GenRandInts(const uint64_t min, const uint64_t max, const size_t sz)
{
    std::vector<uint64_t> vec;
    vec.reserve(sz);

    for (size_t i = 0; i < sz; ++i)
    {
        vec.push_back(static_cast<uint64_t>(std::rand()) % (max - min + 1) + min);
    }

    return vec;
}

// cached
std::string HashComplex(const ComplexObject& cx)
{
    std::stringstream hash;
    auto values = cx.vals;

    if (cx.flag1)
    {
        std::reverse(values.begin(), values.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        const int acc = cx.flag2 ? cx.name[i] + values[i % 12] : cx.name[i] - values[i % 12];
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

template<typename Serial>
void BindFuncs(TestServer<Serial>& server)
{
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    static std::function<std::string()> get_connection_info = [&server]
    {
        return server.GetConnectionInfo();
    };

    server.bind("GetConnectionInfo", get_connection_info);
#endif

    server.bind("KillServer", &KillServer);
    server.bind("ThrowError", &ThrowError);
    server.bind("AddOneToEachRef", &AddOneToEachRef);
    server.bind("FibonacciRef", &FibonacciRef);
    server.bind("SquareRootRef", &SquareRootRef);
    server.bind("GenRandInts", &GenRandInts);
    server.bind("HashComplexRef", &HashComplexRef);
    server.template bind<void, size_t&>("AddOne", [](size_t& n) { AddOne(n); });

    // Cached
    server.bind("SimpleSum", &SimpleSum);
    server.bind("StrLen", &StrLen);
    server.bind("AddOneToEach", &AddOneToEach);
    server.bind("Fibonacci", &Fibonacci);
    server.bind("Average", &Average);
    server.bind("StdDev", &StdDev);
    server.bind("AverageContainer<uint64_t>", &AverageContainer<uint64_t>);
    server.bind("AverageContainer<double>", &AverageContainer<double>);
    server.bind("HashComplex", &HashComplex);
    server.bind("CountChars", &CountChars);
}

#if defined(RPC_HPP_ENABLE_SERVER_CACHE)
template<typename Serial, typename R, typename... Args>
void dump_cache(TestServer<Serial>& server, RPC_HPP_UNUSED R (*func)(Args...),
    const std::string& func_name, const std::string& dump_dir)
{
    auto& cache = server.template get_func_cache<R>(func_name);
    std::string file_name = func_name;

    std::replace(file_name.begin(), file_name.end(), '<', '(');
    std::replace(file_name.begin(), file_name.end(), '>', ')');

    std::ofstream ofile(dump_dir + "/" + std::move(file_name) + ".dump");

    if constexpr (std::is_arithmetic_v<R> || std::is_same_v<R, std::string>)
    {
        for (const auto& [key, value] : cache)
        {
            ofile << key << '\034' << value << '\n';
        }
    }
    else if constexpr (std::is_same_v<R, std::vector<int>>)
    {
        std::stringstream ss;

        for (const auto& [key, value] : cache)
        {
            if (value.empty())
            {
                continue;
            }

            ss << '[';
            auto it = value.begin();

            for (; it != value.end() - 1; ++it)
            {
                ss << *it << ',';
            }

            ss << *it << ']';
            ofile << key << '\034' << ss.str() << '\n';
        }
    }
}

template<typename Serial, typename R, typename... Args>
void load_cache(TestServer<Serial>& server, RPC_HPP_UNUSED R (*func)(Args...),
    const std::string& func_name, const std::string& dump_dir)
{
    auto& cache = server.template get_func_cache<R>(func_name);
    std::string file_name = func_name;

    std::replace(file_name.begin(), file_name.end(), '<', '(');
    std::replace(file_name.begin(), file_name.end(), '>', ')');

    std::ifstream ifile(dump_dir + "/" + std::move(file_name) + ".dump");

    if (!ifile.is_open())
    {
        printf("Could not load cache for function: %s\n", func_name.c_str());
        return;
    }

    std::stringstream ss;
    std::string val_str;

    while (ifile.get(*ss.rdbuf(), '\034'))
    {
        // Toss out separator
        std::ignore = ifile.get();

        std::getline(ifile, val_str);

        if constexpr (std::is_arithmetic_v<R>)
        {
            R value;
            std::stringstream ss2(val_str);

            ss2 >> value;
            cache.emplace(ss.str(), value);
        }
        else if constexpr (std::is_same_v<R, std::string>)
        {
            cache.emplace(ss.str(), std::move(val_str));
        }
        else if constexpr (std::is_same_v<R, std::vector<int>>)
        {
            std::vector<int> value;
            std::stringstream ss2(val_str);

            // ignore first '['
            ss2.ignore();

            for (int i; ss2 >> i;)
            {
                value.push_back(i);

                if (ss2.peek() == ',' || ss2.peek() == ']')
                {
                    ss2.ignore();
                }
            }

            cache.emplace(ss.str(), std::move(value));
        }

        // clear stream and its buffer
        std::stringstream{}.swap(ss);
    }
}

#  define DUMP_CACHE(SERVER, FUNCNAME, DIR) dump_cache(SERVER, FUNCNAME, #  FUNCNAME, DIR)
#  define LOAD_CACHE(SERVER, FUNCNAME, DIR) load_cache(SERVER, FUNCNAME, #  FUNCNAME, DIR)
#endif

int main(const int argc, char* argv[])
{
    if (argc > 1 && strcmp(argv[1], "--help") == 0)
    {
        return 0;
    }

    try
    {
        asio::io_context io_context{};
        RUNNING = true;

        std::vector<std::thread> threads;

#if defined(RPC_HPP_ENABLE_NJSON)
        TestServer<njson_adapter> njson_server{ io_context, 5000U };
        BindFuncs(njson_server);

#  if defined(RPC_HPP_ENABLE_SERVER_CACHE)
        const std::string njson_dump_path("dump_cache");

        if (filesystem::exists(njson_dump_path) && filesystem::is_directory(njson_dump_path))
        {
            LOAD_CACHE(njson_server, SimpleSum, njson_dump_path);
            LOAD_CACHE(njson_server, StrLen, njson_dump_path);
            LOAD_CACHE(njson_server, AddOneToEach, njson_dump_path);
            LOAD_CACHE(njson_server, Fibonacci, njson_dump_path);
            LOAD_CACHE(njson_server, Average, njson_dump_path);
            LOAD_CACHE(njson_server, StdDev, njson_dump_path);
            LOAD_CACHE(njson_server, AverageContainer<uint64_t>, njson_dump_path);
            LOAD_CACHE(njson_server, AverageContainer<double>, njson_dump_path);
            LOAD_CACHE(njson_server, HashComplex, njson_dump_path);
            LOAD_CACHE(njson_server, CountChars, njson_dump_path);
        }
#  endif

        threads.emplace_back(&TestServer<njson_adapter>::Run, &njson_server);
        puts("Running njson server on port 5000...");
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
        TestServer<rapidjson_adapter> rapidjson_server{ io_context, 5001U };
        BindFuncs(rapidjson_server);
        threads.emplace_back(&TestServer<rapidjson_adapter>::Run, &rapidjson_server);
        puts("Running rapidjson server on port 5001...");
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
        TestServer<boost_json_adapter> bjson_server{ io_context, 5002U };
        BindFuncs(bjson_server);
        threads.emplace_back(&TestServer<boost_json_adapter>::Run, &bjson_server);
        puts("Running Boost.JSON server on port 5002...");
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
        TestServer<bitsery_adapter> bitsery_server{ io_context, 5003U };
        BindFuncs(bitsery_server);
        threads.emplace_back(&TestServer<bitsery_adapter>::Run, &bitsery_server);
        puts("Running Bitsery server on port 5003...");
#endif

        for (auto& th : threads)
        {
            th.join();
        }

#if defined(RPC_HPP_ENABLE_NJSON) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        DUMP_CACHE(njson_server, SimpleSum, njson_dump_path);
        DUMP_CACHE(njson_server, StrLen, njson_dump_path);
        DUMP_CACHE(njson_server, AddOneToEach, njson_dump_path);
        DUMP_CACHE(njson_server, Fibonacci, njson_dump_path);
        DUMP_CACHE(njson_server, Average, njson_dump_path);
        DUMP_CACHE(njson_server, StdDev, njson_dump_path);
        DUMP_CACHE(njson_server, AverageContainer<uint64_t>, njson_dump_path);
        DUMP_CACHE(njson_server, AverageContainer<double>, njson_dump_path);
        DUMP_CACHE(njson_server, HashComplex, njson_dump_path);
        DUMP_CACHE(njson_server, CountChars, njson_dump_path);
#endif

        puts("Exited normally");
        return 0;
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Exception: %s\n", ex.what());
        return 1;
    }
}
