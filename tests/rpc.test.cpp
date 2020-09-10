///@file rpc.test.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Unit test source file for rpc.hpp
///@version 0.2.0
///@date 09-10-2020
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

#include <catch2/catch.hpp>

#if !defined(RPC_HPP_NJSON_ENABLED)
static_assert(false, "Test requires nlohmann/json adapter to be enabled!");
#endif

#include "rpc_adapters/rpc_njson.hpp"

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
#    include "rpc_adapters/rpc_rapidjson.hpp"
#endif

#include "rpc.client.hpp"

template<typename Serial>
TestClient& GetClient();

template<typename Serial>
void TestType()
{
    auto& client = GetClient<Serial>();
    auto pack = rpc::call<Serial, int>(client, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

template<>
TestClient& GetClient<njson>()
{
    static TestClient client("127.0.0.1", "5000");
    return client;
}

TEST_CASE("NJSON")
{
    TestType<njson>();
}

#if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
template<>
TestClient& GetClient<generic_serial_t>()
{
    static TestClient client("127.0.0.1", "5001");
    return client;
}

TEST_CASE("GENERIC_SERIAL_T")
{
    TestType<generic_serial_t>();
}
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
template<>
TestClient& GetClient<rpdjson_doc>()
{
    static TestClient client("127.0.0.1", "5002");
    return client;
}

TEST_CASE("RAPIDJSON")
{
    TestType<rpdjson_doc>();
}
#endif

using test_serial_t = njson;

TEST_CASE("StrLen")
{
    auto& client = GetClient<test_serial_t>();
    auto pack = rpc::call<test_serial_t, int>(client, "StrLen", std::string("hello, world"));

    REQUIRE(*pack.get_result() == 12);
}

TEST_CASE("AddOneToEach")
{
    auto& client = GetClient<test_serial_t>();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = rpc::call<test_serial_t, std::vector<int>>(client, "AddOneToEach", vec);

    const auto retVec = *pack.get_result();
    REQUIRE(retVec.size() == vec.size());

    for (size_t i = 0; i < retVec.size(); ++i)
    {
        REQUIRE(retVec[i] == vec[i] + 1);
    }
}

TEST_CASE("AddOneToEachRef")
{
    auto& client = GetClient<test_serial_t>();
    std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = rpc::call<test_serial_t>(client, "AddOneToEachRef", vec);

    const auto retVec = pack.get_arg<std::vector<int>>(0);
    REQUIRE(retVec.size() == vec.size());

    for (size_t i = 0; i < retVec.size(); ++i)
    {
        REQUIRE(retVec[i] == vec[i] + 1);
    }
}

TEST_CASE("KillServer")
{
    auto& client = GetClient<test_serial_t>();
    try
    {
        rpc::call<test_serial_t>(client, "KillServer");
    }
    catch (...)
    {
    }

    REQUIRE_THROWS(TestType<njson>());
}
