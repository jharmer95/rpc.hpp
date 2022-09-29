#include "server.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using asio::ip::tcp;
using rpc_hpp::adapters::njson_adapter;

static std::unique_ptr<RpcServer> P_SERVER;

// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
inline void KillServer()
{
    P_SERVER->Stop();
}

constexpr int Sum(int n1, int n2)
{
    return n1 + n2;
}

inline void AddOneToEach(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

template<typename T>
std::string GetTypeName()
{
    return std::string{ typeid(T).name() };
}

std::string RpcServer::receive()
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

    return { data.data(), data.data() + len };
}

void RpcServer::send(std::string&& bytes)
{
    RPC_HPP_PRECONDITION(m_socket.has_value());
    write(m_socket.value(), asio::buffer(std::move(bytes), bytes.size()));
}

void RpcServer::Run()
{
    m_running = true;

    while (m_running)
    {
        m_socket = m_accept.accept();

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
            std::cerr << "Exception in server thread #" << std::this_thread::get_id() << ": "
                      << ex.what() << '\n';
        }

        m_socket.reset();
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "USAGE: rpc_server <port_num>\n";
        return EXIT_FAILURE;
    }

    const auto port_num = static_cast<uint16_t>(strtoul(argv[1], nullptr, 10));

    try
    {
        asio::io_context io_context{};

        P_SERVER = std::make_unique<RpcServer>(io_context, port_num);
        P_SERVER->bind("KillServer", &KillServer);
        P_SERVER->bind("Sum", &Sum);
        P_SERVER->bind("AddOneToEach", &AddOneToEach);
        P_SERVER->bind("GetTypeName<int>", &GetTypeName<int>);
        P_SERVER->bind("GetTypeName<double>", &GetTypeName<double>);
        P_SERVER->bind("GetTypeName<std::string>", &GetTypeName<std::string>);

        std::thread server_thread{ &RpcServer::Run, P_SERVER.get() };
        std::cout << "Running server on port: " << port_num << "...\n";

        server_thread.join();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
