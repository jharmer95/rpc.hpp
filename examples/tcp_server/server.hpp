#pragma once

#include <asio.hpp>

#include <rpc_adapters/rpc_njson.hpp>

#include <atomic>
#include <cstdint>

using asio::ip::tcp;
using rpc::adapters::njson_adapter;

class RpcServer : public rpc::server_interface<njson_adapter>
{
public:
    RpcServer(asio::io_context& io, uint16_t port) : m_io(io), m_port(port) {}

    void Run();
    void Stop() { m_running = false; }

private:
    void dispatch_impl(rpc::adapters::njson::njson_t& serial_obj) override;

    asio::io_context& m_io;
    std::atomic<bool> m_running{ false };
    uint16_t m_port;
};
