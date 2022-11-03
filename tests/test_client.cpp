///@file test_client.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Test source file for rpc.hpp
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

#define RPC_HPP_ASSERT_THROW

#if defined(RPC_HPP_ENABLE_BITSERY)
#  define RPC_HPP_BITSERY_EXACT_SZ
#endif

#include "test_client.hpp"
#include "test_structs.hpp"
#include "static_funcs.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <numeric>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(RPC_HPP_ENABLE_BITSERY)
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_func_name_size = 30;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_string_size = 2'048;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_container_size = 1'000;
#endif

// TODO: Clean this up somehow
#if defined(RPC_HPP_ENABLE_BITSERY)
#  if defined(TEST_USE_COMMA)
#    define TEST_BITSERY_T , rpc_hpp::adapters::bitsery_adapter
#  else
#    define TEST_BITSERY_T rpc_hpp::adapters::bitsery_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_BITSERY_T
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_BOOST_JSON_T , rpc_hpp::adapters::boost_json_adapter
#  else
#    define TEST_BOOST_JSON_T rpc_hpp::adapters::boost_json_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_BOOST_JSON_T
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_NJSON_T , rpc_hpp::adapters::njson_adapter
#  else
#    define TEST_NJSON_T rpc_hpp::adapters::njson_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_NJSON_T
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_RAPIDJSON_T , rpc_hpp::adapters::rapidjson_adapter
#  else
#    define TEST_RAPIDJSON_T rpc_hpp::adapters::rapidjson_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_RAPIDJSON_T
#endif

#if !defined(TEST_USE_COMMA)
#  error At least one adapter must be enabled for testing
#endif

#define RPC_TEST_TYPES TEST_BITSERY_T TEST_BOOST_JSON_T TEST_NJSON_T TEST_RAPIDJSON_T

namespace rpc_hpp::tests
{
template<typename Serial>
static void TestType()
{
    auto p_client = GetClient<Serial>();
    const auto response = p_client->call_func("SimpleSum", 1, 2);
    CHECK(response.get_type() == rpc_type::func_result);
    REQUIRE(response.template get_result<int>() == 3);
}

#if defined(RPC_HPP_ENABLE_NJSON)
TEST_CASE("NJSON")
{
    TestType<adapters::njson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
TEST_CASE("RAPIDJSON")
{
    TestType<adapters::rapidjson_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
TEST_CASE("BOOST_JSON")
{
    TestType<adapters::boost_json_adapter>();
}
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
TEST_CASE("BITSERY")
{
    TestType<adapters::bitsery_adapter>();
}
#endif

TEST_CASE_TEMPLATE("CountChars (static)", TestType, RPC_TEST_TYPES)
{
    static constexpr char counted = 'p';
    static constexpr auto test_str = "peter piper picked a pack of pickled peppers";
    auto p_client = GetClient<TestType>();
    const auto response = p_client->call_header_func(CountChars, test_str, counted);
    REQUIRE(!response.is_error());
    REQUIRE(response.template get_result<int>() == 9);
}

TEST_CASE_TEMPLATE("AddOne (static)", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    size_t test_num = 2;
    auto response = p_client->call_header_func(AddOne, test_num);

    CHECK(!response.is_error());

    response = p_client->call_header_func(AddOne, test_num);

    CHECK(!response.is_error());
    REQUIRE(test_num == 4);
}

TEST_CASE_TEMPLATE("StrLen", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    static constexpr size_t test_str_len = 2048UL;
    const std::string test_str(test_str_len, 'f');
    const auto response = p_client->call_func("StrLen", test_str);

    static constexpr char cstr[] = "12345";
    const auto response2 = p_client->call_func("StrLen", cstr);

    CHECK(response.get_type() == rpc_type::func_result);
    REQUIRE(response.template get_result<size_t>() == test_str_len);

    CHECK(response2.get_type() == rpc_type::func_result);
    REQUIRE(response2.template get_result<size_t>() == 5);
}

TEST_CASE_TEMPLATE("AddOneToEach", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto response = p_client->call_func("AddOneToEach", vec);

    CHECK(response.get_type() == rpc_type::func_result);

    const auto result = response.template get_result<std::vector<int>>();

    REQUIRE(result.size() == vec.size());

    const auto result_sz = result.size();

    for (size_t i = 0; i < result_sz; ++i)
    {
        CHECK(result[i] == vec[i] + 1);
    }
}

TEST_CASE_TEMPLATE("AddOneToEachRef", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    std::vector<int> vec2{ 1, 3, 5, 7 };
    const auto response = p_client->call_func_w_bind("AddOneToEachRef", vec2);

    CHECK(response.get_type() == rpc_type::func_result_w_bind);
    REQUIRE(vec2.size() == vec.size());

    const auto vec2_sz = vec2.size();

    for (size_t i = 0; i < vec2_sz; ++i)
    {
        CHECK(vec2[i] == vec[i]);
    }
}

TEST_CASE_TEMPLATE("Fibonacci", TestType, RPC_TEST_TYPES)
{
    static constexpr uint64_t expected = 6'765;
    static constexpr uint64_t test_val = 20;
    auto p_client = GetClient<TestType>();

    const auto response = p_client->call_func("Fibonacci", test_val);

    CHECK(response.get_type() == rpc_type::func_result);
    REQUIRE(response.template get_result<uint64_t>() == expected);
}

TEST_CASE_TEMPLATE("FibonacciRef", TestType, RPC_TEST_TYPES)
{
    static constexpr uint64_t expected = 6'765;
    static constexpr uint64_t test_val = 20;
    auto p_client = GetClient<TestType>();

    uint64_t test = test_val;
    const auto response = p_client->call_func_w_bind("FibonacciRef", test);

    CHECK(response.get_type() == rpc_type::func_result_w_bind);
    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("StdDev", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 3313.695594785;
    auto p_client = GetClient<TestType>();

    const auto response = p_client->call_func("StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6,
        1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

    CHECK(response.get_type() == rpc_type::func_result);
    REQUIRE(response.template get_result<double>() == doctest::Approx(expected));
}

TEST_CASE_TEMPLATE("SquareRootRef", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 313.2216436152;
    auto p_client = GetClient<TestType>();

    double num1 = 55.65;
    double num2 = 125.325;
    double num3 = 552.125;
    double num4 = 12.767;
    double num5 = 2599.6;
    double num6 = 1245.125663;
    double num7 = 9783.49;
    double num8 = 125.12;
    double num9 = 553.3333333333;
    double num10 = 2266.1;

    const auto response = p_client->call_func_w_bind(
        "SquareRootRef", num1, num2, num3, num4, num5, num6, num7, num8, num9, num10);

    CHECK(response.get_type() == rpc_type::func_result_w_bind);

    const auto test = num1 + num2 + num3 + num4 + num5 + num6 + num7 + num8 + num9 + num10;
    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("AverageContainer<double>", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 1731.8635996333;
    auto p_client = GetClient<TestType>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto response = p_client->call_func("AverageContainer<double>", vec);

    CHECK(response.get_type() == rpc_type::func_result);
    REQUIRE(response.template get_result<double>() == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("SquareArray", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    std::array<int, 12> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    const auto response = p_client->call_func_w_bind("SquareArray", arr);
    CHECK(response.get_type() == rpc_type::func_result_w_bind);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[11] == 144);
}

TEST_CASE_TEMPLATE("RemoveFromList", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    std::forward_list<std::string> word_list{ "Test", "word", "fox", "test", "sphere", "Word",
        "test", "Test" };

    const auto response1 = p_client->call_func_w_bind("RemoveFromList", word_list, "Word", false);
    CHECK(response1.get_type() == rpc_type::func_result_w_bind);
    REQUIRE(std::distance(word_list.begin(), word_list.end()) == 6);

    const auto response2 = p_client->call_func_w_bind("RemoveFromList", word_list, "test", true);
    CHECK(response2.get_type() == rpc_type::func_result_w_bind);
    REQUIRE(std::distance(word_list.begin(), word_list.end()) == 4);
}

TEST_CASE_TEMPLATE("CharacterMap", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const std::string str = "The quick brown fox ran over the hill last night";

    const auto response = p_client->call_func("CharacterMap", str);

    CHECK(response.get_type() == rpc_type::func_result);

    const auto char_map = response.template get_result<std::map<char, unsigned>>();

    REQUIRE(!char_map.empty());
    REQUIRE(char_map.at('e') == 3U);
    REQUIRE(char_map.at('x') == 1U);
}

TEST_CASE_TEMPLATE("CountResidents", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const std::multimap<int, std::string> registry{ { 1, "Fred Jones" }, { 1, "Ron Taylor" },
        { 1, "Janice Filber" }, { 2, "Peter Reynolds" }, { 2, "Jonathan Fields" },
        { 3, "Dorothy Petras" } };

    const auto response1 = p_client->call_func("CountResidents", registry, 1);
    CHECK(response1.get_type() == rpc_type::func_result);
    const auto result1 = response1.template get_result<size_t>();
    REQUIRE(result1 == 3UL);

    const auto response2 = p_client->call_func("CountResidents", registry, 4);
    CHECK(response2.get_type() == rpc_type::func_result);
    const auto result2 = response2.template get_result<size_t>();
    REQUIRE(result2 == 0UL);
}

TEST_CASE_TEMPLATE("GetUniqueNames", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const std::vector<std::string> names{ "John", "Frank", "Susan", "John", "Darlene", "Frank",
        "John", "Steve" };

    const auto response = p_client->call_func("GetUniqueNames", names);

    CHECK(response.get_type() == rpc_type::func_result);

    const auto result = response.template get_result<std::unordered_set<std::string>>();
    REQUIRE(!result.empty());
    REQUIRE(result.size() == 5UL);
}

TEST_CASE_TEMPLATE("SafeDivide", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const auto response1 = p_client->call_func("SafeDivide", 10, 2);

    CHECK(response1.get_type() == rpc_type::func_result);

    const auto result1 = response1.template get_result<std::optional<int>>();
    REQUIRE(result1.has_value());
    REQUIRE(result1.value() == 5);

    const auto response2 = p_client->call_func("SafeDivide", 10, 0);

    CHECK(response2.get_type() == rpc_type::func_result);

    const auto result2 = response2.template get_result<std::optional<int>>();
    REQUIRE(!result2.has_value());
}

TEST_CASE_TEMPLATE("TopTwo", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    static constexpr std::pair<int, int> expected{ 7382, 6668 };
    const std::vector<int> vec{ -9022, -122, 6668, 3853, -9304, -2002, -4100, -8521, -8155, -9358,
        485, -4806, -2263, 7382, -696, 5695, -2946, 3698, -2103, -4112, 3001, -686, -5925, -8116,
        -1509, 1537, -3898, -6371, -2197, 369 };

    const auto response = p_client->call_func("TopTwo", vec);

    CHECK(response.get_type() == rpc_type::func_result);

    const auto result = response.template get_result<std::pair<int, int>>();
    REQUIRE(result.first == expected.first);
    REQUIRE(result.second == expected.second);
}

TEST_CASE_TEMPLATE("HashComplex", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto p_client = GetClient<TestType>();

    const ComplexObject test_obj{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    const auto response = p_client->call_func("HashComplex", test_obj);

    CHECK(response.get_type() == rpc_type::func_result);
    REQUIRE(response.template get_result<std::string>() == expected);
}

TEST_CASE_TEMPLATE("HashComplexRef", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto p_client = GetClient<TestType>();

    ComplexObject test_obj{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    // initialize empty string to pass
    std::string test{};

    // re-assign string to arg<1>
    const auto response = p_client->call_func_w_bind("HashComplexRef", test_obj, test);

    CHECK(response.get_type() == rpc_type::func_result_w_bind);
    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("GetConnectionInfo", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    auto callback_request = p_client->template install_callback<std::string>(
        "GetClientName", [] { return std::string{ "MyClient" }; });

    const auto response = p_client->call_func("GetConnectionInfo");

    CHECK(response.get_type() == rpc_type::func_result);

    const auto value = response.template get_result<std::string>();
    REQUIRE(!value.empty());

    p_client->uninstall_callback(std::move(callback_request));
}

TEST_CASE_TEMPLATE("Callback already installed", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    auto callback_request = p_client->template install_callback<std::string>(
        "TestCallback", [] { return "Hello, callback!"; });

    REQUIRE(callback_request.func_name == "TestCallback");
    REQUIRE_THROWS_AS((std::ignore = p_client->template install_callback<std::string>(
                           "TestCallback", [] { return "Goodbye, callback!"; })),
        callback_install_error);

    p_client->uninstall_callback(std::move(callback_request));
}

TEST_CASE_TEMPLATE("Function not found", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const auto response = p_client->call_func("FUNC_WHICH_DOES_NOT_EXIST");

    REQUIRE(response.is_error());
    REQUIRE(response.get_error_type() == exception_type::function_missing);
}

TEST_CASE_TEMPLATE("FunctionMismatch", TestType, RPC_TEST_TYPES)
{
#if defined(RPC_HPP_ENABLE_BITSERY)
    // TODO: Figure out why bitsery isn't reporting errors
    if constexpr (!std::is_same_v<TestType, adapters::bitsery_adapter>)
    {
#endif
        auto p_client = GetClient<TestType>();

        rpc_object<TestType> obj =
            p_client->call_func("SimpleSum", 2, std::string{ "Hello, world" });

        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == exception_type::func_signature_mismatch);

        obj = p_client->call_func("SimpleSum", 1, 2);
        REQUIRE(obj.get_type() == rpc_type::func_result);
        REQUIRE_THROWS_AS(
            (std::ignore = obj.template get_result<std::string>()), function_mismatch_error);

        obj = p_client->call_func("SimpleSum", 2.4, 1.2);
        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == exception_type::func_signature_mismatch);

        obj = p_client->call_func("StdDev", -4.2, 125.325, 552.125, 55.123, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1, 111.222, 1234.56789);

        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == exception_type::func_signature_mismatch);

        obj = p_client->call_func("StdDev", -4, 125.325, 552.125, 55, 2599.6, 1245.125663, 9783.49,
            125.12, 553.3333333333, 2266.1);
        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == exception_type::func_signature_mismatch);

        obj = p_client->call_func("StdDev", -4.2, 125.325);
        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == exception_type::func_signature_mismatch);
#if defined(RPC_HPP_ENABLE_BITSERY)
    }
#endif
}

TEST_CASE_TEMPLATE("ThrowError", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const auto bad_call = [&p_client]
    {
        p_client->call_func("ThrowError").template get_result<void>();
    };

    REQUIRE_THROWS_AS(bad_call(), remote_exec_error);
}

TEST_CASE_TEMPLATE("InvalidObject", TestType, RPC_TEST_TYPES)
{
    static constexpr size_t test_sz = 8UL;
    typename TestType::bytes_t bytes(test_sz, {});

    //std::iota(bytes.begin(), bytes.end(), typename TestType::bytes_t::value_type{});
    bytes[0] = 6;

    auto p_client = GetClient<TestType>();
    p_client->send(std::move(bytes));
    bytes = p_client->receive();

    auto rpc_obj = rpc_object<TestType>::parse_bytes(std::move(bytes));

    REQUIRE(rpc_obj.has_value());

    const auto& response = rpc_obj.value();

    REQUIRE(response.is_error());
    REQUIRE(response.get_error_type() == exception_type::server_receive);
}

TEST_CASE_TEMPLATE("KillServer", TestType, RPC_TEST_TYPES)
{
    auto p_client = GetClient<TestType>();

    const auto dead_call = [&p_client]
    {
        std::ignore = p_client->call_func("SimpleSum", 1, 2).template get_result<int>();
    };

    const auto kill_server = [&p_client]
    {
        p_client->call_func("KillServer").template get_result<void>();
    };

    REQUIRE_THROWS_AS(kill_server(), client_receive_error);
    REQUIRE_THROWS_AS(dead_call(), client_receive_error);
}
} //namespace rpc_hpp::tests
