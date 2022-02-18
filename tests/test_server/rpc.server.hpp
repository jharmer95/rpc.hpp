///@file rpc.server.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Declarations and implementations of an RPC server for testing
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2021, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#pragma once

#include "../test_structs.hpp"

#include <asio.hpp>
#include <rpc.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using asio::ip::tcp;

extern std::atomic<bool> RUNNING;

// Forward declares
[[noreturn]] void ThrowError() noexcept(false);
void KillServer() noexcept;

// cached
constexpr int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

size_t StrLen(const std::string& str);
std::vector<int> AddOneToEach(std::vector<int> vec);
void AddOneToEachRef(std::vector<int>& vec);

// cached
constexpr uint64_t Fibonacci(const uint64_t number)
{
    return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}

void FibonacciRef(uint64_t& number);

// cached
constexpr double Average(const double n1, const double n2, const double n3, const double n4,
    const double n5, const double n6, const double n7, const double n8, const double n9,
    const double n10)
{
    return (n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10) / 10.00;
}

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

std::vector<uint64_t> GenRandInts(uint64_t min, uint64_t max, size_t sz);
std::string HashComplex(const ComplexObject& cx);
void HashComplexRef(ComplexObject& cx, std::string& hashStr);

template<typename Serial>
class TestServer final : public rpc_hpp::server_interface<Serial>
{
public:
    TestServer(asio::io_context& io, const uint16_t port)
        : m_accept(io, tcp::endpoint(tcp::v4(), port))
    {
    }

    void Run()
    {
        static constexpr auto BUFFER_SZ = 64U * 1024UL;
        std::array<uint8_t, BUFFER_SZ> data{};

        while (RUNNING)
        {
            tcp::socket sock = m_accept.accept();

            try
            {
                while (RUNNING)
                {
                    asio::error_code error;
                    const size_t len = sock.read_some(asio::buffer(data.data(), BUFFER_SZ), error);

                    if (error == asio::error::eof)
                    {
                        break;
                    }

                    // other error
                    if (error)
                    {
                        throw asio::system_error(error);
                    }

                    const auto bytes = this->dispatch({ std::begin(data), std::begin(data) + len });
                    write(sock, asio::buffer(bytes, bytes.size()));
                }
            }
            catch (const std::exception& ex)
            {
                fprintf(stderr, "Exception in thread: %s\n", ex.what());
            }
        }
    }

private:
    tcp::acceptor m_accept;
};
