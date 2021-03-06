///@file rpc.benchmark.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Benchmarking source file for rpc.hpp
///@version 0.4.1
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
///FOR ANY DIRECT, asio::tcp, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#define CATCH_CONFIG_ENABLE_BENCHMARKING
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

template<>
TestClient& GetClient<njson_adapter>()
{
    static TestClient client("127.0.0.1", "5000");
    return client;
}

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
template<>
TestClient& GetClient<rapidjson_adapter>()
{
    static TestClient client("127.0.0.1", "5001");
    return client;
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
template<>
TestClient& GetClient<bjson_adapter>()
{
    static TestClient client("127.0.0.1", "5002");
    return client;
}
#endif

TEST_CASE("By Value (simple)", "[value][simple][cached]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test = rpc::call_func<njson_adapter, uint64_t>(GetClient<njson_adapter>(), "Fibonacci", 20)
                   .get_result();
    };

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = rpc::call_func<rapidjson_adapter, uint64_t>(
            GetClient<rapidjson_adapter>(), "Fibonacci", 20)
                   .get_result();
    };

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = rpc::call_func<bjson_adapter, uint64_t>(GetClient<bjson_adapter>(), "Fibonacci", 20)
                   .get_result();
    };

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Value (complex)", "[value][complex][cached]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = rpc::call_func<njson_adapter, std::string>(
            GetClient<njson_adapter>(), "HashComplex", cx)
                   .get_result();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = rpc::call_func<rapidjson_adapter, std::string>(
            GetClient<rapidjson_adapter>(), "HashComplex", cx)
                   .get_result();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = rpc::call_func<bjson_adapter, std::string>(
            GetClient<bjson_adapter>(), "HashComplex", cx)
                   .get_result();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif
}

TEST_CASE("By Value (many)", "[value][many][cached]")
{
    constexpr double expected = 3313.695594785;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test = rpc::call_func<njson_adapter, double>(GetClient<njson_adapter>(), "StdDev", 55.65,
            125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                   .get_result();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = rpc::call_func<rapidjson_adapter, double>(GetClient<rapidjson_adapter>(), "StdDev",
            55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333,
            2266.1)
                   .get_result();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = rpc::call_func<bjson_adapter, double>(GetClient<bjson_adapter>(), "StdDev", 55.65,
            125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                   .get_result();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif
}

TEST_CASE("By Reference (simple)", "[ref][simple]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        uint64_t num = 20;
        test = rpc::call_func<njson_adapter>(GetClient<njson_adapter>(), "FibonacciRef", num)
                   .template get_arg<0>();
    };

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        uint64_t num = 20;
        test =
            rpc::call_func<rapidjson_adapter>(GetClient<rapidjson_adapter>(), "FibonacciRef", num)
                .template get_arg<0>();
    };
#endif

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        uint64_t num = 20;
        test = rpc::call_func<bjson_adapter>(GetClient<bjson_adapter>(), "FibonacciRef", num)
                   .template get_arg<0>();
    };

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Reference (complex)", "[ref][complex]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = rpc::call_func<njson_adapter>(GetClient<njson_adapter>(), "HashComplexRef", cx, test)
                   .template get_arg<1>();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = rpc::call_func<rapidjson_adapter>(
            GetClient<rapidjson_adapter>(), "HashComplexRef", cx, test)
                   .template get_arg<1>();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = rpc::call_func<bjson_adapter>(GetClient<bjson_adapter>(), "HashComplexRef", cx, test)
                   .template get_arg<1>();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif
}

TEST_CASE("By Reference (many)", "[ref][many]")
{
    constexpr double expected = 313.2216436152;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
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

        const auto pack = rpc::call_func<njson_adapter>(
            GetClient<njson_adapter>(), "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

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
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
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

        const auto pack = rpc::call_func<rapidjson_adapter>(GetClient<rapidjson_adapter>(),
            "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

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
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
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

        const auto pack = rpc::call_func<bjson_adapter>(
            GetClient<bjson_adapter>(), "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

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
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#endif
}

TEST_CASE("With Container", "[container][cached]")
{
    constexpr double expected = 1731.8635996333;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = rpc::call_func<njson_adapter, double>(
            GetClient<njson_adapter>(), "AverageContainer<double>", vec)
                   .get_result();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = rpc::call_func<rapidjson_adapter, double>(
            GetClient<rapidjson_adapter>(), "AverageContainer<double>", vec)
                   .get_result();
    };
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = rpc::call_func<bjson_adapter, double>(
            GetClient<bjson_adapter>(), "AverageContainer<double>", vec)
                   .get_result();
    };
#endif
}

TEST_CASE("Sequential", "[sequential][cached]")
{
    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        auto vec = rpc::call_func<njson_adapter, std::vector<uint64_t>>(
            GetClient<njson_adapter>(), "RandInt", 5, 30, 1000)
                       .get_result();

        for (auto& val : vec)
        {
            val = rpc::call_func<njson_adapter, uint64_t>(
                GetClient<njson_adapter>(), "Fibonacci", val)
                      .get_result();
        }

        return rpc::call_func<njson_adapter, double>(
            GetClient<njson_adapter>(), "AverageContainer<uint64_t>", vec)
            .get_result();
    };

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        auto vec = rpc::call_func<rapidjson_adapter, std::vector<uint64_t>>(
            GetClient<rapidjson_adapter>(), "RandInt", 5, 30, 1000)
                       .get_result();

        for (auto& val : vec)
        {
            val = rpc::call_func<rapidjson_adapter, uint64_t>(
                GetClient<rapidjson_adapter>(), "Fibonacci", val)
                      .get_result();
        }

        return rpc::call_func<rapidjson_adapter, double>(
            GetClient<rapidjson_adapter>(), "AverageContainer<uint64_t>", vec)
            .get_result();
    };
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    BENCHMARK("rpc.hpp (asio::tcp, bjson)")
    {
        auto vec = rpc::call_func<bjson_adapter, std::vector<uint64_t>>(
            GetClient<bjson_adapter>(), "RandInt", 5, 30, 1000)
                       .get_result();

        for (auto& val : vec)
        {
            val = rpc::call_func<bjson_adapter, uint64_t>(
                GetClient<bjson_adapter>(), "Fibonacci", val)
                      .get_result();
        }

        return rpc::call_func<bjson_adapter, double>(
            GetClient<bjson_adapter>(), "AverageContainer<uint64_t>", vec)
            .get_result();
    };
#endif
}

#if defined(RPC_HPP_ENABLE_POINTERS)
TEST_CASE("By Pointer (simple)", "[pointer][simple]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        uint64_t num = 20;
        test = *rpc::call_func<njson_adapter>(GetClient<njson_adapter>(), "FibonacciPtr", &num)
                    .template get_arg<0>();
    };

    REQUIRE(expected == test);

#    if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        uint64_t num = 20;
        test =
            *rpc::call_func<rapidjson_adapter>(GetClient<rapidjson_adapter>(), "FibonacciPtr", &num)
                 .template get_arg<0>();
    };

    REQUIRE(expected == test);
#    endif

#    if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        uint64_t num = 20;
        test = *rpc::call_func<bjson_adapter>(GetClient<bjson_adapter>(), "FibonacciPtr", &num)
                    .template get_arg<0>();
    };

    REQUIRE(expected == test);
#    endif
}

TEST_CASE("By Pointer (complex)", "[pointer][complex]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        char hash[256]{};

        test = std::string(
            rpc::call_func<njson_adapter>(GetClient<njson_adapter>(), "HashComplexPtr", &cx, hash)
                .template get_arg<1>());
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

#    if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        char hash[256]{};

        test = std::string(rpc::call_func<rapidjson_adapter>(
            GetClient<rapidjson_adapter>(), "HashComplexPtr", &cx, hash)
                               .template get_arg<1>());
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#    endif

#    if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        char hash[256]{};

        test = std::string(rpc::call_func<bjson_adapter>(
                               GetClient<bjson_adapter>(), "HashComplexPtr", &cx, hash)
                               .get_arg
                       < char*,
                   1)
            > ();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#    endif
}

TEST_CASE("By Pointer (many)", "[pointer][many]")
{
    constexpr double expected = 313.2216436152;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
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

        const auto pack = rpc::call_func<njson_adapter>(GetClient<njson_adapter>(), "SquareRootPtr",
            &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10);

        n1 = *pack.template get_arg<0>();
        n2 = *pack.template get_arg<1>();
        n3 = *pack.template get_arg<2>();
        n4 = *pack.template get_arg<3>();
        n5 = *pack.template get_arg<4>();
        n6 = *pack.template get_arg<5>();
        n7 = *pack.template get_arg<6>();
        n8 = *pack.template get_arg<7>();
        n9 = *pack.template get_arg<8>();
        n10 = *pack.template get_arg<9>();
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#    if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
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

        const auto pack = rpc::call_func<rapidjson_adapter>(GetClient<rapidjson_adapter>(),
            "SquareRootPtr", &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10);

        n1 = *pack.template get_arg<0>();
        n2 = *pack.template get_arg<1>();
        n3 = *pack.template get_arg<2>();
        n4 = *pack.template get_arg<3>();
        n5 = *pack.template get_arg<4>();
        n6 = *pack.template get_arg<5>();
        n7 = *pack.template get_arg<6>();
        n8 = *pack.template get_arg<7>();
        n9 = *pack.template get_arg<8>();
        n10 = *pack.template get_arg<9>();
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#    endif

#    if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
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

        const auto pack = rpc::call_func<bjson_adapter>(GetClient<bjson_adapter>(), "SquareRootPtr",
            &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10);

        n1 = *pack.template get_arg<0>();
        n2 = *pack.template get_arg<1>();
        n3 = *pack.template get_arg<2>();
        n4 = *pack.template get_arg<3>();
        n5 = *pack.template get_arg<4>();
        n6 = *pack.template get_arg<5>();
        n7 = *pack.template get_arg<6>();
        n8 = *pack.template get_arg<7>();
        n9 = *pack.template get_arg<8>();
        n10 = *pack.template get_arg<9>();
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#    endif
}
#endif

TEST_CASE("KillServer",
    "[!mayfail][value][simple][cached][ref][complex][sequential][pointer][many][container]")
{
    auto& client = GetClient<njson_adapter>();

    try
    {
        rpc::call_func<njson_adapter>(client, "KillServer");
    }
    catch (...)
    {
    }
}
