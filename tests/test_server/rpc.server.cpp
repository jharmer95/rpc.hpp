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

#include <algorithm>
#include <cmath>
#include <map>
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

constexpr size_t bitsery_adapter::config::max_func_name_size = 30;
constexpr size_t bitsery_adapter::config::max_string_size = 2'048;
constexpr size_t bitsery_adapter::config::max_container_size = 1'000;
#endif

namespace test_server
{
[[noreturn]] void ThrowError() noexcept(false)
{
    throw std::domain_error("THIS IS A TEST ERROR!");
}

// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
void KillServer() noexcept
{
    std::puts("\nShutting down from remote KillServer call...");
    RUNNING = false;
}

// cached
size_t StrLen(std::string_view str)
{
    return str.size();
}

// cached
std::vector<int> AddOneToEach(std::vector<int> vec)
{
    for (auto& num : vec)
    {
        ++num;
    }

    return vec;
}

void AddOneToEachRef(std::vector<int>& vec)
{
    for (auto& num : vec)
    {
        ++num;
    }
}

static int CountChars(const std::string& str, char chr)
{
    return static_cast<int>(
        std::count_if(str.begin(), str.end(), [chr](const char tmp_c) { return tmp_c == chr; }));
}

static void AddOne(size_t& n)
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
        uint64_t num1 = number - 1;
        uint64_t num2 = number - 2;
        FibonacciRef(num1);
        FibonacciRef(num2);
        number = num1 + num2;
    }
}

// cached
double StdDev(const double num1, const double num2, const double num3, const double num4,
    const double num5, const double num6, const double num7, const double num8, const double num9,
    const double num10)
{
    const auto avg = Average(num1 * num1, num2 * num2, num3 * num3, num4 * num4, num5 * num5,
        num6 * num6, num7 * num7, num8 * num8, num9 * num9, num10 * num10);

    return std::sqrt(avg);
}

void SquareRootRef(double& num1, double& num2, double& num3, double& num4, double& num5,
    double& num6, double& num7, double& num8, double& num9, double& num10)
{
    num1 = std::sqrt(num1);
    num2 = std::sqrt(num2);
    num3 = std::sqrt(num3);
    num4 = std::sqrt(num4);
    num5 = std::sqrt(num5);
    num6 = std::sqrt(num6);
    num7 = std::sqrt(num7);
    num8 = std::sqrt(num8);
    num9 = std::sqrt(num9);
    num10 = std::sqrt(num10);
}

void SquareArray(std::array<int, 12>& arr)
{
    for (int& val : arr)
    {
        val *= val;
    }
}

void RemoveFromList(
    std::forward_list<std::string>& list, const std::string& str, bool case_sensitive)
{
    list.remove_if(
        [case_sensitive, &str](const std::string& val)
        {
            if (case_sensitive)
            {
                return val == str;
            }

            const auto str_tolower = [](std::string tmp_str)
            {
                std::transform(tmp_str.begin(), tmp_str.end(), tmp_str.begin(),
                    [](unsigned char tmp_char) { return std::tolower(tmp_char); });

                return tmp_str;
            };

            return str_tolower(val) == str_tolower(str);
        });
}

std::map<char, unsigned> CharacterMap(std::string_view str)
{
    std::map<char, unsigned> ret_map;

    for (const auto chr : str)
    {
        if (ret_map.find(chr) != ret_map.end())
        {
            ret_map[chr] += 1;
        }
        else
        {
            ret_map[chr] = 1;
        }
    }

    return ret_map;
}

size_t CountResidents(const std::multimap<int, std::string>& registry, int floor_num)
{
    return registry.count(floor_num);
}

std::unordered_set<std::string> GetUniqueNames(const std::vector<std::string>& names)
{
    std::unordered_set<std::string> result;

    for (const auto& str : names)
    {
        result.insert(str);
    }

    return result;
}

std::vector<uint64_t> GenRandInts(const uint64_t min, const uint64_t max, const size_t num_ints)
{
    std::vector<uint64_t> vec;
    vec.reserve(num_ints);

    for (size_t i = 0; i < num_ints; ++i)
    {
        vec.push_back(static_cast<uint64_t>(std::rand()) % (((max - min) + 1) + min));
    }

    return vec;
}

// cached
std::string HashComplex(const ComplexObject& cx_obj)
{
    std::stringstream hash;
    auto values = cx_obj.vals;

    if (cx_obj.flag1)
    {
        std::reverse(values.begin(), values.end());
    }

    const auto name_sz = cx_obj.name.size();

    for (size_t i = 0; i < name_sz; ++i)
    {
        const int acc =
            (cx_obj.flag2) ? (cx_obj.name[i] + values[i % 12]) : (cx_obj.name[i] - values[i % 12]);
        hash << std::hex << acc;
    }

    return hash.str();
}

void HashComplexRef(ComplexObject& cx_obj, std::string& hashStr)
{
    std::stringstream hash;

    if (cx_obj.flag1)
    {
        std::reverse(cx_obj.vals.begin(), cx_obj.vals.end());
    }

    const auto name_sz = cx_obj.name.size();

    for (size_t i = 0; i < name_sz; ++i)
    {
        const int acc = (cx_obj.flag2) ? (cx_obj.name[i] + cx_obj.vals[i % 12])
                                       : (cx_obj.name[i] - cx_obj.vals[i % 12]);
        hash << std::hex << acc;
    }

    hashStr = hash.str();
}

template<typename Serial>
static void BindFuncs(TestServer<Serial>& server)
{
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    server.template bind<std::string>(
        "GetConnectionInfo", std::move([&server] { return server.GetConnectionInfo(); }));
#endif

    server.bind("KillServer", &KillServer);
    server.bind("ThrowError", &ThrowError);
    server.bind("AddOneToEachRef", &AddOneToEachRef);
    server.bind("FibonacciRef", &FibonacciRef);
    server.bind("SquareRootRef", &SquareRootRef);
    server.bind("GenRandInts", &GenRandInts);
    server.bind("HashComplexRef", &HashComplexRef);
    server.bind("SquareArray", &SquareArray);
    server.bind("RemoveFromList", &RemoveFromList);
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
    server.bind("CharacterMap", &CharacterMap);
    server.bind("CountResidents", &CountResidents);
    server.bind("GetUniqueNames", &GetUniqueNames);
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
} //namespace test_server

int main(const int argc, char* argv[])
{
    if ((argc > 1) && (std::strcmp(argv[1], "--help") == 0))
    {
        return 0;
    }

    try
    {
        asio::io_context io_ctx{};
        test_server::RUNNING = true;

        std::vector<std::thread> threads;

#if defined(RPC_HPP_ENABLE_NJSON)
        test_server::TestServer<njson_adapter> njson_server{ io_ctx, 5000U };
        test_server::BindFuncs(njson_server);

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

        threads.emplace_back(&test_server::TestServer<njson_adapter>::Run, &njson_server);
        std::puts("Running njson server on port 5000...");
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
        TestServer<rapidjson_adapter> rapidjson_server{ io_ctx, 5001U };
        BindFuncs(rapidjson_server);
        threads.emplace_back(&TestServer<rapidjson_adapter>::Run, &rapidjson_server);
        puts("Running rapidjson server on port 5001...");
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
        TestServer<boost_json_adapter> bjson_server{ io_ctx, 5002U };
        BindFuncs(bjson_server);
        threads.emplace_back(&TestServer<boost_json_adapter>::Run, &bjson_server);
        puts("Running Boost.JSON server on port 5002...");
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
        TestServer<bitsery_adapter> bitsery_server{ io_ctx, 5003U };
        BindFuncs(bitsery_server);
        threads.emplace_back(&TestServer<bitsery_adapter>::Run, &bitsery_server);
        puts("Running Bitsery server on port 5003...");
#endif

        for (auto& thrd : threads)
        {
            thrd.join();
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

        std::puts("Exited normally");
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::fprintf(stderr, "Exception: %s\n", ex.what());
        return 1;
    }
}
