#include "rpc.hpp"

#include <asio.hpp>

#include <iostream>

#include "rpc_adapters/rpc_njson.hpp"
#include "rpc_adapters/rpc_rapidjson.hpp"

using asio::ip::tcp;

int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

int StrLen(std::string str)
{
    return str.size();
}

void PtrSum(int* n1, const int n2)
{
    *n1 += n2;
}

template<typename Serial, typename R, typename... Args>
rpc::packed_func<R, Args...> create_func(R (*)(Args...), const Serial& obj)
{
    return rpc::serial_adapter<Serial>::template to_packed_func<R, Args...>(obj);
}

template<typename Serial>
void rpc::server::dispatch(Serial& serial_obj)
{
    const auto func_name = serial_adapter<Serial>::extract_func_name(serial_obj);

    if (func_name == "SimpleSum")
    {
        auto pack = create_func(SimpleSum, serial_obj);
        run_callback(SimpleSum, pack);
        serial_obj = serial_adapter<Serial>::from_packed_func(pack);
    }
    else if (func_name == "StrLen")
    {
        auto pack = create_func(StrLen, serial_obj);
        run_callback(StrLen, pack);
        serial_obj = serial_adapter<Serial>::from_packed_func(pack);
    }
    //else if (func_name == "PtrSum")
    //{
    //    auto pack = create_func(PtrSum, serial_obj);
    //    run_callback(PtrSum, pack);
    //    serial_obj = serial_adapter<Serial>::from_packed_func(pack);
    //}
    else
    {
        throw std::runtime_error("Function not found!");
    }
}

void session_njson(tcp::socket sock)
{
    try
    {
        while (true)
        {
            std::unique_ptr<char[]> data = std::make_unique<char[]>(64U * 1024UL);

            asio::error_code error;
            const size_t len = sock.read_some(asio::buffer(data.get(), 64U * 1024UL), error);

            if (error == asio::error::eof)
            {
                break;
            }
            else if (error)
            {
                throw asio::system_error(error);
            }

            const std::string str(data.get(), data.get() + len);
            auto serial_obj = rpc::serial_adapter<njson>::from_string(str);
            rpc::server::dispatch(serial_obj);
            const auto ret_str = rpc::serial_adapter<njson>::to_string(serial_obj);

            write(sock, asio::buffer(ret_str, ret_str.size()));
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception in thread: " << ex.what() << '\n';
    }
}

void session_rapidjson(tcp::socket sock)
{
    try
    {
        while (true)
        {
            std::unique_ptr<char[]> data = std::make_unique<char[]>(64U * 1024UL);

            asio::error_code error;
            const size_t len = sock.read_some(asio::buffer(data.get(), 64U * 1024UL), error);

            if (error == asio::error::eof)
            {
                break;
            }
            else if (error)
            {
                throw asio::system_error(error);
            }

            const std::string str(data.get(), data.get() + len);
            auto serial_obj = rpc::serial_adapter<rpdjson_doc>::from_string(str);
            rpc::server::dispatch(serial_obj);
            const auto ret_str = rpc::serial_adapter<rpdjson_doc>::to_string(serial_obj);

            write(sock, asio::buffer(ret_str, ret_str.size()));
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception in thread: " << ex.what() << '\n';
    }
}

void server(asio::io_context& io_context, uint16_t port_njson, uint16_t port_rapidjson)
{
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port_njson));
    tcp::acceptor b(io_context, tcp::endpoint(tcp::v4(), port_rapidjson));
    std::cout << "Running njson server on port " << port_njson << "...\n";
    std::cout << "Running rapidjson server on port " << port_rapidjson << "...\n";

    while (true)
    {
        std::thread(session_njson, a.accept()).detach();
        std::thread(session_rapidjson, b.accept()).detach();
    }
}

int main()
{
    try
    {
        asio::io_context io_context;
        server(io_context, 5000, 5001);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
