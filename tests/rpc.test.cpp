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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_USE_STD_HEADERS
#include <doctest/doctest.h>

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
void T()
{
    auto& client = GetClient<Serial>();
    const auto result =
        rpc::client::call_func<njson_adapter, int>(client, "SimpleSum", 1, 2).get_result();

    REQUIRE_EQ(result, 3);
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
    T<njson_adapter>();
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
    T<rapidjson_adapter>();
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
    T<bjson_adapter>();
}
#endif

TEST_CASE_TEMPLATE_DEFINE("StrLen", T, StrLen)
{
    auto& client = GetClient<T>();
    const auto result =
        rpc::call_func<T, int>(client, "StrLen", std::string("hello, world")).get_result();

    REQUIRE_EQ(result, 12);
}

TEST_CASE_TEMPLATE_DEFINE("AddOneToEach", T, AddOneToEach)
{
    auto& client = GetClient<T>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto result =
        rpc::call_func<T, std::vector<int>>(client, "AddOneToEach", vec).get_result();

    REQUIRE_EQ(result.size(), vec.size());

    for (size_t i = 0; i < result.size(); ++i)
    {
        REQUIRE_EQ(result[i], vec[i] + 1);
    }
}

TEST_CASE_TEMPLATE_DEFINE("AddOneToEachRef", T, AddOneToEachRef)
{
    auto& client = GetClient<T>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = rpc::call_func<T>(client, "AddOneToEachRef", vec);

    const auto vec2 = pack.template get_arg<0>();

    REQUIRE_EQ(vec2.size(), vec.size());

    for (size_t i = 0; i < vec2.size(); ++i)
    {
        REQUIRE_EQ(vec2[i], vec[i] + 1);
    }
}

TEST_CASE_TEMPLATE_DEFINE("Fibonacci", T, Fibonacci)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<T>();

    const auto test = rpc::call_func<T, uint64_t>(client, "Fibonacci", 20).get_result();
    REQUIRE_EQ(expected, test);
}

TEST_CASE_TEMPLATE_DEFINE("FibonacciRef", T, FibonacciRef)
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<T>();

    uint64_t num = 20ULL;
    const auto test = rpc::call_func<T>(client, "FibonacciRef", num).template get_arg<0>();

    REQUIRE_EQ(expected, test);
}

TEST_CASE_TEMPLATE_DEFINE("StdDev", T, StdDev)
{
    constexpr double expected = 3313.695594785;
    auto& client = GetClient<T>();

    const auto test = rpc::call_func<T, double>(client, "StdDev", 55.65, 125.325, 552.125, 12.767,
        2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                          .get_result();

    REQUIRE_EQ(test, doctest::Approx(expected));
}

TEST_CASE_TEMPLATE_DEFINE("SquareRootRef", T, SquareRootRef)
{
    constexpr double expected = 313.2216436152;
    auto& client = GetClient<T>();

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
        rpc::call_func<T>(client, "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

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

    REQUIRE_EQ(test, doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE_DEFINE("AverageContainer<double>", T, AverageContainer_double_)
{
    constexpr double expected = 1731.8635996333;
    auto& client = GetClient<T>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto test =
        rpc::call_func<T, double>(client, "AverageContainer<double>", vec).get_result();

    REQUIRE_EQ(test, doctest::Approx(expected).epsilon(0.001));
}

TEST_CASE_TEMPLATE_DEFINE("HashComplex", T, HashComplex)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<T>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    const auto test = rpc::call_func<T, std::string>(client, "HashComplex", cx).get_result();

    REQUIRE_EQ(expected, test);
}

TEST_CASE_TEMPLATE_DEFINE("HashComplexRef", T, HashComplexRef)
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<T>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    // initialize empty string to pass
    std::string test{};

    // re-assign string to arg<1>
    test = rpc::call_func<T>(client, "HashComplexRef", cx, test).template get_arg<1>();

    REQUIRE_EQ(expected, test);
}

TEST_CASE_TEMPLATE_DEFINE("Function not found", T, Func_Not_Found)
{
    auto& client = GetClient<T>();

    const auto exp = [&client]() {
        [[maybe_unused]] auto _unused =
            rpc::call_func<T, int>(client, "FUNC_WHICH_DOES_NOT_EXIST").get_result();
    };

    REQUIRE_THROWS_WITH(
        exp(), "RPC error: Called function: \"FUNC_WHICH_DOES_NOT_EXIST\" not found!");
}

TEST_CASE_TEMPLATE_DEFINE("ThrowError", T, ThrowError)
{
    auto& client = GetClient<T>();

    const auto exp = [&client]() {
        [[maybe_unused]] auto _unused = rpc::call_func<T, int>(client, "ThrowError").get_result();
    };

    REQUIRE_THROWS_WITH(exp(), "THIS IS A TEST ERROR!");
}

#if defined(RPC_HPP_ENABLE_NJSON)
TEST_CASE_TEMPLATE_INVOKE(StrLen, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AddOneToEach, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AddOneToEachRef, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(Fibonacci, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(FibonacciRef, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(StdDev, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(SquareRootRef, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AverageContainer_double_, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(HashComplex, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(HashComplexRef, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(Func_Not_Found, njson_adapter);
TEST_CASE_TEMPLATE_INVOKE(ThrowError, njson_adapter);
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
TEST_CASE_TEMPLATE_INVOKE(StrLen, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AddOneToEach, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AddOneToEachRef, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(Fibonacci, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(FibonacciRef, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(StdDev, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(SquareRootRef, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AverageContainer_double_, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(HashComplex, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(HashComplexRef, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(Func_Not_Found, rapidjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(ThrowError, rapidjson_adapter);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
TEST_CASE_TEMPLATE_INVOKE(StrLen, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AddOneToEach, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AddOneToEachRef, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(Fibonacci, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(FibonacciRef, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(StdDev, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(SquareRootRef, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(AverageContainer_double_, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(HashComplex, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(HashComplexRef, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(Func_Not_Found, bjson_adapter);
TEST_CASE_TEMPLATE_INVOKE(ThrowError, bjson_adapter);
#endif

TEST_CASE("KillServer" * doctest::no_breaks(true) * doctest::may_fail(true))
{
    auto& client = GetClient<njson_adapter>();

    try
    {
        rpc::call_func<njson_adapter>(client, "KillServer");
    }
    catch (...)
    {
    }

    REQUIRE_THROWS(T<njson_adapter>());
}
