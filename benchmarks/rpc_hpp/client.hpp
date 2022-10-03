///@file rpc.client.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of an RPC client class for testing
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

#pragma once

#include <asio.hpp>
#include <rpc_client.hpp>

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

#include <cstdint>
#include <string>
#include <utility>

namespace rpc_hpp::tests
{
template<typename Serial>
class TestClient final : public rpc_hpp::client_interface<Serial>
{
public:
    using bytes_t = typename Serial::bytes_t;

    TestClient(const std::string_view host, const std::string_view port)
        : m_socket(m_io), m_resolver(m_io)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    [[nodiscard]] std::string getIP() const
    {
        return m_socket.remote_endpoint().address().to_string();
    }

    void send(bytes_t&& mesg) override
    {
        asio::write(m_socket, asio::buffer(std::move(mesg), mesg.size()));
    }

    [[nodiscard]] bytes_t receive() override
    {
        const auto bytes_received = m_socket.read_some(asio::buffer(m_buffer, m_buffer.size()));
        return bytes_t{ m_buffer.begin(), m_buffer.begin() + bytes_received };
    }

private:
    static constexpr size_t buffer_sz{ 64UL * 1024UL };

    asio::io_context m_io{};
    asio::ip::tcp::socket m_socket;
    asio::ip::tcp::resolver m_resolver;
    std::array<uint8_t, buffer_sz> m_buffer{};
};

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
} //namespace rpc_hpp::tests
