#define RPC_HPP_SERVER_IMPL

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

void RpcServer::Run()
{
    m_running = true;

    while (m_running)
    {
        tcp::acceptor acc(m_io, tcp::endpoint(tcp::v4(), m_port));
        tcp::socket sock = acc.accept();

        try
        {
            constexpr auto BUFFER_SZ = 128;
            uint8_t data[BUFFER_SZ];
            while (true)
            {
                asio::error_code error;
                const size_t len = sock.read_some(asio::buffer(data, BUFFER_SZ), error);

                if (error == asio::error::eof)
                {
                    break;
                }

                // other error
                if (error)
                {
                    throw asio::system_error(error);
                }

                std::string bytes(data, data + len);
                dispatch(bytes);

                write(sock, asio::buffer(bytes, bytes.size()));
            }
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception in server thread #" << std::this_thread::get_id() << ": "
                      << ex.what() << '\n';
        }
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
