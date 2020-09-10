///@file rpc.benchmark.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Benchmarking source file for rpc.hpp
///@version 0.2.0.0
///@date 09-09-2020
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020, Jackson Harmer
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

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "rpc.hpp"

#if !defined(RPC_HPP_NJSON_ENABLED)
static_assert(false, "Test requires nlohmann/json adapter to be enabled!");
#endif

#include "rpc_adapters/rpc_njson.hpp"

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
#    include "rpc_adapters/rpc_rapidjson.hpp"
#endif

#include "rpc.client.hpp"
#include "test_structs.hpp"

template<typename Serial>
TestClient& GetClient();

template<>
TestClient& GetClient<njson>()
{
    static TestClient client("127.0.0.1", "5000");
    return client;
}

#if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
template<>
TestClient& GetClient<generic_serial_t>()
{
    static TestClient client("127.0.0.1", "5001");
    return client;
}
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
template<>
TestClient& GetClient<rpdjson_doc>()
{
    static TestClient client("127.0.0.1", "5002");
    return client;
}
#endif

TEST_CASE("Start server")
{
    // TODO: Spawn rpc_server process
}

TEST_CASE("By Value (simple)", "[value][simple]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 1;
    auto& njson_client = GetClient<njson>();
    auto& rpdjson_client = GetClient<rpdjson_doc>();

    BENCHMARK("rpc.hpp (indirect, njson)")
    {
        const auto pack = rpc::call<njson, uint64_t>(njson_client, "Fibonacci", 20);
        test = *pack.get_result();
    };

    REQUIRE(expected == test);

    test = 1;

    BENCHMARK("rpc.hpp (indirect, rapidjson)")
    {
        const auto pack = rpc::call<rpdjson_doc, uint64_t>(rpdjson_client, "Fibonacci", 20);
        test = *pack.get_result();
    };

    REQUIRE(expected == test);
}

TEST_CASE("By Value (complex)", "[value][complex]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;
    auto& njson_client = GetClient<njson>();
    auto& rpdjson_client = GetClient<rpdjson_doc>();

    BENCHMARK("rpc.hpp (indirect, njson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        const auto pack = rpc::call<njson, std::string>(njson_client, "HashComplex", cx);
        test = *pack.get_result();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

    test = "";

    BENCHMARK("rpc.hpp (indirect, rapidjson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        const auto pack = rpc::call<rpdjson_doc, std::string>(rpdjson_client, "HashComplex", cx);
        test = *pack.get_result();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

TEST_CASE("By Value (many)", "[value][many]")
{
    constexpr double expected = 3313.695594785;
    double test = 1.0;
    auto& njson_client = GetClient<njson>();
    auto& rpdjson_client = GetClient<rpdjson_doc>();

    BENCHMARK("rpc.hpp (indirect, njson)")
    {
        const auto pack = rpc::call<njson, double>(njson_client, "StdDev", 55.65, 125.325, 552.125,
            12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

        test = *pack.get_result();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

    test = 1.0;

    BENCHMARK("rpc.hpp (indirect, rapidjson)")
    {
        const auto pack = rpc::call<rpdjson_doc, double>(rpdjson_client, "StdDev", 55.65, 125.325, 552.125,
            12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);

        test = *pack.get_result();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
}

TEST_CASE("By Reference (simple)", "[ref][simple]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 1;
    auto& njson_client = GetClient<njson>();
    auto& rpdjson_client = GetClient<rpdjson_doc>();

    BENCHMARK("rpc.hpp (indirect, njson)")
    {
        const auto pack = rpc::call<njson>(njson_client, "FibonacciRef", test);
        test = pack.get_arg<uint64_t>(0);
    };

    REQUIRE(expected == test);

    test = 1;

    BENCHMARK("rpc.hpp (indirect, rapidjson)")
    {
        const auto pack = rpc::call<rpdjson_doc>(rpdjson_client, "FibonacciRef", test);
        test = pack.get_arg<uint64_t>(0);
    };

    REQUIRE(expected == test);
}

TEST_CASE("By Reference (complex)", "[ref][complex]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;
    auto& njson_client = GetClient<njson>();
    auto& rpdjson_client = GetClient<rpdjson_doc>();

    BENCHMARK("rpc.hpp (indirect, njson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        const auto pack = rpc::call<njson>(njson_client, "HashComplexRef", cx, test);
        test = pack.get_arg<std::string>(1);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

    test = "";

    BENCHMARK("rpc.hpp (indirect, rapidjson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        const auto pack = rpc::call<rpdjson_doc>(rpdjson_client, "HashComplexRef", cx, test);
        test = pack.get_arg<std::string>(1);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test)); 
}

TEST_CASE("KillServer")
{
    auto& client = GetClient<njson>();
    rpc::call<njson>(client, "KillServer");
}
