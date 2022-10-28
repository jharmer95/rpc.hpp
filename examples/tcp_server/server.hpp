#pragma once

#include <asio.hpp>

#include <rpc_adapters/rpc_njson.hpp>
#include <rpc_server.hpp>

#include <atomic>
#include <cstdint>
#include <optional>

class RpcServer : public rpc_hpp::server_interface<rpc_hpp::adapters::njson_adapter>
{
public:
    RpcServer(asio::io_context& io_ctx, const uint16_t port)
        : m_accept(io_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
    }

    std::string receive();
    void send(std::string&& bytes);

    void Run();
    void Stop() noexcept { m_running = false; }

private:
    mutable std::atomic<bool> m_running{ false };
    asio::ip::tcp::acceptor m_accept;
    std::optional<asio::ip::tcp::socket> m_socket{};
};
