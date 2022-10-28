#pragma once

#include <asio.hpp>
#include <rpc_adapters/rpc_njson.hpp>
#include <rpc_client.hpp>

#include <string>

class RpcClient : public rpc_hpp::client_interface<rpc_hpp::adapters::njson_adapter>
{
public:
    static constexpr auto BUF_SZ = 256;

    RpcClient(const std::string_view host, const std::string_view port)
        : m_socket(m_io), m_resolver(m_io)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    std::string getIP() const { return m_socket.remote_endpoint().address().to_string(); }

private:
    void send(std::string&& bytes) override
    {
        asio::write(m_socket, asio::buffer(std::move(bytes), bytes.size()));
    }

    std::string receive() override
    {
        const auto numBytes = m_socket.read_some(asio::buffer(m_buffer, BUF_SZ));
        return std::string{ &m_buffer[0], &m_buffer[numBytes] };
    }

    asio::io_context m_io{};
    asio::ip::tcp::socket m_socket;
    asio::ip::tcp::resolver m_resolver;
    uint8_t m_buffer[BUF_SZ]{};
};
