#pragma once

#include <asio.hpp>
#include <rpc_server.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

extern std::atomic<bool> RUNNING;

template<typename Serial>
class TestServer final : public rpc_hpp::server_interface<Serial>
{
public:
    using bytes_t = typename rpc_hpp::server_interface<Serial>::bytes_t;
    using rpc_hpp::server_interface<Serial>::bind;
    using rpc_hpp::server_interface<Serial>::handle_bytes;

    TestServer() = delete;
    TestServer(asio::io_context& io_ctx, const uint16_t port)
        : rpc_hpp::server_interface<Serial>{},
          m_accept(io_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
    }

    bytes_t receive()
    {
        RPC_HPP_PRECONDITION(m_socket.has_value());

        static constexpr unsigned BUFFER_SZ = 64 * 1024;
        static std::array<uint8_t, BUFFER_SZ> data{};

        asio::error_code error;
        const size_t len = m_socket.value().read_some(asio::buffer(data.data(), BUFFER_SZ), error);

        if (error == asio::error::eof)
        {
            return {};
        }

        // other error
        if (error)
        {
            throw asio::system_error{ error };
        }

        return { data.begin(), std::next(data.begin(), static_cast<ptrdiff_t>(len)) };
    }

    void send(bytes_t&& bytes)
    {
        RPC_HPP_PRECONDITION(m_socket.has_value());
        write(m_socket.value(), asio::buffer(std::move(bytes), bytes.size()));
    }

    void Run()
    {
        while (::RUNNING)
        {
            m_socket = m_accept.accept();

            try
            {
                while (::RUNNING)
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

            m_socket.reset();
        }
    }

private:
    asio::ip::tcp::acceptor m_accept;
    std::optional<asio::ip::tcp::socket> m_socket{};
};
