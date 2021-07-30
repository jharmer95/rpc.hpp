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

#define CATCH_CONFIG_MAIN
#define RPC_HPP_CLIENT_IMPL

#include "rpc.client.hpp"
#include "test_structs.hpp"

#include <catch2/catch.hpp>

template<typename Serial>
TestClient<Serial>& GetClient();

template<typename Serial>
void TestType()
{
    auto& client = GetClient<Serial>();
    const auto result = client.template call_func<int>("SimpleSum", 1, 2).get_result();

    REQUIRE(result.has_value());
    REQUIRE(result.value() == 3);
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
        client.template call_func<int>("StrLen", std::string("hello, world")).get_result();

    REQUIRE(result.has_value());
    REQUIRE(result.value() == 12);
}

TEMPLATE_LIST_TEST_CASE("AddOneToEach", "", test_types_t)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto result =
        client.template call_func<std::vector<int>>("AddOneToEach", vec).get_result();

    REQUIRE(result.has_value());

    const auto& val = result.value();

    REQUIRE(val.size() == vec.size());

    for (size_t i = 0; i < val.size(); ++i)
    {
        REQUIRE(val[i] == vec[i] + 1);
    }
}

TEMPLATE_LIST_TEST_CASE("AddOneToEachRef", "", test_types_t)
{
    auto& client = GetClient<TestType>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = client.call_func("AddOneToEachRef", vec);

    const auto vec2 = pack.template get_arg<0>();

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

    const auto test = client.template call_func<uint64_t>("Fibonacci", 20).get_result();

    REQUIRE(test.has_value());
    REQUIRE(expected == test.value());
}

TEMPLATE_LIST_TEST_CASE("FibonacciRef", "", test_types_t)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<TestType>();

    uint64_t num = 20ULL;
    const auto test = client.call_func("FibonacciRef", num).template get_arg<0>();

    REQUIRE(expected == test);
}

TEMPLATE_LIST_TEST_CASE("StdDev", "", test_types_t)
{
    constexpr double expected = 3313.695594785;
    auto& client = GetClient<TestType>();

    const auto test = client
                          .template call_func<double>("StdDev", 55.65, 125.325, 552.125, 12.767,
                              2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                          .get_result();

    REQUIRE(test.has_value());
    REQUIRE_THAT(test.value(), Catch::Matchers::WithinRel(expected));
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

    const auto pack = client.call_func("SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

    n1 = pack.template get_arg<0>();
    n2 = pack.template get_arg<1>();
    n3 = pack.template get_arg<2>();
    n4 = pack.template get_arg<3>();
    n5 = pack.template get_arg<4>();
    n6 = pack.template get_arg<5>();
    n7 = pack.template get_arg<6>();
    n8 = pack.template get_arg<7>();
    n9 = pack.template get_arg<8>();
    n10 = pack.template get_arg<9>();

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
        client.template call_func<double>("AverageContainer<double>", vec).get_result();

    REQUIRE(test.has_value());
    REQUIRE_THAT(test.value(), Catch::Matchers::WithinAbs(expected, 0.001));
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

    const auto test = client.template call_func<std::string>("HashComplex", cx).get_result();

    REQUIRE(test.has_value());
    REQUIRE_THAT(expected, Catch::Matchers::Equals(test.value()));
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
    test = client.call_func("HashComplexRef", cx, test).template get_arg<1>();

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

TEMPLATE_LIST_TEST_CASE("Function not found", "", test_types_t)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]()
    {
        const auto pack = client.template call_func<int>("FUNC_WHICH_DOES_NOT_EXIST");

        if (!pack.get_result().has_value())
        {
            throw std::runtime_error(pack.get_err_mesg());
        }
    };

    REQUIRE_THROWS_WITH(exp(),
        Catch::Matchers::Equals(
            "RPC error: Called function: \"FUNC_WHICH_DOES_NOT_EXIST\" not found!"));
}

TEMPLATE_LIST_TEST_CASE("ThrowError", "", test_types_t)
{
    auto& client = GetClient<TestType>();

    const auto exp = [&client]()
    {
        const auto pack = client.template call_func<int>("ThrowError");

        if (!pack.get_result().has_value())
        {
            throw std::runtime_error(pack.get_err_mesg());
        }
    };

    REQUIRE_THROWS_WITH(exp(), "THIS IS A TEST ERROR!");
}

TEST_CASE("KillServer", "[!mayfail]")
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
