///@file test_client.hpp
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

#include "sync_queue.hpp"
#include "test_server.hpp"

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

#include <memory>
#include <utility>

namespace rpc_hpp::tests
{
template<typename Serial>
class TestClient final : public rpc_hpp::client_interface<Serial>
{
public:
    using bytes_t = typename Serial::bytes_t;

    explicit TestClient(std::shared_ptr<TestServer<Serial>> server)
        : m_p_message_queue{ std::make_shared<SyncQueue<bytes_t>>() },
          m_p_server_queue(server->attach_client(m_p_message_queue))
    {
        m_p_message_queue->activate();
    }

    void send(bytes_t&& mesg) override { m_p_server_queue.lock()->push(std::move(mesg)); }
    [[nodiscard]] bytes_t receive() override
    {
        if (m_p_server_queue.expired())
        {
            throw client_receive_error{ "Server is deactivated" };
        }

        auto response = m_p_message_queue->pop();

        if (!response.has_value())
        {
            throw client_receive_error{ "Server did not provide a response" };
        }

        return response.value();
    }

private:
    std::shared_ptr<SyncQueue<bytes_t>> m_p_message_queue;
    std::weak_ptr<SyncQueue<bytes_t>> m_p_server_queue;
};

template<typename Serial>
std::shared_ptr<TestClient<Serial>> GetClient()
{
    static std::shared_ptr<TestClient<Serial>> p_client{};

    if (!p_client)
    {
        p_client = std::make_shared<TestClient<Serial>>(GetServer<Serial>());
    }

    return p_client;
}
} //namespace rpc_hpp::tests
