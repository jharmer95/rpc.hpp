#pragma once

#include "test_structs.hpp"

#include <asio.hpp>
#include <rpc.hpp>
#include <rpc_dispatch_helper.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using asio::ip::tcp;

// Forward declares
[[noreturn]] void ThrowError() noexcept(false);
void KillServer() noexcept;
int SimpleSum(int n1, int n2);
size_t StrLen(std::string str);
std::vector<int> AddOneToEach(std::vector<int> vec);
void AddOneToEachRef(std::vector<int>& vec);
uint64_t Fibonacci(uint64_t number);
void FibonacciRef(uint64_t& number);
double Average(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10);

double StdDev(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10);

void SquareRootRef(double& n1, double& n2, double& n3, double& n4, double& n5, double& n6,
    double& n7, double& n8, double& n9, double& n10);

// cached
template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}

std::vector<uint64_t> GenRandInts(const uint64_t min, const uint64_t max, const size_t sz = 1000);
std::string HashComplex(const ComplexObject& cx);
void HashComplexRef(ComplexObject& cx, std::string& hashStr);

template<typename Serial>
class TestServer : public rpc::server_interface<Serial>
{
public:
    TestServer(asio::io_context& io, uint16_t port) : m_accept(io, tcp::endpoint(tcp::v4(), port))
    {
    }

    [[noreturn]] void Run()
    {
        constexpr size_t BUF_SZ = 16 * 1024;
        std::array<char, BUF_SZ> data_buf;

        while (true)
        {
            tcp::socket sock = m_accept.accept();

            try
            {
                while (true)
                {
                    asio::error_code error;
                    const size_t len = sock.read_some(asio::buffer(data_buf), error);

                    if (error == asio::error::eof)
                    {
                        break;
                    }

                    // other error
                    if (error)
                    {
                        throw asio::system_error(error);
                    }

                    typename Serial::bytes_t bytes{ data_buf.data(), len };
                    this->dispatch(bytes);

#if !defined(NDEBUG)
                    std::cout << "Return message: \"" << bytes << "\"\n";
#endif

                    write(sock, asio::buffer(bytes));
                }
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Exception in thread: " << ex.what() << '\n';
            }
        }
    }

private:
    void dispatch_impl(typename Serial::serial_t& serial_obj) override
    {
        const auto func_name = rpc::pack_adapter<Serial>::get_func_name(serial_obj);

        RPC_ATTACH_FUNCS(KillServer, ThrowError, SimpleSum, AddOneToEachRef, FibonacciRef,
            SquareRootRef, GenRandInts, HashComplex, HashComplexRef, StrLen)

        RPC_ATTACH_CACHED_FUNCS(AddOneToEach, Fibonacci, Average, StdDev,
            AverageContainer<uint64_t>, AverageContainer<double>)

        throw std::runtime_error("RPC error: Called function: \"" + func_name + "\" not found!");
    }

    tcp::acceptor m_accept;
};
