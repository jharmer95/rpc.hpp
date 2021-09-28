///@file rpc.server.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of an RPC server for testing
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

#define RPC_HPP_SERVER_IMPL
#define RPC_HPP_ENABLE_SERVER_CACHE

#include "rpc.server.hpp"

#if defined(RPC_HPP_ENABLE_NJSON)
#    include <rpc_adapters/rpc_njson.hpp>

using rpc::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include <rpc_adapters/rpc_rapidjson.hpp>

using rpc::adapters::rapidjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include <rpc_adapters/rpc_boost_json.hpp>

using rpc::adapters::boost_json_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
#    include <rpc_adapters/rpc_bitsery.hpp>

using rpc::adapters::bitsery_adapter;

const uint64_t rpc::adapters::bitsery::config::max_func_name_size = 30;
const uint64_t rpc::adapters::bitsery::config::max_string_size = 2048U;
const uint64_t rpc::adapters::bitsery::config::max_container_size = 100;
#endif

#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>

static bool RUNNING = false;
static std::mutex MUTEX;
static std::condition_variable cv;

[[noreturn]] void ThrowError() noexcept(false)
{
    throw std::runtime_error("THIS IS A TEST ERROR!");
}

// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
inline void KillServer() noexcept
{
    std::unique_lock<std::mutex> lk{ MUTEX };
    RUNNING = false;
    lk.unlock();
    cv.notify_one();
}

// cached
inline size_t StrLen(const std::string& str)
{
    return str.size();
}

// cached
inline std::vector<int> AddOneToEach(std::vector<int> vec)
{
    for (auto& n : vec)
    {
        ++n;
    }

    return vec;
}

inline void AddOneToEachRef(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

int (*CountChars)(const std::string&, char) = [](const std::string& str, char c)
{
    return static_cast<int>(
        std::count_if(str.begin(), str.end(), [c](const char x) { return x == c; }));
};

void (*AddOne)(size_t&) = [](size_t& n)
{
    n += 1;
};

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
inline double StdDev(const double n1, const double n2, const double n3, const double n4,
    const double n5, const double n6, const double n7, const double n8, const double n9,
    const double n10)
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

std::map<char, unsigned> CharacterMap(const std::string& str)
{
    std::map<char, unsigned> ret_map;

    for (const auto c : str)
    {
        if (ret_map.find(c) != ret_map.end())
        {
            ret_map[c] += 1;
        }
        else
        {
            ret_map[c] = 1;
        }
    }

    return ret_map;
}

inline unsigned UMapSum(const std::unordered_map<std::string, unsigned>& umap)
{
    unsigned sum = 0;

    for (const auto& [_, value] : umap)
    {
        sum += value;
    }

    return sum;
}

// inline void AddToPQueue(std::priority_queue<int>& que, int num)
// {
//     que.push(num);
// }

// inline int QueueSum(std::queue<int>& que)
// {
//     int sum = que.front();
//     que.pop();

//     sum += que.front();
//     que.pop();

//     return sum;
// }

// inline int StackSum(std::stack<int>& stk)
// {
//     int sum = stk.top();
//     stk.pop();

//     sum += stk.top();
//     stk.pop();

//     return sum;
// }

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

template<typename Serial, typename R, typename... Args>
void dump_cache(TestServer<Serial>& server, [[maybe_unused]] R (*func)(Args...),
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
void load_cache(TestServer<Serial>& server, [[maybe_unused]] R (*func)(Args...),
    const std::string& func_name, const std::string& dump_dir)
{
    auto& cache = server.template get_func_cache<R>(func_name);

    std::string file_name = func_name;

    std::replace(file_name.begin(), file_name.end(), '<', '(');
    std::replace(file_name.begin(), file_name.end(), '>', ')');

    std::ifstream ifile(dump_dir + "/" + std::move(file_name) + ".dump");

    if (!ifile.is_open())
    {
        std::cout << "Could not load cache for function: " << func_name << '\n';
        return;
    }

    std::stringstream ss;
    std::string val_str;

    while (ifile.get(*ss.rdbuf(), '\034'))
    {
        // Toss out separator
        [[maybe_unused]] auto unused = ifile.get();

        std::getline(ifile, val_str);

        if constexpr (std::is_arithmetic_v<R>)
        {
            R value;
            std::stringstream ss2(val_str);

            ss2 >> value;

            cache[ss.str()] = value;
        }
        else if constexpr (std::is_same_v<R, std::string>)
        {
            cache[ss.str()] = std::move(val_str);
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

            cache[ss.str()] = std::move(value);
        }

        ss.clear();
    }
}

#define DUMP_CACHE(SERVER, FUNCNAME, DIR) dump_cache(SERVER, FUNCNAME, #FUNCNAME, DIR)
#define LOAD_CACHE(SERVER, FUNCNAME, DIR) load_cache(SERVER, FUNCNAME, #FUNCNAME, DIR)

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

#if defined(RPC_HPP_ENABLE_NJSON)
        TestServer<njson_adapter> njson_server{ io_context, 5000U };

        const std::string njson_dump_path("dump_cache");

        if (std::filesystem::exists(njson_dump_path)
            && std::filesystem::is_directory(njson_dump_path))
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

        std::thread(&TestServer<njson_adapter>::Run, &njson_server).detach();
        std::cout << "Running njson server on port 5000...\n";
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
        TestServer<rapidjson_adapter> rapidjson_server{ io_context, 5001U };
        std::thread(&TestServer<rapidjson_adapter>::Run, &rapidjson_server).detach();
        std::cout << "Running rapidjson server on port 5001...\n";
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
        TestServer<boost_json_adapter> bjson_server{ io_context, 5002U };
        std::thread(&TestServer<boost_json_adapter>::Run, &bjson_server).detach();
        std::cout << "Running Boost.JSON server on port 5002...\n";
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
        TestServer<bitsery_adapter> bitsery_server{ io_context, 5003U };
        std::thread(&TestServer<bitsery_adapter>::Run, &bitsery_server).detach();
        std::cout << "Running Bitsery server on port 5003...\n";
#endif

        std::unique_lock<std::mutex> lk{ MUTEX };
        cv.wait(lk, [] { return !RUNNING; });

#if defined(RPC_HPP_ENABLE_NJSON)
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

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
        return 1;
    }
}
