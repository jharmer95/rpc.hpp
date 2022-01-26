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

#define RPC_HPP_CLIENT_IMPL

#include "rpc.client.hpp"
#include "../test_structs.hpp"
#include "../static_funcs.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#if defined(RPC_HPP_ENABLE_BITSERY)
const uint64_t rpc::adapters::bitsery::config::max_func_name_size = 30;
const uint64_t rpc::adapters::bitsery::config::max_string_size = 2048;
const uint64_t rpc::adapters::bitsery::config::max_container_size = 100;
#endif

template<typename Serial>
void TestType()
{
    auto& client = GetClient<Serial>();
    const auto result = client.template call_func<int>("SimpleSum", 1, 2);

    REQUIRE(result == 3);
}

#if defined(RPC_HPP_ENABLE_NJSON)
TEST_CASE("NJSON")
{
    TestType<njson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
TEST_CASE("RAPIDJSON")
{
    TestType<rapidjson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
TEST_CASE("BOOST_JSON")
{
    TestType<boost_json_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
TEST_CASE("BITSERY")
{
    TestType<bitsery_adapter>();
}
#endif

// TODO: Clean this up somehow
#if defined(RPC_HPP_ENABLE_BITSERY)
#    if defined(TEST_USE_COMMA)
#        define TEST_BITSERY_T , bitsery_adapter
#    else
#        define TEST_BITSERY_T bitsery_adapter
#        define TEST_USE_COMMA
#    endif
#else
#    define TEST_BITSERY_T
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    if defined(TEST_USE_COMMA)
#        define TEST_BOOST_JSON_T , boost_json_adapter
#    else
#        define TEST_BOOST_JSON_T boost_json_adapter
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

#define RPC_TEST_TYPES TEST_BITSERY_T TEST_BOOST_JSON_T TEST_NJSON_T TEST_RAPIDJSON_T

TEST_CASE_TEMPLATE("CountChars (static)", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::string s = "peter piper picked a pack of pickled peppers";
    const int result = client.call_header_func(CountChars, s, 'p');

    REQUIRE(result == 9);
}

TEST_CASE_TEMPLATE("AddOne (static)", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    size_t n = 2;
    client.call_header_func(AddOne, n);
    client.call_header_func(AddOne, n);

    REQUIRE(n == 4);
}

TEST_CASE_TEMPLATE("StrLen", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    static constexpr auto test_str_len = 2048U;
    const std::string test_str(test_str_len, 'f');
    const auto result = client.template call_func<size_t>("StrLen", test_str);

    REQUIRE(result == test_str_len);
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
    static constexpr uint64_t expected = 10946;
    static constexpr uint64_t input = 20;
    auto& client = GetClient<TestType>();

    const auto test = client.template call_func<uint64_t>("Fibonacci", input);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("FibonacciRef", TestType, RPC_TEST_TYPES)
{
    static constexpr uint64_t expected = 10946;
    auto& client = GetClient<TestType>();

    uint64_t test = 20;
    client.call_func("FibonacciRef", test);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("StdDev", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 3313.695594785;
    auto& client = GetClient<TestType>();

    const auto test = client.template call_func<double>("StdDev", 55.65, 125.325, 552.125, 12.767,
        2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

    REQUIRE(test == doctest::Approx(expected));
}

TEST_CASE_TEMPLATE("SquareRootRef", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 313.2216436152;
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
    static constexpr double expected = 1731.8635996333;
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

    const ComplexObject cx{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    const auto test = client.template call_func<std::string>("HashComplex", cx);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("HashComplexRef", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    ComplexObject cx{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    // initialize empty string to pass
    std::string test{};

    // re-assign string to arg<1>
    client.call_func("HashComplexRef", cx, test);

    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("Function not found", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]
    {
        std::ignore = client.template call_func<int>("FUNC_WHICH_DOES_NOT_EXIST");
    };

    REQUIRE_THROWS_AS(exp(), rpc::exceptions::function_not_found);
}

TEST_CASE_TEMPLATE("FunctionMismatch", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto wrong_param = [&client]
    {
        std::ignore = client.template call_func<int>("SimpleSum", 2, std::string{ "Hello, world" });
    };

    const auto wrong_return = [&client]
    {
        std::ignore = client.template call_func<std::string>("SimpleSum", 1, 2);
    };

    const auto float_to_int = [&client]
    {
        std::ignore = client.template call_func<int>("SimpleSum", 2.4, 1.2);
    };

    const auto int_to_float = [&client]
    {
        std::ignore = client.template call_func<double>("StdDev", -4, 125.325, 552.125, 55, 2599.6,
            1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
    };

    const auto less_params = [&client]
    {
        std::ignore = client.template call_func<double>("StdDev", -4.2, 125.325);
    };

    const auto more_params = [&client]
    {
        std::ignore = client.template call_func<double>("StdDev", -4.2, 125.325, 552.125, 55.123,
            2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1, 111.222, 1234.56789);
    };

    REQUIRE_THROWS_AS(wrong_param(), rpc::exceptions::function_mismatch);
    REQUIRE_THROWS_AS(wrong_return(), rpc::exceptions::function_mismatch);
    REQUIRE_THROWS_AS(float_to_int(), rpc::exceptions::function_mismatch);
    REQUIRE_THROWS_AS(int_to_float(), rpc::exceptions::function_mismatch);
    REQUIRE_THROWS_AS(less_params(), rpc::exceptions::function_mismatch);
    REQUIRE_THROWS_AS(more_params(), rpc::exceptions::function_mismatch);
}

TEST_CASE_TEMPLATE("ThrowError", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]
    {
        client.call_func("ThrowError");
    };

    REQUIRE_THROWS_AS(exp(), rpc::exceptions::remote_exec_error);
}

TEST_CASE("KillServer")
{
    auto& client = GetClient<njson_adapter>();

    const auto exp = [&client]
    {
        std::ignore = client.template call_func<int>("SimpleSum", 1, 2);
    };

    try
    {
        client.call_func("KillServer");
    }
    catch (...)
    {
        // Exception is expected so continue
    }

    REQUIRE_THROWS_AS(exp(), rpc::exceptions::client_receive_error);
}
