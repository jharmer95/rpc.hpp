#pragma once

#include <asio.hpp>

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include <rpc_adapters/rpc_boost_json.hpp>

using rpc::adapters::bjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#    include <rpc_adapters/rpc_njson.hpp>

using rpc::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include <rpc_adapters/rpc_rapidjson.hpp>

using rpc::adapters::rapidjson_adapter;
#endif

using asio::ip::tcp;

template<typename Serial>
class TestClient final : public rpc::client_interface<Serial>
{
public:
    TestClient(const std::string_view host, const std::string_view port)
        : m_socket(m_io), m_resolver(m_io)
    {
        asio::connect(m_socket, m_resolver.resolve(host, port));
    }

    void send(const typename Serial::bytes_t& mesg) override
    {
        asio::write(m_socket, asio::buffer(mesg, mesg.size()));
    }

    [[nodiscard]] typename Serial::bytes_t receive() override
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
    uint8_t m_buffer[64U * 1024UL]{};
};
