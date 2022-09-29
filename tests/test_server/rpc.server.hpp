///@file rpc.server.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Declarations and implementations of an RPC server for testing
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

#include "../test_structs.hpp"

#include <asio.hpp>
#include <rpc_server.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace test_server
{
inline std::atomic_bool RUNNING{ false };

template<typename Serial>
class TestServer final : public rpc_hpp::server_interface<Serial>
{
public:
    using bytes_t = typename rpc_hpp::server_interface<Serial>::bytes_t;
    using rpc_hpp::server_interface<Serial>::bind;
    using rpc_hpp::server_interface<Serial>::handle_bytes;

    TestServer() = delete;
    TestServer(asio::io_context& io_ctx, const uint16_t port)
        : rpc_hpp::server_interface<Serial>{},
          m_accept(io_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
    }

    bytes_t receive() override
    {
        RPC_HPP_PRECONDITION(m_socket.has_value());

        static constexpr unsigned BUFFER_SZ = 64 * 1024;
        static std::array<uint8_t, BUFFER_SZ> data{};

        asio::error_code error;
        const size_t len = m_socket.value().read_some(asio::buffer(data.data(), BUFFER_SZ), error);

        if (error == asio::error::eof)
        {
            return {};
        }

        // other error
        if (error)
        {
            throw asio::system_error{ error };
        }

        return { data.data(), data.data() + len };
    }

    void send(bytes_t&& bytes) override
    {
        RPC_HPP_PRECONDITION(m_socket.has_value());
        write(m_socket.value(), asio::buffer(std::move(bytes), bytes.size()));
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    std::string GetConnectionInfo()
    {
        std::stringstream ss;

        ss << "Server name: MyServer\n";
        ss << "Client name: " << this->template call_callback<std::string>("GetClientName") << '\n';

        return ss.str();
    }
#endif

    void Run()
    {
        while (RUNNING)
        {
            m_socket = m_accept.accept();

            try
            {
                while (RUNNING)
                {
                    auto recv_data = receive();

                    if (std::size(recv_data) == 0)
                    {
                        break;
                    }

                    handle_bytes(recv_data);
                    send(std::move(recv_data));
                }
            }
            catch (const std::exception& ex)
            {
                std::fprintf(stderr, "Exception in thread: %s\n", ex.what());
            }

            m_socket.reset();
        }
    }

private:
    asio::ip::tcp::acceptor m_accept;
    std::optional<asio::ip::tcp::socket> m_socket{};
};
} //namespace test_server
