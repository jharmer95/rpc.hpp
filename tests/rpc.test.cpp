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

using rpc::adapters::boost_json_adapter;
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
TestClient& GetClient<boost_json_adapter>()
{
    static TestClient client("127.0.0.1", "5002");
    return client;
}

TEST_CASE("BOOST_JSON")
{
    TestType<boost_json_adapter>();
}
#endif

using test_serial_t = njson_adapter;

#if defined(RPC_HPP_ENABLE_POINTERS)
TEST_CASE("PtrSum")
{
    auto& client = GetClient<test_serial_t>();

    int n = 12;
    const auto pack = rpc::call_func<test_serial_t>(client, "PtrSum", &n, -3);
    auto* ptr = pack.get_arg<int*, 0>();

    REQUIRE(ptr != nullptr);
    REQUIRE(*ptr == 9);
}

TEST_CASE("AddAllPtr")
{
    auto& client = GetClient<test_serial_t>();

    int myArr[] = { 2, 5, 7, 3 };
    const auto pack = rpc::call_func<test_serial_t, int>(client, "AddAllPtr", myArr, 4);

    REQUIRE(pack.get_result() == 17);
}

TEST_CASE("FibonacciPtr")
{
    auto& client = GetClient<test_serial_t>();

    uint64_t n = 20;
    const auto pack = rpc::call_func<test_serial_t>(client, "FibonacciPtr", &n);
    auto* ptr = pack.get_arg<uint64_t*, 0>();

    REQUIRE(ptr != nullptr);
    REQUIRE(*ptr == 10946ULL);
}

TEST_CASE("SquareRootPtr")
{
    auto& client = GetClient<test_serial_t>();

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

    const auto pack = rpc::call_func<test_serial_t>(
        client, "SquareRootPtr", &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10);

    n1 = *pack.get_arg<double*, 0>();
    n2 = *pack.get_arg<double*, 1>();
    n3 = *pack.get_arg<double*, 2>();
    n4 = *pack.get_arg<double*, 3>();
    n5 = *pack.get_arg<double*, 4>();
    n6 = *pack.get_arg<double*, 5>();
    n7 = *pack.get_arg<double*, 6>();
    n8 = *pack.get_arg<double*, 7>();
    n9 = *pack.get_arg<double*, 8>();
    n10 = *pack.get_arg<double*, 9>();
    const auto test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(313.2216436152, 0.001));
}

TEST_CASE("HashComplexPtr")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<test_serial_t>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    char hash[256]{};

    const std::string test(
        rpc::call_func<test_serial_t>(client, "HashComplexPtr", &cx, hash).get_arg<char*, 1>());

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

TEST_CASE("WriteMessagePtr")
{
    auto& client = GetClient<test_serial_t>();

    TestMessage msg[2];
    msg[0].flag1 = true;
    msg[0].flag2 = false;
    msg[0].id = 14;
    msg[0].data_sz = 22;

    for (int i = 0; i < msg[0].data_sz; ++i)
    {
        msg[0].data[i] = i * 2;
    }

    msg[1].flag1 = false;
    msg[1].flag2 = false;
    msg[1].id = 15;
    msg[1].data_sz = 12;

    for (int i = 0; i < msg[1].data_sz; ++i)
    {
        msg[1].data[i] = i * 3;
    }

    int numMsg = 2;

    const auto pack = rpc::call_func<test_serial_t, int>(client, "WriteMessagePtr", msg, &numMsg);
    numMsg = *pack.get_arg<int*, 1>();

    REQUIRE(numMsg == 2);
    REQUIRE(pack.get_result() == 0);
}

TEST_CASE("ReadMessagePtr")
{
    auto& client = GetClient<test_serial_t>();

    TestMessage msg[4];

    int numMsg = 2;
    const auto pack = rpc::call_func<test_serial_t, int>(client, "ReadMessagePtr", msg, &numMsg);
    const auto* ptr = pack.get_arg<TestMessage*, 0>();
    numMsg = *pack.get_arg<int*, 1>();

    REQUIRE(numMsg == 2);
    REQUIRE(pack.get_result() == 0);
    REQUIRE(ptr[0].id == 14);
    REQUIRE(ptr[1].id == 15);
}
#endif

TEST_CASE("StrLen")
{
    auto& client = GetClient<test_serial_t>();
    const auto result =
        rpc::call_func<test_serial_t, int>(client, "StrLen", std::string("hello, world"))
            .get_result();

    REQUIRE(result == 12);
}

TEST_CASE("AddOneToEach")
{
    auto& client = GetClient<test_serial_t>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto result =
        rpc::call_func<test_serial_t, std::vector<int>>(client, "AddOneToEach", vec).get_result();

    REQUIRE(result.size() == vec.size());

    for (size_t i = 0; i < result.size(); ++i)
    {
        REQUIRE(result[i] == vec[i] + 1);
    }
}

TEST_CASE("AddOneToEachRef")
{
    auto& client = GetClient<test_serial_t>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = rpc::call_func<test_serial_t>(client, "AddOneToEachRef", vec);

    const auto vec2 = pack.get_arg<0>();

    REQUIRE(vec2.size() == vec.size());

    for (size_t i = 0; i < vec2.size(); ++i)
    {
        REQUIRE(vec2[i] == vec[i] + 1);
    }
}

TEST_CASE("Fibonacci")
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<test_serial_t>();

    const auto test = rpc::call_func<test_serial_t, uint64_t>(client, "Fibonacci", 20).get_result();
    REQUIRE(expected == test);
}

TEST_CASE("FibonacciRef")
{
    constexpr uint64_t expected = 10946ULL;
    auto& client = GetClient<test_serial_t>();

    uint64_t num = 20ULL;
    const auto test = rpc::call_func<test_serial_t>(client, "FibonacciRef", num).get_arg<0>();

    REQUIRE(expected == test);
}

TEST_CASE("StdDev")
{
    constexpr double expected = 3313.695594785;
    auto& client = GetClient<test_serial_t>();

    const auto test = rpc::call_func<test_serial_t, double>(client, "StdDev", 55.65, 125.325,
        552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                          .get_result();

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
}

TEST_CASE("SquareRootRef")
{
    constexpr double expected = 313.2216436152;
    auto& client = GetClient<test_serial_t>();

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

    const auto pack = rpc::call_func<test_serial_t>(
        client, "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

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

TEST_CASE("AverageContainer<double>")
{
    constexpr double expected = 1731.8635996333;
    auto& client = GetClient<test_serial_t>();

    const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    const auto test =
        rpc::call_func<test_serial_t, double>(client, "AverageContainer<double>", vec).get_result();

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
}

TEST_CASE("HashComplex")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<test_serial_t>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    const auto test =
        rpc::call_func<test_serial_t, std::string>(client, "HashComplex", cx).get_result();

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

TEST_CASE("HashComplexRef")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    auto& client = GetClient<test_serial_t>();

    ComplexObject cx;
    cx.flag1 = false;
    cx.flag2 = true;
    cx.id = 24;
    cx.name = "Franklin D. Roosevelt";
    cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

    // initialize empty string to pass
    std::string test{};

    // re-assign string to arg<1>
    test = rpc::call_func<test_serial_t>(client, "HashComplexRef", cx, test).get_arg<1>();

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
}

// TEST_CASE("Function not found")
// {
//     auto& client = GetClient<test_serial_t>();

//     const auto exp = [&client]() {
//         rpc::call_func<test_serial_t, int>(client, "FUNC_WHICH_DOES_NOT_EXIST");
//     };

//     REQUIRE_THROWS_WITH(exp(),
//         Catch::Matchers::Equals(
//             "RPC error: Called function: \"FUNC_WHICH_DOES_NOT_EXIST\" not found!"));
// }

// TEST_CASE("ThrowError")
// {
//     auto& client = GetClient<test_serial_t>();

//     const auto exp = [&client]() { rpc::call_func<test_serial_t, int>(client, "ThrowError"); };

//     REQUIRE_THROWS_WITH(exp(), "THIS IS A TEST ERROR!");
// }

TEST_CASE("KillServer", "[!mayfail]")
{
    auto& client = GetClient<test_serial_t>();

    try
    {
        rpc::call_func<test_serial_t>(client, "KillServer");
    }
    catch (...)
    {
    }

    REQUIRE_THROWS(TestType<njson_adapter>());
}
