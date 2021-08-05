#pragma once

#include <asio.hpp>
#include <rpc_adapters/rpc_njson.hpp>

#include <array>
#include <cstdint>
#include <string>

using asio::ip::tcp;
using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

class RpcClient : public rpc::client::client_interface<njson_adapter>
{
public:
    RpcClient(const std::string& host, const std::string& port)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    std::string getIP() const { return m_socket.remote_endpoint().address().to_string(); }

private:
    void send(const std::string& mesg) override { asio::write(m_socket, asio::buffer(mesg)); }

    [[nodiscard("Data is lost after receive is called")]] std::string receive() override
    {
        const auto numBytes = m_socket.read_some(asio::buffer(m_buffer));
        return std::string{ m_buffer.data(), numBytes };
    }

    // Define a maximum buffer size for sending data to the server, keeping it small for this example
    static constexpr size_t BUF_SZ = 256;

    asio::io_context m_io{};
    tcp::resolver m_resolver{ m_io };
    tcp::socket m_socket{ m_io };
    std::array<char, BUF_SZ> m_buffer{};
};
