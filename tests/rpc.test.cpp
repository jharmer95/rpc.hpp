///@file rpc.test.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Unit test source file for rpc.hpp
///@version 0.1.0.0
///@date 06-05-2020
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

#include "rpc.hpp"

#if defined(RPC_HPP_NJSON_ENABLED) || defined(RPC_HPP_NCBOR_ENABLED)       \
    || defined(RPC_HPP_NBSON_ENABLED) || defined(RPC_HPP_NMSGPACK_ENABLED) \
    || defined(RPC_HPP_NUBJSON_ENABLED)
#    include "rpc_adapters/rpc_njson.hpp"
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
#    include "rpc_adapters/rpc_rapidjson.hpp"
#endif

#include "rpc.client.hpp"

#if defined(RPC_HPP_NJSON_ENABLED)
template<>
void rpc::client::send_to_server(const njson& serial_obj, TestClient& client)
{
    client.send(serial_adapter<njson>::to_string(serial_obj));
}

template<>
njson rpc::client::get_server_response(TestClient& client)
{
    return serial_adapter<njson>::from_string(client.receive());
}
#endif

#if defined(RPC_HPP_NCBOR_ENABLED)
template<>
void rpc::client::send_to_server(const ncbor& serial_obj, TestClient& client)
{
    client.send(serial_adapter<ncbor>::to_string(serial_obj));
}

template<>
ncbor rpc::client::get_server_response(TestClient& client)
{
    return serial_adapter<ncbor>::from_string(client.receive());
}
#endif

#if defined(RPC_HPP_NBSON_ENABLED)
template<>
void rpc::client::send_to_server(const nbson& serial_obj, TestClient& client)
{
    client.send(serial_adapter<nbson>::to_string(serial_obj));
}

template<>
nbson rpc::client::get_server_response(TestClient& client)
{
    return serial_adapter<nbson>::from_string(client.receive());
}
#endif

#if defined(RPC_HPP_NMSGPACK_ENABLED)
template<>
void rpc::client::send_to_server(const nmsgpack& serial_obj, TestClient& client)
{
    client.send(serial_adapter<nmsgpack>::to_string(serial_obj));
}

template<>
nmsgpack rpc::client::get_server_response(TestClient& client)
{
    return serial_adapter<nmsgpack>::from_string(client.receive());
}
#endif

#if defined(RPC_HPP_NUBJSON_ENABLED)
template<>
void rpc::client::send_to_server(const nubjson& serial_obj, TestClient& client)
{
    client.send(serial_adapter<nubjson>::to_string(serial_obj));
}

template<>
nubjson rpc::client::get_server_response(TestClient& client)
{
    return serial_adapter<nubjson>::from_string(client.receive());
}
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
template<>
void rpc::client::send_to_server(const rpdjson_doc& serial_obj, TestClient& client)
{
    client.send(serial_adapter<rpdjson_doc>::to_string(serial_obj));
}

template<>
rpdjson_doc rpc::client::get_server_response(TestClient& client)
{
    return serial_adapter<rpdjson_doc>::from_string(client.receive());
}
#endif

#if defined(RPC_HPP_NJSON_ENABLED)
TestClient& GetClient_NJSON()
{
    static TestClient client("127.0.0.1", "5000");
    return client;
}

TEST_CASE("SimpleSum (njson)")
{
    auto& c = GetClient_NJSON();
    auto pack = rpc::call<njson, TestClient, int>(c, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

TEST_CASE("StrLen (njson)")
{
    auto& c = GetClient_NJSON();
    auto pack = rpc::call<njson, TestClient, int>(c, "StrLen", std::string("hello, world"));
    REQUIRE(*pack.get_result() == 12);
}

TEST_CASE("AddOneToEach")
{
    auto& c = GetClient_NJSON();
    const std::vector<int> vec{ 2, 4, 6, 8 };
    const auto pack = rpc::call<njson, TestClient, std::vector<int>>(c, "AddOneToEach", vec);
    const auto retVec = *pack.get_result();
    REQUIRE(retVec.size() == vec.size());

    for (size_t i = 0; i < retVec.size(); ++i)
    {
        REQUIRE(retVec[i] == vec[i] + 1);
    }
}
#endif

#if defined(RPC_HPP_NCBOR_ENABLED)
TestClient& GetClient_NCBOR()
{
    static TestClient client("127.0.0.1", "5001");
    return client;
}

TEST_CASE("SimpleSum (ncbor)")
{
    auto& c = GetClient_NCBOR();
    auto pack = rpc::call<ncbor, TestClient, int>(c, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

TEST_CASE("StrLen (ncbor)")
{
    auto& c = GetClient_NCBOR();
    auto pack = rpc::call<ncbor, TestClient, int>(c, "StrLen", std::string("hello, world"));
    REQUIRE(*pack.get_result() == 12);
}
#endif

#if defined(RPC_HPP_NBSON_ENABLED)
TestClient& GetClient_NBSON()
{
    static TestClient client("127.0.0.1", "5002");
    return client;
}

TEST_CASE("SimpleSum (nbson)")
{
    auto& c = GetClient_NBSON();
    auto pack = rpc::call<nbson, TestClient, int>(c, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

TEST_CASE("StrLen (nbson)")
{
    auto& c = GetClient_NBSON();
    auto pack = rpc::call<nbson, TestClient, int>(c, "StrLen", std::string("hello, world"));
    REQUIRE(*pack.get_result() == 12);
}
#endif

#if defined(RPC_HPP_NMSGPACK_ENABLED)
TestClient& GetClient_NMSGPACK()
{
    static TestClient client("127.0.0.1", "5003");
    return client;
}

TEST_CASE("SimpleSum (nmsgpack)")
{
    auto& c = GetClient_NMSGPACK();
    auto pack = rpc::call<nmsgpack, TestClient, int>(c, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

TEST_CASE("StrLen (nmsgpack)")
{
    auto& c = GetClient_NMSGPACK();
    auto pack = rpc::call<nmsgpack, TestClient, int>(c, "StrLen", std::string("hello, world"));
    REQUIRE(*pack.get_result() == 12);
}
#endif

#if defined(RPC_HPP_NUBJSON_ENABLED)
TestClient& GetClient_NUBJSON()
{
    static TestClient client("127.0.0.1", "5004");
    return client;
}

TEST_CASE("SimpleSum (nubjson)")
{
    auto& c = GetClient_NUBJSON();
    auto pack = rpc::call<nubjson, TestClient, int>(c, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

TEST_CASE("StrLen (nubjson)")
{
    auto& c = GetClient_NUBJSON();
    auto pack = rpc::call<nubjson, TestClient, int>(c, "StrLen", std::string("hello, world"));
    REQUIRE(*pack.get_result() == 12);
}
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
TestClient& GetClient_RAPIDJSON()
{
    static TestClient client("127.0.0.1", "5005");
    return client;
}
TEST_CASE("SimpleSum (rapidjson)")
{
    auto& c = GetClient_RAPIDJSON();
    auto pack = rpc::call<rpdjson_doc, TestClient, int>(c, "SimpleSum", 1, 2);
    REQUIRE(*pack.get_result() == 3);
}

TEST_CASE("StrLen (rapidjson)")
{
    auto& c = GetClient_RAPIDJSON();
    auto pack = rpc::call<rpdjson_doc, TestClient, int>(c, "StrLen", std::string("hello, world"));
    REQUIRE(*pack.get_result() == 12);
}
#endif
