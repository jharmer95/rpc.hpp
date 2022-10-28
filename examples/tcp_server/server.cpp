#include "server.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

int Sum(int num1, int num2)
{
    return num1 + num2;
}

void AddOneToEach(std::vector<int>& vec)
{
    for (auto& num : vec)
    {
        ++num;
    }
}

template<typename T>
std::string GetTypeName()
{
    return std::string{ typeid(T).name() };
}

std::string RpcServer::receive()
{
    static constexpr unsigned BUFFER_SZ = 64 * 1024;
    static std::array<uint8_t, BUFFER_SZ> data{};

    assert(m_socket.has_value());
    asio::error_code error;
    const size_t len = m_socket->read_some(asio::buffer(data.data(), BUFFER_SZ), error);

    if (error == asio::error::eof)
    {
        return "";
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
    assert(m_socket.has_value());
    write(*m_socket, asio::buffer(std::move(bytes), bytes.size()));
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

    const auto port_num = static_cast<uint16_t>(std::strtoul(argv[1], nullptr, 10));

    try
    {
        asio::io_context io_context{};

        const auto p_server = std::make_unique<RpcServer>(io_context, port_num);

        p_server->bind("Sum", &Sum);
        p_server->bind("AddOneToEach", &AddOneToEach);
        p_server->bind("GetTypeName<int>", &GetTypeName<int>);
        p_server->bind("GetTypeName<double>", &GetTypeName<double>);
        p_server->bind("GetTypeName<std::string>", &GetTypeName<std::string>);

        // NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
        p_server->template bind<void>("KillServer", [&p_server] { p_server->Stop(); });

        std::thread server_thread{ &RpcServer::Run, p_server.get() };
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
