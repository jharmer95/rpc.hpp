#include "rpc.hpp"

#include <asio.hpp>

#include <iostream>

#include "rpc_adapters/rpc_njson.hpp"

using asio::ip::tcp;

int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

void PtrSum(const int n1, const int n2, int* result)
{
    *result = n1 + n2;
}

const std::unordered_map<std::string_view, size_t> rpc::server::dispatch_table{
    { "SimpleSum", reinterpret_cast<size_t>(&SimpleSum) },
    { "PtrSum", reinterpret_cast<size_t>(&PtrSum) },
};

void session(tcp::socket sock)
{
    try
    {
        while (true)
        {
            char data[64U * 1024UL];

            asio::error_code error;
            const size_t len = sock.read_some(asio::buffer(data), error);

            if (error == asio::error::eof)
            {
                break;
            }
            else if (error)
            {
                throw asio::system_error(error);
            }

            const std::string str(data, data + len);
            const auto serial_obj = rpc::serial_adapter<njson>::from_string(str);
            auto func = rpc::server::dispatch_table.at(serial_obj["func_name"]);
            //rpc::server::run(rpc::serial_adapter<njson>::to_packed_func<>());

            //asio::write(sock, asio::buffer(str, str.size()));
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception in thread: " << ex.what() << '\n';
    }
}

void server(asio::io_context& io_context, uint16_t port)
{
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));

    while (true)
    {
        std::thread(session, a.accept()).detach();
    }
}

int main()
{
    try
    {
        asio::io_context io_context;
        server(io_context, 55555);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
