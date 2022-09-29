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

#define RPC_HPP_ENABLE_CALLBACKS

#include "rpc.client.hpp"
#include "../test_structs.hpp"
#include "../static_funcs.hpp"

#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <rpc_adapters/rpc_bitsery.hpp>
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  include <rpc_adapters/rpc_boost_json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#  include <rpc_adapters/rpc_njson.hpp>
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  include <rpc_adapters/rpc_rapidjson.hpp>
#endif

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <forward_list>
#include <map>
#include <unordered_set>

#if defined(RPC_HPP_ENABLE_BITSERY)
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_func_name_size = 30;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_string_size = 2'048;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_container_size = 1'000;
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

#if !defined(TEST_USE_COMMA)
#  error At least one adapter must be enabled for testing
#endif

#define RPC_TEST_TYPES TEST_BITSERY_T TEST_BOOST_JSON_T TEST_NJSON_T TEST_RAPIDJSON_T

namespace rpc_hpp::tests
{
template<typename Serial>
static TestClient<Serial>& GetClient();

#if defined(RPC_HPP_ENABLE_NJSON)
using adapters::njson_adapter;

template<>
[[nodiscard]] inline TestClient<njson_adapter>& GetClient()
{
    static TestClient<njson_adapter> njson_client{ "127.0.0.1", "5000" };
    return njson_client;
}
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
using adapters::rapidjson_adapter;

template<>
[[nodiscard]] inline TestClient<rapidjson_adapter>& GetClient()
{
    static TestClient<rapidjson_adapter> rapidjson_client{ "127.0.0.1", "5001" };
    return rapidjson_client;
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
using adapters::boost_json_adapter;

template<>
[[nodiscard]] inline TestClient<boost_json_adapter>& GetClient()
{
    static TestClient<boost_json_adapter> boost_json_client{ "127.0.0.1", "5002" };
    return boost_json_client;
}
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
using adapters::bitsery_adapter;

template<>
[[nodiscard]] inline TestClient<bitsery_adapter>& GetClient()
{
    static TestClient<bitsery_adapter> bitsery_client{ "127.0.0.1", "5003" };
    return bitsery_client;
}
#endif

template<typename Serial>
static void TestType()
{
    auto& client = GetClient<Serial>();
    const auto response = client.call_func("SimpleSum", 1, 2);
    CHECK(response.type() == rpc_hpp::rpc_type::func_result);
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

TEST_CASE_TEMPLATE("CountChars (static)", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::string test_str = "peter piper picked a pack of pickled peppers";
    const auto response = client.call_header_func(CountChars, test_str, 'p');

    REQUIRE(!response.is_error());
    REQUIRE(response.template get_result<int>() == 9);
}

TEST_CASE_TEMPLATE("AddOne (static)", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    size_t test_num = 2;
    auto response = client.call_header_func(AddOne, test_num);

    CHECK(!response.is_error());

    response = client.call_header_func(AddOne, test_num);

    CHECK(!response.is_error());
    REQUIRE(test_num == 4);
}

TEST_CASE_TEMPLATE("StrLen", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    static constexpr size_t test_str_len = 2048UL;
    const std::string test_str(test_str_len, 'f');
    const auto response = client.call_func("StrLen", test_str);

    static constexpr char cstr[] = "12345";
    const auto response2 = client.call_func("StrLen", cstr);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);
    REQUIRE(response.template get_result<size_t>() == test_str_len);

    CHECK(response2.type() == rpc_hpp::rpc_type::func_result);
    REQUIRE(response2.template get_result<size_t>() == 5);
}

TEST_CASE_TEMPLATE("AddOneToEach", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto response = client.call_func("AddOneToEach", vec);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);

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
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    std::vector<int> vec2{ 1, 3, 5, 7 };
    const auto response = client.call_func_w_bind("AddOneToEachRef", vec2);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result_w_bind);
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
    auto& client = GetClient<TestType>();

    const auto response = client.call_func("Fibonacci", test_val);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);
    REQUIRE(response.template get_result<uint64_t>() == expected);
}

TEST_CASE_TEMPLATE("FibonacciRef", TestType, RPC_TEST_TYPES)
{
    static constexpr uint64_t expected = 6'765;
    static constexpr uint64_t test_val = 20;
    auto& client = GetClient<TestType>();

    uint64_t test = test_val;
    const auto response = client.call_func_w_bind("FibonacciRef", test);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result_w_bind);
    REQUIRE(expected == test);
}

TEST_CASE_TEMPLATE("StdDev", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 3313.695594785;
    auto& client = GetClient<TestType>();

    const auto response = client.call_func("StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6,
        1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);
    REQUIRE(response.template get_result<double>() == doctest::Approx(expected));
}

TEST_CASE_TEMPLATE("SquareRootRef", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 313.2216436152;
    auto& client = GetClient<TestType>();

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

    const auto response = client.call_func_w_bind(
        "SquareRootRef", num1, num2, num3, num4, num5, num6, num7, num8, num9, num10);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result_w_bind);

    const auto test = num1 + num2 + num3 + num4 + num5 + num6 + num7 + num8 + num9 + num10;
    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("AverageContainer<double>", TestType, RPC_TEST_TYPES)
{
    static constexpr double expected = 1731.8635996333;
    auto& client = GetClient<TestType>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto response = client.call_func("AverageContainer<double>", vec);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);
    REQUIRE(response.template get_result<double>() == doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE("SquareArray", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    std::array<int, 12> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    const auto response = client.call_func_w_bind("SquareArray", arr);
    CHECK(response.type() == rpc_hpp::rpc_type::func_result_w_bind);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[11] == 144);
}

TEST_CASE_TEMPLATE("RemoveFromList", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    std::forward_list<std::string> word_list{ "Test", "word", "fox", "test", "sphere", "Word",
        "test", "Test" };

    const auto response1 = client.call_func_w_bind("RemoveFromList", word_list, "Word", false);
    CHECK(response1.type() == rpc_hpp::rpc_type::func_result_w_bind);
    REQUIRE(std::distance(word_list.begin(), word_list.end()) == 6);

    const auto response2 = client.call_func_w_bind("RemoveFromList", word_list, "test", true);
    CHECK(response2.type() == rpc_hpp::rpc_type::func_result_w_bind);
    REQUIRE(std::distance(word_list.begin(), word_list.end()) == 4);
}

TEST_CASE_TEMPLATE("CharacterMap", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const std::string str = "The quick brown fox ran over the hill last night";

    const auto response = client.call_func("CharacterMap", str);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);

    const auto char_map = response.template get_result<std::map<char, unsigned>>();

    REQUIRE(!char_map.empty());
    REQUIRE(char_map.at('e') == 3U);
    REQUIRE(char_map.at('x') == 1U);
}

TEST_CASE_TEMPLATE("CountResidents", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const std::multimap<int, std::string> registry{ { 1, "Fred Jones" }, { 1, "Ron Taylor" },
        { 1, "Janice Filber" }, { 2, "Peter Reynolds" }, { 2, "Jonathan Fields" },
        { 3, "Dorothy Petras" } };

    const auto response1 = client.call_func("CountResidents", registry, 1);
    CHECK(response1.type() == rpc_hpp::rpc_type::func_result);
    const auto result1 = response1.template get_result<size_t>();
    REQUIRE(result1 == 3UL);

    const auto response2 = client.call_func("CountResidents", registry, 4);
    CHECK(response2.type() == rpc_hpp::rpc_type::func_result);
    const auto result2 = response2.template get_result<size_t>();
    REQUIRE(result2 == 0UL);
}

TEST_CASE_TEMPLATE("GetUniqueNames", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const std::vector<std::string> names{ "John", "Frank", "Susan", "John", "Darlene", "Frank",
        "John", "Steve" };

    const auto response = client.call_func("GetUniqueNames", names);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);

    const auto result = response.template get_result<std::unordered_set<std::string>>();
    REQUIRE(!result.empty());
    REQUIRE(result.size() == 5UL);
}

TEST_CASE_TEMPLATE("HashComplex", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    const ComplexObject test_obj{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    const auto response = client.call_func("HashComplex", test_obj);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);
    REQUIRE(response.template get_result<std::string>() == expected);
}

TEST_CASE_TEMPLATE("HashComplexRef", TestType, RPC_TEST_TYPES)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<TestType>();

    ComplexObject test_obj{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    // initialize empty string to pass
    std::string test{};

    // re-assign string to arg<1>
    const auto response = client.call_func_w_bind("HashComplexRef", test_obj, test);

    CHECK(response.type() == rpc_hpp::rpc_type::func_result_w_bind);
    REQUIRE(expected == test);
}

#if defined(RPC_HPP_ENABLE_CALLBACKS)
TEST_CASE_TEMPLATE("GetConnectionInfo", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    auto callback_request = client.template install_callback<std::string>(
        "GetClientName", [] { return std::string{ "MyClient" }; });

    const auto response = client.call_func("GetConnectionInfo");

    CHECK(response.type() == rpc_hpp::rpc_type::func_result);

    const auto value = response.template get_result<std::string>();
    REQUIRE(!value.empty());

    client.uninstall_callback(std::move(callback_request));
}

TEST_CASE_TEMPLATE("Callback already installed", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    auto callback_request = client.template install_callback<void>(
        "TestCallback", [] { std::cout << "Hello, callback!\n"; });

    REQUIRE(callback_request.func_name == "TestCallback");
    REQUIRE_THROWS_AS((std::ignore = client.template install_callback<void>(
                           "TestCallback", [] { std::cout << "Goodbye, callback!\n"; })),
        rpc_hpp::callback_install_error);

    client.uninstall_callback(std::move(callback_request));
}
#endif

TEST_CASE_TEMPLATE("Function not found", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto response = client.call_func("FUNC_WHICH_DOES_NOT_EXIST");

    REQUIRE(response.is_error());
    REQUIRE(response.get_error_type() == rpc_hpp::exception_type::func_not_found);
}

TEST_CASE_TEMPLATE("FunctionMismatch", TestType, RPC_TEST_TYPES)
{
#if defined(RPC_HPP_ENABLE_BITSERY)
    // TODO: Figure out why bitsery isn't reporting errors
    if constexpr (!std::is_same_v<TestType, rpc_hpp::adapters::bitsery_adapter>)
    {
#endif
        auto& client = GetClient<TestType>();

        rpc_hpp::rpc_object<TestType> obj =
            client.call_func("SimpleSum", 2, std::string{ "Hello, world" });

        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

        obj = client.call_func("SimpleSum", 1, 2);
        REQUIRE(obj.type() == rpc_hpp::rpc_type::func_result);
        REQUIRE_THROWS_AS(
            (std::ignore = obj.template get_result<std::string>()), rpc_hpp::function_mismatch);

        obj = client.call_func("SimpleSum", 2.4, 1.2);
        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

        obj = client.call_func("StdDev", -4.2, 125.325, 552.125, 55.123, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1, 111.222, 1234.56789);

        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

        obj = client.call_func("StdDev", -4, 125.325, 552.125, 55, 2599.6, 1245.125663, 9783.49,
            125.12, 553.3333333333, 2266.1);
        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);

        obj = client.call_func("StdDev", -4.2, 125.325);
        REQUIRE(obj.is_error());
        REQUIRE(obj.get_error_type() == rpc_hpp::exception_type::signature_mismatch);
#if defined(RPC_HPP_ENABLE_BITSERY)
    }
#endif
}

TEST_CASE_TEMPLATE("ThrowError", TestType, RPC_TEST_TYPES)
{
    auto& client = GetClient<TestType>();

    const auto bad_call = [&client]
    {
        client.call_func("ThrowError").template get_result<void>();
    };

    REQUIRE_THROWS_AS(bad_call(), rpc_hpp::remote_exec_error);
}

TEST_CASE_TEMPLATE("InvalidObject", TestType, RPC_TEST_TYPES)
{
#if defined(RPC_HPP_ENABLE_BITSERY)
    if (std::is_same_v<TestType, bitsery_adapter>)
    {
        // Ignoring bitsery
        // TODO: Verify bitsery data somehow
        return;
    }
#endif

    static constexpr size_t test_sz = 8UL;
    typename TestType::bytes_t bytes(test_sz, {});

    std::iota(bytes.begin(), bytes.end(), typename TestType::bytes_t::value_type{});

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

    const auto bad_call = [&client]
    {
        std::ignore = client.call_func("SimpleSum", 1, 2);
    };

    const auto kill_server = [&client]
    {
        std::ignore = client.call_func("KillServer");
    };

    WARN_NOTHROW(kill_server());
    REQUIRE_THROWS_AS(bad_call(), rpc_hpp::client_receive_error);
}
} //namespace rpc_hpp::tests
