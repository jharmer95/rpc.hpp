///@file rpc.test.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Unit test source file for rpc.hpp
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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define RPC_HPP_CLIENT_IMPL

#include "rpc.client.hpp"
#include "test_structs.hpp"

#include <doctest/doctest.h>

template<typename Serial>
TestClient<Serial>& GetClient();

template<typename Serial>
void TestType()
{
    auto& client = GetClient<Serial>();
    const auto result = client.template call_func<int>("SimpleSum", 1, 2);

    REQUIRE(result == 3);
}

#if defined(RPC_HPP_ENABLE_NJSON)
template<>
TestClient<njson_adapter>& GetClient()
{
    static TestClient<njson_adapter> client("127.0.0.1", "5000");
    return client;
}

TEST_CASE("NJSON")
{
    TestType<njson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
template<>
TestClient<rapidjson_adapter>& GetClient()
{
    static TestClient<rapidjson_adapter> client("127.0.0.1", "5001");
    return client;
}

TEST_CASE("RAPIDJSON")
{
    TestType<rapidjson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
template<>
TestClient<bjson_adapter>& GetClient()
{
    static TestClient<bjson_adapter> client("127.0.0.1", "5002");
    return client;
}

TEST_CASE("BOOST_JSON")
{
    TestType<bjson_adapter>();
}
#endif

// TODO: Clean this up somehow
#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    if defined(TEST_USE_COMMA)
#        define TEST_BOOST_JSON_T , bjson_adapter
#    else
#        define TEST_BOOST_JSON_T bjson_adapter
#        define TEST_USE_COMMA
#    endif
#else
#    define TEST_BOOST_JSON_T
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#    if defined(TEST_USE_COMMA)
#        define TEST_NJSON_T , njson_adapter
#    else
#        define TEST_NJSON_T njson_adapter
#        define TEST_USE_COMMA
#    endif
#else
#    define TEST_NJSON_T
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    if defined(TEST_USE_COMMA)
#        define TEST_RAPIDJSON_T , rapidjson_adapter
#    else
#        define TEST_RAPIDJSON_T rapidjson_adapter
#        define TEST_USE_COMMA
#    endif
#else
#    define TEST_RAPIDJSON_T
#endif

#define RPC_TEST_TYPES TEST_BOOST_JSON_T TEST_NJSON_T TEST_RAPIDJSON_T

TEST_CASE_TEMPLATE("StrLen", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const auto result = client.template call_func<int>("StrLen", std::string("hello, world"));

    REQUIRE(result == 12);
}

TEST_CASE_TEMPLATE("AddOneToEach", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto result = client.template call_func<std::vector<int>>("AddOneToEach", vec);

    REQUIRE(result.size() == vec.size());

    for (size_t i = 0; i < result.size(); ++i)
    {
        REQUIRE(result[i] == vec[i] + 1);
    }
}

TEST_CASE_TEMPLATE("AddOneToEachRef", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    std::vector<int> vec2{ 1, 3, 5, 7 };
    client.call_func("AddOneToEachRef", vec2);

    REQUIRE(vec2.size() == vec.size());

    for (size_t i = 0; i < vec2.size(); ++i)
    {
        REQUIRE(vec2[i] == vec[i]);
    }
}

TEST_CASE_TEMPLATE("Fibonacci", TestType, RPC_TEST_TYPES)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<TestType>();

    const auto test = client.template call_func<uint64_t>("Fibonacci", 20);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("FibonacciRef", TestType, RPC_TEST_TYPES)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<TestType>();

    uint64_t test = 20ULL;
    client.call_func("FibonacciRef", test);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("StdDev", TestType, RPC_TEST_TYPES)
{
    constexpr double expected = 3313.695594785;
    auto& client = GetClient<TestType>();

    const auto test = client.template call_func<double>("StdDev", 55.65, 125.325, 552.125, 12.767,
        2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

    REQUIRE(test == doctest::Approx(expected));
}

TEST_CASE_TEMPLATE("SquareRootRef", TestType, RPC_TEST_TYPES)
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

    client.call_func("SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

    const auto test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("AverageContainer<double>", TestType, RPC_TEST_TYPES)
{
    constexpr double expected = 1731.8635996333;
    auto& client = GetClient<TestType>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto test = client.template call_func<double>("AverageContainer<double>", vec);

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("HashComplex", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    const auto test = client.template call_func<std::string>("HashComplex", cx);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("HashComplexRef", TestType, RPC_TEST_TYPES)
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
    client.call_func("HashComplexRef", cx, test);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("Function not found", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]()
    { [[maybe_unused]] int _unused = client.template call_func<int>("FUNC_WHICH_DOES_NOT_EXIST"); };

    REQUIRE_THROWS_WITH(exp(), "RPC error: Called function: \"FUNC_WHICH_DOES_NOT_EXIST\" not found!");
}

TEST_CASE_TEMPLATE("ThrowError", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]()
    { [[maybe_unused]] int _unused = client.template call_func<int>("ThrowError"); };

    REQUIRE_THROWS_WITH(exp(), "THIS IS A TEST ERROR!");
}

TEST_CASE("KillServer")
{
    auto& client = GetClient<njson_adapter>();

    try
    {
        client.call_func("KillServer");
    }
    catch (...)
    {
    }

    REQUIRE_THROWS(TestType<njson_adapter>());
}
