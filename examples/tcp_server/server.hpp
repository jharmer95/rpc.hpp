#pragma once

#include <asio.hpp>

#include <rpc_adapters/rpc_njson.hpp>
#include <rpc_server.hpp>

#include <atomic>
#include <cstdint>
#include <optional>

using asio::ip::tcp;
using rpc_hpp::adapters::njson_adapter;

class RpcServer : public rpc_hpp::server_interface<njson_adapter>
{
public:
    RpcServer(asio::io_context& io_ctx, const uint16_t port)
        : m_accept(io_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
    }

    std::string receive() override;
    void send(std::string&&) override;

    void Run();
    void Stop() noexcept { m_running = false; }

private:
    mutable std::atomic<bool> m_running{ false };
    asio::ip::tcp::acceptor m_accept;
    std::optional<asio::ip::tcp::socket> m_socket{};
};
