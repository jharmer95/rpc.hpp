///@file test_server.hpp
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

#include "sync_queue.hpp"
#include "test_structs.hpp"

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
#include <cstdio>
#include <exception>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

namespace rpc_hpp::tests
{
template<typename Serial>
class TestServer final : public callback_server_interface<Serial>
{
public:
    using typename callback_server_interface<Serial>::bytes_t;
    using typename callback_server_interface<Serial>::object_t;
    using callback_server_interface<Serial>::bind;
    using callback_server_interface<Serial>::call_callback;
    using callback_server_interface<Serial>::handle_bytes;

    [[nodiscard]] bytes_t receive()
    {
        auto response = m_p_message_queue->pop();

        if (!response.has_value())
        {
            throw server_receive_error{ "Test server error: client did not provide a response" };
        }

        return response.value();
    }

    void send(bytes_t&& bytes)
    {
        if (m_p_client_queue.expired())
        {
            throw server_receive_error{
                "Test server error: no clients are attached to the server"
            };
        }

        m_p_client_queue.lock()->push(std::move(bytes));
    }

    [[nodiscard]] std::weak_ptr<SyncQueue<bytes_t>> attach_client(
        std::weak_ptr<SyncQueue<bytes_t>> p_client_queue)
    {
        assert(m_p_client_queue.use_count() == 0 && "Only one client can be attached (for now)");
        m_p_client_queue = p_client_queue;
        return m_p_message_queue;
    }

    std::string GetConnectionInfo()
    {
        return std::string{ "Server name: MyServer\nClient name: " }.append(
            this->template call_callback<std::string>("GetClientName"));
    }

    void Run()
    {
        m_running = true;
        m_p_message_queue->activate();

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
        m_running.store(false);
        m_p_message_queue->deactivate();
        m_p_client_queue.lock()->deactivate();
    }

private:
    [[nodiscard]] auto recv_impl() -> object_t
    {
        bytes_t bytes = [this]
        {
            try
            {
                return receive();
            }
            catch (const std::exception& ex)
            {
                throw server_receive_error{ ex.what() };
            }
        }();

        auto o_response = object_t::parse_bytes(std::move(bytes));

        if (!o_response.has_value())
        {
            throw server_receive_error{ "Test server error: invalid RPC object received" };
        }

        switch (auto& response = o_response.value(); response.type())
        {
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
            case rpc_type::callback_error:
                return std::move(response);

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::func_error:
            case rpc_type::func_request:
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
                throw object_mismatch_error{
                    "Test server error: invalid rpc_object type detected"
                };

            default:
                RPC_HPP_ASSUME(0);
        }
    }

    auto call_callback_impl(object_t&& request) -> object_t override
    {
        RPC_HPP_PRECONDITION(request.type() == rpc_type::callback_request);

        if (const auto func_name = request.get_func_name();
            m_installed_callbacks.find(func_name) == m_installed_callbacks.cend())
        {
            throw callback_missing_error{
                std::string{ "Test server error: callback " }.append(func_name).append(
                    "() was called but not installed")
            };
        }

        try
        {
            send(std::move(request).to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw server_send_error{ ex.what() };
        }

        return recv_impl();
    }

    void install_callback(object_t& rpc_obj) override
    {
        const auto func_name = rpc_obj.get_func_name();
        const auto [_, inserted] = m_installed_callbacks.insert(func_name);

        if (!inserted)
        {
            rpc_obj = object_t{ detail::callback_error{ func_name,
                callback_install_error(
                    std::string{ "Test server error: callback " }.append(func_name).append(
                        "() is already installed")) } };
        }
    }

    void uninstall_callback(const object_t& rpc_obj) override
    {
        m_installed_callbacks.erase(rpc_obj.get_func_name());
    }

    std::atomic<bool> m_running{ false };
    std::shared_ptr<SyncQueue<bytes_t>> m_p_message_queue{ std::make_shared<SyncQueue<bytes_t>>() };
    std::weak_ptr<SyncQueue<bytes_t>> m_p_client_queue{};
    std::unordered_set<std::string> m_installed_callbacks{};
};

template<typename Serial>
std::shared_ptr<TestServer<Serial>> GetServer();

#if defined(RPC_HPP_ENABLE_NJSON)
extern template std::shared_ptr<TestServer<adapters::njson_adapter>> GetServer<
    adapters::njson_adapter>();
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
extern template std::shared_ptr<TestServer<adapters::rapidjson_adapter>> GetServer<
    adapters::rapidjson_adapter>();
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
extern template std::shared_ptr<TestServer<adapters::boost_json_adapter>> GetServer<
    adapters::boost_json_adapter>();
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
extern template std::shared_ptr<TestServer<adapters::bitsery_adapter>> GetServer<
    adapters::bitsery_adapter>();
#endif
} //namespace rpc_hpp::tests
