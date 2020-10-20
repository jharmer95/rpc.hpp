///@file rpc.client.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Example implementation of an RPC client class
///@version 0.2.1
///@date 10-08-2020
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

#pragma once

#include "rpc.hpp"

#include <asio.hpp>

using asio::ip::tcp;

class TestClient final : public rpc::client::client_base
{
public:
    TestClient(const std::string_view host, const std::string_view port)
        : m_socket(m_io), m_resolver(m_io)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    void send(const std::string& mesg) override
    {
        write(m_socket, asio::buffer(mesg, mesg.size()));
    }

    [[nodiscard]] std::string receive() override
    {
        const auto numBytes = m_socket.read_some(asio::buffer(m_buffer, 64U * 1024UL));
        return std::string(m_buffer, m_buffer + numBytes);
    }

    [[nodiscard]] std::string getIP() const
    {
        return m_socket.remote_endpoint().address().to_string();
    }

private:
    asio::io_context m_io{};
    tcp::socket m_socket;
    tcp::resolver m_resolver;
    char m_buffer[64U * 1024UL]{};
};
