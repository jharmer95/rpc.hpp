#pragma once

#include "rpc.hpp"

#include <asio.hpp>

using asio::ip::tcp;

class TestClient final : public rpc::client::client_base
{
public:
    TestClient(std::string_view host, std::string_view port) : m_socket(m_io), m_resolver(m_io)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    void send(const std::string& mesg) override
    {
        write(m_socket, asio::buffer(mesg, mesg.size()));
    }

    [[nodiscard]] std::string receive() override
    {
        const auto numBytes = m_socket.read_some(asio::buffer(m_buffer, 64U * 1024UL));
        return std::string(m_buffer, m_buffer + numBytes);
    }

    [[nodiscard]] std::string getIP() const
    {
        return m_socket.remote_endpoint().address().to_string();
    }

private:
    asio::io_context m_io{};
    tcp::socket m_socket;
    tcp::resolver m_resolver;
    char m_buffer[64U * 1024UL]{};
};
