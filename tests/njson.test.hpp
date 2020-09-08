#pragma once

#include <catch2/catch.hpp>
#include "rpc.hpp"
#include "rpc_adapters/rpc_njson.hpp"
#include "rpc.client.hpp"

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

TEST_CASE("AddOneToEach (ncbor)")
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
