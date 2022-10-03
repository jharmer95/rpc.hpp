///@file test_server.cpp
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

#define RPC_HPP_ENABLE_CALLBACKS

#include "test_server.hpp"
#include "static_funcs.hpp"

#if defined(RPC_HPP_ENABLE_BITSERY)
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_func_name_size = 30;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_string_size = 2'048;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_container_size = 1'000;
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <forward_list>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <thread>
#include <unordered_set>
#include <vector>

int CountChars(const std::string& str, char chr)
{
    return static_cast<int>(
        std::count_if(str.begin(), str.end(), [chr](const char tmp_c) { return tmp_c == chr; }));
}

void AddOne(size_t& n) noexcept
{
    n += 1;
}

namespace rpc_hpp::tests
{
[[noreturn]] void ThrowError() noexcept(false)
{
    throw std::domain_error{ "THIS IS A TEST ERROR!" };
}

// cached
size_t StrLen(std::string_view str) noexcept
{
    return str.size();
}

// cached
constexpr int SimpleSum(const int num1, const int num2)
{
    return num1 + num2;
}

// cached
constexpr double Average(const double num1, const double num2, const double num3, const double num4,
    const double num5, const double num6, const double num7, const double num8, const double num9,
    const double num10)
{
    return (num1 + num2 + num3 + num4 + num5 + num6 + num7 + num8 + num9 + num10) / 10.00;
}

// cached
template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}

// cached
std::vector<int> AddOneToEach(std::vector<int> vec) noexcept
{
    for (auto& num : vec)
    {
        ++num;
    }

    return vec;
}

void AddOneToEachRef(std::vector<int>& vec) noexcept
{
    for (auto& num : vec)
    {
        ++num;
    }
}

// cached
constexpr uint64_t Fibonacci(const uint64_t number)
{
    uint64_t num1{ 0 };
    uint64_t num2{ 1 };

    if (number == 0)
    {
        return 0;
    }

    for (uint64_t i = 2; i <= number; ++i)
    {
        const uint64_t next{ num1 + num2 };
        num1 = num2;
        num2 = next;
    }

    return num2;
}

void FibonacciRef(uint64_t& number) noexcept
{
    uint64_t num1{ 0 };
    uint64_t num2{ 1 };

    if (number == 0)
    {
        return;
    }

    for (uint64_t i = 2; i <= number; ++i)
    {
        const uint64_t next{ num1 + num2 };
        num1 = num2;
        num2 = next;
    }

    number = num2;
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
    double& num6, double& num7, double& num8, double& num9, double& num10) noexcept
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

void SquareArray(std::array<int, 12>& arr) noexcept
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
        [case_sensitive, &str](const std::string& val) noexcept
        {
            if (case_sensitive)
            {
                return val == str;
            }

            const auto str_tolower = [](std::string tmp_str)
            {
                std::transform(tmp_str.begin(), tmp_str.end(), tmp_str.begin(),
                    [](unsigned char tmp_char)
                    { return static_cast<char>(std::tolower(tmp_char)); });

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

std::optional<int> SafeDivide(int numerator, int denominator) noexcept
{
    if (denominator == 0)
    {
        return std::nullopt;
    }

    return numerator / denominator;
}

std::pair<int, int> TopTwo(const std::vector<int>& num_list) noexcept
{
    // TODO: Replace this naive version with algorithm (transform/reduce?)
    int max1{ std::numeric_limits<int>::min() };
    int max2{ std::numeric_limits<int>::min() };

    for (const auto num : num_list)
    {
        if (num > max2)
        {
            max2 = num;

            if (max2 > max1)
            {
                std::swap(max1, max2);
            }
        }
    }

    return { max1, max2 };
}

std::vector<uint64_t> GenRandInts(const ValueRange<uint64_t> num_range, const size_t num_ints)
{
    static std::mt19937_64 mt_gen{ static_cast<uint_fast64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count()) };

    std::uniform_int_distribution<uint64_t> distribution{ num_range.min, num_range.max };
    std::vector<uint64_t> vec(num_ints);
    std::generate(begin(vec), end(vec), [&distribution] { return distribution(mt_gen); });
    return vec;
}

// cached
std::string HashComplex(const ComplexObject& cx_obj)
{
    std::stringstream hash;
    auto rev_vals = cx_obj.vals;

    if (cx_obj.flag1)
    {
        std::reverse(rev_vals.begin(), rev_vals.end());
    }

    const auto name_sz = cx_obj.name.size();

    for (size_t i = 0; i < name_sz; ++i)
    {
        const size_t wrap_idx = i % rev_vals.size();
        const int acc = (cx_obj.flag2) ? (cx_obj.name[i] + rev_vals[wrap_idx])
                                       : (cx_obj.name[i] - rev_vals[wrap_idx]);

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
        const size_t wrap_idx = i % cx_obj.vals.size();
        const int acc = (cx_obj.flag2) ? (cx_obj.name[i] + cx_obj.vals[wrap_idx])
                                       : (cx_obj.name[i] - cx_obj.vals[wrap_idx]);
        hash << std::hex << acc;
    }

    hashStr = hash.str();
}

template<typename Serial>
static void BindFuncs(TestServer<Serial>& server)
{
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    server.template bind<std::string>(
        "GetConnectionInfo", [&server] { return server.GetConnectionInfo(); });
#endif

    server.template bind<void>("KillServer", [&server] { return server.Stop(); });
    server.bind("ThrowError", &ThrowError);
    server.bind("AddOneToEachRef", &AddOneToEachRef);
    server.bind("FibonacciRef", &FibonacciRef);
    server.bind("SquareRootRef", &SquareRootRef);
    server.bind("GenRandInts", &GenRandInts);
    server.bind("HashComplexRef", &HashComplexRef);
    server.bind("SquareArray", &SquareArray);
    server.bind("RemoveFromList", &RemoveFromList);
    server.template bind<void, size_t&>("AddOne", [](size_t& n) noexcept { ::AddOne(n); });

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
    server.bind("CountChars", &::CountChars);
    server.bind("CharacterMap", &CharacterMap);
    server.bind("CountResidents", &CountResidents);
    server.bind("GetUniqueNames", &GetUniqueNames);
    server.bind("SafeDivide", &SafeDivide);
    server.bind("TopTwo", &TopTwo);
}

template<typename Serial>
static std::unique_ptr<TestServer<Serial>> CreateServer()
{
    auto p_server = std::make_unique<TestServer<Serial>>();
    BindFuncs(*p_server);
    std::thread{ &TestServer<Serial>::Run, p_server.get() }.detach();
    return p_server;
}

#if defined(RPC_HPP_ENABLE_NJSON)
std::unique_ptr<TestServer<adapters::njson_adapter>> njson_server =
    CreateServer<adapters::njson_adapter>();
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
std::unique_ptr<TestServer<adapters::rapidjson_adapter>> rapidjson_server =
    CreateServer<adapters::rapidjson_adapter>();
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
std::unique_ptr<TestServer<adapters::boost_json_adapter>> boost_json_server =
    CreateServer<adapters::boost_json_adapter>();
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
std::unique_ptr<TestServer<adapters::bitsery_adapter>> bitsery_server =
    CreateServer<adapters::bitsery_adapter>();
#endif
} //namespace rpc_hpp::tests