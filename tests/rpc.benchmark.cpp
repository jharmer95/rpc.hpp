///@file rpc.benchmark.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Benchmark comparison for rpc.hpp
///@version 0.1.0.0
///@date 01-17-2020
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
///FOR ANY DIRECT, INDIRECT, INCIDENTA, SPECIA, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "bench_funcs.hpp"

TEST_CASE("By Value (simple)", "[value][simple]")
{
    uint64_t expected = 0;
    uint64_t test = 1;

    BENCHMARK("Direct Call") { expected = Fibonacci(20); };

    BENCHMARK("rpc.hpp indirect")
    {
        njson::json send_j;
        send_j["args"] = njson::json::array({ 20 });
        send_j["function"] = "Fibonacci";

        test = njson::json::parse(rpc::run<njson::json>(send_j))["result"];
    };

    REQUIRE(expected == test);

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Value (complex)", "[value][complex]")
{
    std::string expected;
    std::string test;

    BENCHMARK("Direct Call")
    {
        Complex myC;
        myC.flag1 = false;
        myC.flag2 = true;
        myC.id = 24;
        myC.name = "Franklin D. Roosevelt";
        myC.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };
        expected = HashComplex(myC);
    };

    BENCHMARK("rpc.hpp indirect")
    {
        Complex myC;
        myC.flag1 = false;
        myC.flag2 = true;
        myC.id = 24;
        myC.name = "Franklin D. Roosevelt";
        myC.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        njson::json send_j;
        send_j["args"] = njson::json::array( { rpc::serialize<njson::json, Complex>(myC) } );
        send_j["function"] = "HashComplex";

        test = njson::json::parse(rpc::run<njson::json>(send_j))["result"].get<std::string>();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Value (many)", "[value][many]")
{
    double expected = 0.0;
    double test = 1.0;

    BENCHMARK("Direct Call")
    {
        expected = StdDev(55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
    };

    BENCHMARK("rpc.hpp indirect")
    {
        njson::json send_j;
        send_j["args"] = njson::json::array({ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1 });
        send_j["function"] = "StdDev";

        test = njson::json::parse(rpc::run<njson::json>(send_j))["result"];
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Pointer (simple)", "[pointer][simple]")
{
    uint64_t expected = 0;
    uint64_t test = 1;

    BENCHMARK("Direct Call")
    {
        uint64_t n = 20U;
        FibonacciPtr(&n);
        expected = n;
    };

    BENCHMARK("rpc.hpp indirect")
    {
        uint64_t n = 20U;
        njson::json send_j;
        send_j["args"] = njson::json::array({ n });
        send_j["function"] = "FibonacciPtr";

        n = njson::json::parse(rpc::run<njson::json>(send_j))["args"][0];
        test = n;
    };

    REQUIRE(expected == test);

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Pointer (complex)", "[pointer][complex]")
{
    std::string expected;
    std::string test;

    BENCHMARK("Direct Call")
    {
        Complex myC;
        myC.flag1 = false;
        myC.flag2 = true;
        myC.id = 24;
        myC.name = "Franklin D. Roosevelt";
        myC.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };
        char hash[255]{};
        HashComplexPtr(&myC, hash);
        expected = std::string(hash);
    };

    BENCHMARK("rpc.hpp indirect")
    {
        Complex myC;
        myC.flag1 = false;
        myC.flag2 = true;
        myC.id = 24;
        myC.name = "Franklin D. Roosevelt";
        myC.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };
        char hash[255]{};

        njson::json send_j;
        send_j["args"] = njson::json::array({ rpc::serialize<njson::json, Complex>(myC), hash });
        send_j["function"] = "HashComplexPtr";

        const auto retMsg = rpc::run<njson::json>(send_j);
        const auto argList = njson::json::parse(retMsg)["args"];

        myC = rpc::deserialize<njson::json, Complex>(argList[0]);
        const auto str = argList[1].get<std::string>();
        test = str;
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Pointer (many)", "[pointer][many]")
{
    double expected = 0.0;
    double test = 1.0;

    BENCHMARK("Direct Call")
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
        SquareRootPtr(&n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10);
        expected = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    BENCHMARK("rpc.hpp indirect")
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

        njson::json send_j;
        send_j["args"] = njson::json::array({ n1, n2, n3, n4, n5, n6, n7, n8, n9, n10 });
        send_j["function"] = "SquareRootPtr";

        const auto argList = njson::json::parse(rpc::run<njson::json>(send_j))["args"];

        n1 = argList[0];
        n2 = argList[1];
        n3 = argList[2];
        n4 = argList[3];
        n5 = argList[4];
        n6 = argList[5];
        n7 = argList[6];
        n8 = argList[7];
        n9 = argList[8];
        n10 = argList[9];
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Reference (simple)", "[ref][simple]")
{
    uint64_t expected = 0;
    uint64_t test = 1;

    BENCHMARK("Direct Call")
    {
        uint64_t n = 20;
        FibonacciRef(n);
        expected = n;
    };

    BENCHMARK("rpc.hpp indirect")
    {
        uint64_t n = 20;

        njson::json send_j;
        send_j["args"] = njson::json::array({ n });
        send_j["function"] = "FibonacciRef";

        n = njson::json::parse(rpc::run<njson::json>(send_j))["args"][0];
        test = n;
    };

    REQUIRE(expected == test);

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Reference (complex)", "[ref][complex]")
{
    std::string expected;
    std::string test;

    BENCHMARK("Direct Call")
    {
        Complex myC;
        myC.flag1 = false;
        myC.flag2 = true;
        myC.id = 24;
        myC.name = "Franklin D. Roosevelt";
        myC.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };
        std::string hash;
        HashComplexRef(myC, hash);
        expected = hash;
    };

    BENCHMARK("rpc.hpp indirect")
    {
        Complex myC;
        myC.flag1 = false;
        myC.flag2 = true;
        myC.id = 24;
        myC.name = "Franklin D. Roosevelt";
        myC.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };
        std::string hash;

        njson::json send_j;
        send_j["args"] = njson::json::array({ rpc::serialize<njson::json, Complex>(myC), hash });
        send_j["function"] = "HashComplexRef";

        const auto recv_j = njson::json::parse(rpc::run<njson::json>(send_j));

        myC = rpc::deserialize<njson::json, Complex>(recv_j["args"][0]);
        hash = recv_j["args"][1].get<std::string>();
        test = hash;
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("By Reference (many)", "[ref][many]")
{
    double expected = 0.0;
    double test = 1.0;

    BENCHMARK("Direct Call")
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
        SquareRootRef(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);
        expected = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    BENCHMARK("rpc.hpp indirect")
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

        njson::json send_j;
        send_j["args"] = njson::json::array({ n1, n2, n3, n4, n5, n6, n7, n8, n9, n10 });
        send_j["function"] = "SquareRootRef";

        const auto argList = njson::json::parse(rpc::run<njson::json>(send_j))["args"];

        n1 = argList[0];
        n2 = argList[1];
        n3 = argList[2];
        n4 = argList[3];
        n5 = argList[4];
        n6 = argList[5];
        n7 = argList[6];
        n8 = argList[7];
        n9 = argList[8];
        n10 = argList[9];
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("With Container", "[container]")
{
    double expected = 0.0;
    double test = 1.0;

    BENCHMARK("Direct Call")
    {
        const std::vector<double> vec {55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1 };
        expected = AverageContainer(vec);
    };

    BENCHMARK("rpc.hpp indirect")
    {
        std::vector<double> vec {55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1 };

        njson::json send_j;
        send_j["args"] = njson::json::array({ vec });
        send_j["function"] = "AverageContainer<double>";

        test = njson::json::parse(rpc::run<njson::json>(send_j))["result"];
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}

TEST_CASE("Sequential", "[sequential]")
{
    BENCHMARK("Direct Call")
    {
        auto vec = RandInt(5, 30);

        for (auto& val : vec)
        {
            val = Fibonacci(val);
        }

        return AverageContainer(vec);
    };

    BENCHMARK("rpc.hpp indirect")
    {
        njson::json send_j;
        send_j["args"] = njson::json::array({ 5, 30, 1000 });
        send_j["function"] = "RandInt";

        auto vec = njson::json::parse(rpc::run<njson::json>(send_j))["result"].get<std::vector<uint64_t>>();

        for (auto& val : vec)
        {
            send_j["args"] = njson::json::array({ val });
            send_j["function"] = "Fibonacci";
            val = njson::json::parse(rpc::run<njson::json>(send_j))["result"];
        }

        send_j["args"] = njson::json::array({ vec });
        send_j["function"] = "AverageContainer<uint64_t>";

        return njson::json::parse(rpc::run<njson::json>(send_j))["result"];
    };

    //BENCHMARK("rpc.hpp socket IPC") { return XXX; };
    //BENCHMARK("rpclib socket IPC") { return XXX; };
    //BENCHMARK("gRPC socket IPC") { return XXX; };

    //BENCHMARK("rpc.hpp socket RPC") { return XXX; };
    //BENCHMARK("rpclib socket RPC") { return XXX; };
    //BENCHMARK("gRPC socket RPC") { return XXX; };
}
