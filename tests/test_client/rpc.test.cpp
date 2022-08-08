///@file rpc.test.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Unit test source file for rpc.hpp
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

#define RPC_HPP_CLIENT_IMPL

#include "rpc.client.hpp"
#include "../test_structs.hpp"
#include "../static_funcs.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#if defined(RPC_HPP_ENABLE_BITSERY)
constexpr uint64_t bitsery_adapter::config::max_func_name_size = 30;
constexpr uint64_t bitsery_adapter::config::max_string_size = 2048;
constexpr uint64_t bitsery_adapter::config::max_container_size = 100;
#endif

template<typename Serial>
void TestType()
{
    auto& client = GetClient<Serial>();
    const auto response = client.call_func("SimpleSum", 1, 2);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response.template get_result<int>() == 3);
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
#  if defined(TEST_USE_COMMA)
#    define TEST_BITSERY_T , bitsery_adapter
#  else
#    define TEST_BITSERY_T bitsery_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_BITSERY_T
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_BOOST_JSON_T , boost_json_adapter
#  else
#    define TEST_BOOST_JSON_T boost_json_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_BOOST_JSON_T
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_NJSON_T , njson_adapter
#  else
#    define TEST_NJSON_T njson_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_NJSON_T
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_RAPIDJSON_T , rapidjson_adapter
#  else
#    define TEST_RAPIDJSON_T rapidjson_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_RAPIDJSON_T
#endif

#define RPC_TEST_TYPES TEST_BITSERY_T TEST_BOOST_JSON_T TEST_NJSON_T TEST_RAPIDJSON_T

TEST_CASE_TEMPLATE("CountChars (static)", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::string s = "peter piper picked a pack of pickled peppers";
    const auto response = client.call_header_func(CountChars, s, 'p');

    REQUIRE(!response.is_error());
    REQUIRE(response.template get_result<int>() == 9);
}

TEST_CASE_TEMPLATE("AddOne (static)", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    size_t n = 2;
    auto response = client.call_header_func(AddOne, n);

    REQUIRE(!response.is_error());

    response = client.call_header_func(AddOne, n);

    REQUIRE(!response.is_error());
    REQUIRE(n == 4);
}

TEST_CASE_TEMPLATE("StrLen", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    static constexpr auto test_str_len = 2048U;
    const std::string test_str(test_str_len, 'f');
    const auto response = client.call_func("StrLen", test_str);

    static constexpr char cstr[] = "12345";
    const auto response2 = client.call_func("StrLen", cstr);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response.template get_result<size_t>() == test_str_len);

    REQUIRE(response2.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response2.template get_result<size_t>() == 5);
}

TEST_CASE_TEMPLATE("AddOneToEach", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto response = client.call_func("AddOneToEach", vec);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);

    const auto result = response.template get_result<std::vector<int>>();

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
    const auto response = client.call_func_w_bind("AddOneToEachRef", vec2);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result_w_bind);
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

    const auto response = client.call_func("Fibonacci", input);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response.template get_result<uint64_t>() == expected);
}

TEST_CASE_TEMPLATE("FibonacciRef", TestType, RPC_TEST_TYPES)
{
    static constexpr uint64_t expected = 10946;
    auto& client = GetClient<TestType>();

    uint64_t test = 20;
    const auto response = client.call_func_w_bind("FibonacciRef", test);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result_w_bind);
    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("StdDev", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 3313.695594785;
    auto& client = GetClient<TestType>();

    const auto response = client.call_func("StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6,
        1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response.template get_result<double>() == doctest::Approx(expected));
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

    const auto response =
        client.call_func_w_bind("SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result_w_bind);

    const auto test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("AverageContainer<double>", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 1731.8635996333;
    auto& client = GetClient<TestType>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto response = client.call_func("AverageContainer<double>", vec);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response.template get_result<double>() == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("HashComplex", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    const ComplexObject cx{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    const auto response = client.call_func("HashComplex", cx);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE(response.template get_result<std::string>() == expected);
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
    const auto response = client.call_func_w_bind("HashComplexRef", cx, test);

    REQUIRE(response.type() == rpc_hpp::rpc_object_type::func_result_w_bind);
    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("Function not found", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto response = client.call_func("FUNC_WHICH_DOES_NOT_EXIST");

    REQUIRE(response.is_error());
    REQUIRE(response.get_error_type() == rpc_hpp::exception_type::func_not_found);
}

TEST_CASE_TEMPLATE("FunctionMismatch", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    rpc_hpp::rpc_object<TestType> obj =
        client.call_func("SimpleSum", 2, std::string{ "Hello, world" });

    REQUIRE(obj.is_error());
    REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

    obj = client.call_func("SimpleSum", 1, 2);
    REQUIRE(obj.type() == rpc_hpp::rpc_object_type::func_result);
    REQUIRE_THROWS_AS(
        (std::ignore = obj.template get_result<std::string>()), rpc_hpp::function_mismatch);

    obj = client.call_func("SimpleSum", 2.4, 1.2);
    REQUIRE(obj.is_error());
    REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

    obj = client.call_func("StdDev", -4, 125.325, 552.125, 55, 2599.6, 1245.125663, 9783.49, 125.12,
        553.3333333333, 2266.1);
    REQUIRE(obj.is_error());
    REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

    obj = client.call_func("StdDev", -4.2, 125.325);
    REQUIRE(obj.is_error());
    REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

    obj = client.call_func("StdDev", -4.2, 125.325, 552.125, 55.123, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1, 111.222, 1234.56789);
    REQUIRE(obj.is_error());
    REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);
}

TEST_CASE_TEMPLATE("ThrowError", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]
    {
        client.call_func("ThrowError").template get_result<void>();
    };

    REQUIRE_THROWS_AS(exp(), rpc_hpp::remote_exec_error);
}

TEST_CASE_TEMPLATE("InvalidObject", TestType, RPC_TEST_TYPES)
{
#if defined(RPC_HPP_ENABLE_BITSERY)
    if (std::is_same_v<TestType, bitsery_adapter>)
    {
        // TODO: Verify bitsery data somehow
        return;
    }
#endif

    typename TestType::bytes_t bytes{};
    bytes.resize(8);

    std::iota(bytes.begin(), bytes.end(), 0);

    auto& client = GetClient<TestType>();
    client.send(std::move(bytes));
    bytes = client.receive();
    auto rpc_obj = rpc_hpp::rpc_object<TestType>::parse_bytes(std::move(bytes));

    REQUIRE(rpc_obj.has_value());

    const auto& response = rpc_obj.value();

    REQUIRE(response.is_error());
    REQUIRE(response.get_error_type() == rpc_hpp::exception_type::server_receive);
}

TEST_CASE("KillServer")
{
    auto& client = GetClient<njson_adapter>();

    const auto exp = [&client]
    {
        std::ignore = client.call_func("SimpleSum", 1, 2);
    };

    try
    {
        std::ignore = client.call_func("KillServer");
    }
    catch (...)
    {
        // Exception is expected so continue
    }

    REQUIRE_THROWS_AS(exp(), rpc_hpp::client_receive_error);
}
