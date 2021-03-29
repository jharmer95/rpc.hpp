///@file rpc.test.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Unit test source file for rpc.hpp
///@version 0.3.3
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

#include <catch2/catch.hpp>

#if defined(RPC_HPP_ENABLE_NJSON)
#    include "rpc_adapters/rpc_njson.hpp"

using rpc::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include "rpc_adapters/rpc_rapidjson.hpp"

using rpc::adapters::rapidjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include "rpc_adapters/rpc_boost_json.hpp"

using rpc::adapters::bjson_adapter;
#endif

#include "rpc.client.hpp"
#include "test_structs.hpp"

template<typename Serial>
TestClient& GetClient();

template<typename Serial>
void TestType()
{
    auto& client = GetClient<Serial>();
    const auto result =
        rpc::client::call_func<njson_adapter, int>(client, "SimpleSum", 1, 2).get_result();

    REQUIRE(result == 3);
}

#if defined(RPC_HPP_ENABLE_NJSON)
template<>
TestClient& GetClient<njson_adapter>()
{
    static TestClient client("127.0.0.1", "5000");
    return client;
}

TEST_CASE("NJSON")
{
    TestType<njson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
template<>
TestClient& GetClient<rapidjson_adapter>()
{
    static TestClient client("127.0.0.1", "5001");
    return client;
}

TEST_CASE("RAPIDJSON")
{
    TestType<rapidjson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
template<>
TestClient& GetClient<bjson_adapter>()
{
    static TestClient client("127.0.0.1", "5002");
    return client;
}

TEST_CASE("BOOST_JSON")
{
    TestType<bjson_adapter>();
}
#endif

// TODO: Clean this up somehow
#if defined(RPC_HPP_ENABLE_NJSON)
#    if defined(RPC_HPP_ENABLE_RAPIDJSON)
#        if defined(RPC_HPP_ENABLE_BOOST_JSON)
using test_types_t = std::tuple<njson_adapter, rapidjson_adapter, bjson_adapter>;
#        else
using test_types_t = std::tuple<njson_adapter, rapidjson_adapter>;
#        endif
#    elif defined(RPC_HPP_ENABLE_BOOST_JSON)
using test_types_t = std::tuple<njson_adapter, bjson_adapter>;
#    else
using test_types_t = std::tuple<njson_adapter>;
#    endif
#elif defined(RPC_HPP_ENABLE_RAPIDJSON)
#    if defined(RPC_HPP_ENABLE_BOOST_JSON)
using test_types_t = std::tuple<rapidjson_adapter, bjson_adapter>;
#    else
using test_types_t = std::tuple<rapidjson_adapter>;
#    endif
#elif defined(RPC_HPP_ENABLE_BOOST_JSON)
using test_types_t = std::tuple<bjson_adapter>;
#endif

TEMPLATE_LIST_TEST_CASE("StrLen", "", test_types_t)
{
    auto& client = GetClient<TestType>();
    const auto result =
        rpc::call_func<TestType, int>(client, "StrLen", std::string("hello, world")).get_result();

    REQUIRE(result == 12);
}

TEMPLATE_LIST_TEST_CASE("AddOneToEach", "", test_types_t)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto result =
        rpc::call_func<TestType, std::vector<int>>(client, "AddOneToEach", vec).get_result();

    REQUIRE(result.size() == vec.size());

    for (size_t i = 0; i < result.size(); ++i)
    {
        REQUIRE(result[i] == vec[i] + 1);
    }
}

TEMPLATE_LIST_TEST_CASE("AddOneToEachRef", "", test_types_t)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = rpc::call_func<TestType>(client, "AddOneToEachRef", vec);

    const auto vec2 = pack.get_arg<0>();

    REQUIRE(vec2.size() == vec.size());

    for (size_t i = 0; i < vec2.size(); ++i)
    {
        REQUIRE(vec2[i] == vec[i] + 1);
    }
}

TEMPLATE_LIST_TEST_CASE("Fibonacci", "", test_types_t)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<TestType>();

    const auto test = rpc::call_func<TestType, uint64_t>(client, "Fibonacci", 20).get_result();
    REQUIRE(expected == test);
}

TEMPLATE_LIST_TEST_CASE("FibonacciRef", "", test_types_t)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<TestType>();

    uint64_t num = 20ULL;
    const auto test = rpc::call_func<TestType>(client, "FibonacciRef", num).get_arg<0>();

    REQUIRE(expected == test);
}

TEMPLATE_LIST_TEST_CASE("StdDev", "", test_types_t)
{
    constexpr double expected = 3313.695594785;
    auto& client = GetClient<TestType>();

    const auto test = rpc::call_func<TestType, double>(client, "StdDev", 55.65, 125.325, 552.125,
        12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                          .get_result();

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
}

TEMPLATE_LIST_TEST_CASE("SquareRootRef", "", test_types_t)
{
    constexpr double expected = 313.2216436152;
    auto& client = GetClient<TestType>();

    double n1 = 55.65;
    double n2 = 125.325;
    double n3 = 552.125;
    double n4 = 12.767;
    double n5 = 2599.6;
    double n6 = 1245.125663;
    double n7 = 9783.49;
    double n8 = 125.12;
    double n9 = 553.3333333333;
    double n10 = 2266.1;

    const auto pack =
        rpc::call_func<TestType>(client, "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

    n1 = pack.get_arg<0>();
    n2 = pack.get_arg<1>();
    n3 = pack.get_arg<2>();
    n4 = pack.get_arg<3>();
    n5 = pack.get_arg<4>();
    n6 = pack.get_arg<5>();
    n7 = pack.get_arg<6>();
    n8 = pack.get_arg<7>();
    n9 = pack.get_arg<8>();
    n10 = pack.get_arg<9>();

    const auto test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
}

TEMPLATE_LIST_TEST_CASE("AverageContainer<double>", "", test_types_t)
{
    constexpr double expected = 1731.8635996333;
    auto& client = GetClient<TestType>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto test =
        rpc::call_func<TestType, double>(client, "AverageContainer<double>", vec).get_result();

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
}

TEMPLATE_LIST_TEST_CASE("HashComplex", "", test_types_t)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    const auto test = rpc::call_func<TestType, std::string>(client, "HashComplex", cx).get_result();

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

TEMPLATE_LIST_TEST_CASE("HashComplexRef", "", test_types_t)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    // initialize empty string to pass
    std::string test{};

    // re-assign string to arg<1>
    test = rpc::call_func<TestType>(client, "HashComplexRef", cx, test).get_arg<1>();

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

// TEMPLATE_LIST_TEST_CASE("Function not found", "", test_types_t)
// {
//     auto& client = GetClient<TestType>();

//     const auto exp = [&client]() {
//         rpc::call_func<TestType, int>(client, "FUNC_WHICH_DOES_NOT_EXIST");
//     };

//     REQUIRE_THROWS_WITH(exp(),
//         Catch::Matchers::Equals(
//             "RPC error: Called function: \"FUNC_WHICH_DOES_NOT_EXIST\" not found!"));
// }

// TEMPLATE_LIST_TEST_CASE("ThrowError", "", test_types_t)
// {
//     auto& client = GetClient<TestType>();

//     const auto exp = [&client]() { rpc::call_func<TestType, int>(client, "ThrowError"); };

//     REQUIRE_THROWS_WITH(exp(), "THIS IS A TEST ERROR!");
// }

TEST_CASE("KillServer", "[!mayfail]")
{
    auto& client = GetClient<njson_adapter>();

    try
    {
        rpc::call_func<njson_adapter>(client, "KillServer");
    }
    catch (...)
    {
    }

    REQUIRE_THROWS(TestType<njson_adapter>());
}
