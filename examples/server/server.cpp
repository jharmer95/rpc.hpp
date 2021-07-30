#define RPC_HPP_SERVER_IMPL
#define RPC_HPP_ENABLE_SERVER_CACHE

#include <asio.hpp>

#include <rpc_adapters/rpc_njson.hpp>
#include <rpc_dispatch_helper.hpp>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <thread>

using asio::ip::tcp;
using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

static bool RUNNING = false;

// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
void KillServer()
{
    RUNNING = false;
}

constexpr int Sum(int n1, int n2)
{
    return n1 + n2;
}

void AddOneToEach(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

template<typename T>
std::string GetTypeName()
{
    return "Unknown Type";
}

template<>
std::string GetTypeName<int>()
{
    return "int";
}

template<>
std::string GetTypeName<double>()
{
    return "double";
}

template<>
std::string GetTypeName<std::string>()
{
    return "std::string";
}

RPC_DEFAULT_DISPATCH(
    Sum, AddOneToEach, GetTypeName<int>, GetTypeName<double>, GetTypeName<std::string>, KillServer)

void session(tcp::socket sock)
{
    constexpr auto BUFFER_SZ = 128;

    uint8_t data[BUFFER_SZ];

    try
    {
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
            rpc::server::dispatch<njson_adapter>(bytes);

            write(sock, asio::buffer(bytes, bytes.size()));
        }
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Exception in thread: %s\n", ex.what());
    }
}

void server(asio::io_context& io_context, uint16_t port_num)
{
    while (RUNNING)
    {
        tcp::acceptor acc(io_context, tcp::endpoint(tcp::v4(), port_num));
        session(acc.accept());
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: rpc_server <port_num>");
        return EXIT_FAILURE;
    }

    const uint16_t port_num = strtoul(argv[1], nullptr, 10);

    try
    {
        asio::io_context io_context;
        RUNNING = true;

        std::thread server_thread{ server, std::ref(io_context), port_num };
        printf("Running server on port: %hu...\n", port_num);

        server_thread.join();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Exception: %s\n", ex.what());
        return EXIT_FAILURE;
    }
}
