#define RPC_HPP_SERVER_IMPL

#include "server.hpp"

#include <rpc_dispatch_helper.hpp>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using asio::ip::tcp;
using rpc::adapters::njson_adapter;

static std::unique_ptr<RpcServer> P_SERVER;

// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
inline void KillServer()
{
    P_SERVER->Stop();
}

// Note: constexpr is more or less pointless for this example,
// but shows that you could use the const-evaluated version on the server and the non const-evaluated remotely
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

void RpcServer::Run()
{
    // Define a maximum buffer size for receiving data from client, keeping it small for this example
    constexpr size_t BUFFER_SZ = 256;
    std::array<char, BUFFER_SZ> data_buf{};

    m_running = true;

    while (m_running)
    {
        tcp::acceptor acc(m_io, tcp::endpoint(tcp::v4(), m_port));
        tcp::socket sock = acc.accept();

        try
        {
            while (true)
            {
                asio::error_code error;
                const auto len = sock.read_some(asio::buffer(data_buf), error);

                if (error == asio::error::eof)
                {
                    break;
                }

                // other error
                if (error)
                {
                    throw asio::system_error(error);
                }

                std::string bytes(data_buf.data(), len);
                dispatch(bytes);

                write(sock, asio::buffer(bytes));
            }
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception in server thread #" << std::this_thread::get_id() << ": "
                      << ex.what() << '\n';
        }
    }
}

void RpcServer::dispatch_impl(njson& serial_obj)
{
    // IMPORTANT: Template functions must be given explicitly with the supported types
    // or they will not be instantiated on the server
    RPC_DEFAULT_DISPATCH(Sum, AddOneToEach, GetTypeName<int>, GetTypeName<double>,
        GetTypeName<std::string>, KillServer)
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
        P_SERVER = std::make_unique<RpcServer>(port_num);

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
