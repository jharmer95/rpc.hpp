///@file rpc.client.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of an RPC client class for testing
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

#pragma once

#include <asio.hpp>

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include <rpc_adapters/rpc_boost_json.hpp>

using rpc::adapters::bjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#    include <rpc_adapters/rpc_njson.hpp>

using rpc::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include <rpc_adapters/rpc_rapidjson.hpp>

using rpc::adapters::rapidjson_adapter;
#endif

using asio::ip::tcp;

template<typename Serial>
class TestClient final : public rpc::client_interface<Serial>
{
public:
    TestClient(const std::string_view host, const std::string_view port)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    std::string getIP() const { return m_socket.remote_endpoint().address().to_string(); }

private:
    void send(const typename Serial::bytes_t& mesg) override { asio::write(m_socket, asio::buffer(mesg)); }

    [[nodiscard("Data is lost after receive is called")]] typename Serial::bytes_t receive() override
    {
        const auto numBytes = m_socket.read_some(asio::buffer(m_buffer));
        return typename Serial::bytes_t{ m_buffer.data(), numBytes };
    }

    static constexpr size_t BUF_SZ = 16 * 1024;

    asio::io_context m_io{};
    tcp::resolver m_resolver{ m_io };
    tcp::socket m_socket{ m_io };
    std::array<char, BUF_SZ> m_buffer{};
};
