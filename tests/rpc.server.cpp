#include "rpc.hpp"

#include <asio.hpp>

#include <iostream>
#include <thread>

#include "rpc_adapters/rpc_njson.hpp"
#include "rpc_adapters/rpc_rapidjson.hpp"

#include "rpc_dispatch_helper.hpp"

using asio::ip::tcp;

inline int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

inline size_t StrLen(std::string str)
{
    return str.size();
}

inline std::vector<int> AddOneToEach(std::vector<int> vec)
{
    for (auto& n : vec)
    {
        ++n;
    }

    return vec;
}

void PtrSum(int* n1, const int n2)
{
    *n1 += n2;
}

RPC_DEFAULT_DISPATCH(SimpleSum, StrLen, AddOneToEach)

template<typename Serial>
void session(tcp::socket sock)
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
            auto serial_obj = rpc::serial_adapter<Serial>::from_string(str);
            rpc::server::dispatch(serial_obj);
            const auto ret_str = rpc::serial_adapter<Serial>::to_string(serial_obj);

            write(sock, asio::buffer(ret_str, ret_str.size()));
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception in thread: " << ex.what() << '\n';
    }
}

constexpr uint16_t PORT_NJSON = 5000;
constexpr uint16_t PORT_NCBOR = 5001;
constexpr uint16_t PORT_NBSON = 5002;
constexpr uint16_t PORT_NMSGPACK = 5003;
constexpr uint16_t PORT_NUBJSON = 5004;
constexpr uint16_t PORT_RAPIDJSON = 5005;

[[noreturn]] void server(asio::io_context& io_context)
{
#if defined(RPC_HPP_NJSON_ENABLED)
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), PORT_NJSON));
    std::cout << "Running njson server on port " << PORT_NJSON << "...\n";
#endif

#if defined(RPC_HPP_NCBOR_ENABLED)
    tcp::acceptor b(io_context, tcp::endpoint(tcp::v4(), PORT_NCBOR));
    std::cout << "Running ncbor server on port " << PORT_NCBOR << "...\n";
#endif

#if defined(RPC_HPP_NBSON_ENABLED)
    tcp::acceptor c(io_context, tcp::endpoint(tcp::v4(), PORT_NBSON));
    std::cout << "Running nbson server on port " << PORT_NBSON << "...\n";
#endif

#if defined(RPC_HPP_NMSGPACK_ENABLED)
    tcp::acceptor d(io_context, tcp::endpoint(tcp::v4(), PORT_NMSGPACK));
    std::cout << "Running nmsgpack server on port " << PORT_NMSGPACK << "...\n";
#endif

#if defined(RPC_HPP_NUBJSON_ENABLED)
    tcp::acceptor e(io_context, tcp::endpoint(tcp::v4(), PORT_NUBJSON));
    std::cout << "Running nubjson server on port " << PORT_NUBJSON << "...\n";
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
    tcp::acceptor f(io_context, tcp::endpoint(tcp::v4(), PORT_RAPIDJSON));
    std::cout << "Running rapidjson server on port " << PORT_RAPIDJSON << "...\n";
#endif

    while (true)
    {
#if defined(RPC_HPP_NJSON_ENABLED)
        std::thread(session<njson>, a.accept()).detach();
#endif

#if defined(RPC_HPP_NCBOR_ENABLED)
        std::thread(session<ncbor>, b.accept()).detach();
#endif

#if defined(RPC_HPP_NBSON_ENABLED)
        std::thread(session<nbson>, c.accept()).detach();
#endif

#if defined(RPC_HPP_NMSGPACK_ENABLED)
        std::thread(session<nmsgpack>, d.accept()).detach();
#endif

#if defined(RPC_HPP_NUBJSON_ENABLED)
        std::thread(session<nubjson>, e.accept()).detach();
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
        std::thread(session<rpdjson_doc>, f.accept()).detach();
#endif
    }
}

int main()
{
    try
    {
        asio::io_context io_context;
        server(io_context);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
        return 1;
    }
}
