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

#include "../sync_queue.hpp"
#include "../test_structs.hpp"

#include <rpc_server.hpp>

#if defined(RPC_HPP_ENABLE_NJSON)
#  include <rpc_adapters/rpc_njson.hpp>
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  include <rpc_adapters/rpc_rapidjson.hpp>
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  include <rpc_adapters/rpc_boost_json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <rpc_adapters/rpc_bitsery.hpp>
#endif

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

namespace rpc_hpp::tests
{
template<typename Serial>
class TestServer final : public server_interface<Serial>
{
public:
    using bytes_t = typename server_interface<Serial>::bytes_t;
    using server_interface<Serial>::bind;
    using server_interface<Serial>::handle_bytes;

    [[nodiscard]] bytes_t receive() override { return m_p_message_queue->pop(); }

    void send(bytes_t&& bytes) override
    {
        if (!m_p_client_queue || m_p_client_queue.expired())
        {
            throw server_receive_error{ "No clients have been attached to the server" };
        }

        m_p_client_queue.lock()->push(std::move(bytes));
    }

    [[nodiscard]] std::weak_ptr<SyncQueue<bytes_t>> attach_client(
        std::weak_ptr<SyncQueue<bytes_t>> p_client_queue)
    {
        assert(!m_p_client_queue && "Only one client can be attached (for now)");
        m_p_client_queue = p_client_queue;
        return m_p_message_queue;
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
        m_running = true;

        while (m_running)
        {
            try
            {
                while (m_running)
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
        }
    }

    void Stop()
    {
        m_running = false;
    }

private:
    std::atomic<bool> m_running{ false };
    std::shared_ptr<SyncQueue<bytes_t>> m_p_message_queue{ std::make_shared<SyncQueue<bytes_t>>() };
    std::weak_ptr<SyncQueue<bytes_t>> m_p_client_queue{};
};

#if defined(RPC_HPP_ENABLE_NJSON)
extern std::unique_ptr<TestServer<adapters::njson_adapter>> njson_server;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
extern std::unique_ptr<TestServer<adapters::rapidjson_adapter>> rapidjson_server;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
extern std::unique_ptr<TestServer<adapters::boost_json_adapter>> boost_json_server;
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
extern std::unique_ptr<TestServer<adapters::bitsery_adapter>> bitsery_server;
#endif
} //namespace rpc_hpp::tests
