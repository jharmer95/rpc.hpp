#pragma once

#include <asio.hpp>
#include <rpc_adapters/rpc_njson.hpp>

#include <string>

using asio::ip::tcp;
using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

class RpcClient : public rpc::client::client_interface<njson_adapter>
{
public:
    RpcClient(const std::string_view host, const std::string_view port)
        : m_socket(m_io), m_resolver(m_io)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    void send(const std::string& mesg) override
    {
        asio::write(m_socket, asio::buffer(mesg, mesg.size()));
    }

    std::string receive() override
    {
        const auto numBytes = m_socket.read_some(asio::buffer(m_buffer, 64U * 1024UL));
        return std::string(m_buffer, m_buffer + numBytes);
    }

    std::string getIP() const
    {
        return m_socket.remote_endpoint().address().to_string();
    }

private:
    asio::io_context m_io{};
    tcp::socket m_socket;
    tcp::resolver m_resolver;
    uint8_t m_buffer[64U * 1024UL]{};
};