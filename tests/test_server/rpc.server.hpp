///@file rpc.server.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Declarations and implementations of an RPC server for testing
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
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
#include <rpc_server.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <forward_list>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace test_server
{
inline std::atomic<bool> RUNNING{ false };

// Forward declares
[[noreturn]] void ThrowError() noexcept(false);
void KillServer() noexcept;

// cached
constexpr int SimpleSum(const int num1, const int num2)
{
    return num1 + num2;
}

size_t StrLen(std::string_view str);
std::vector<int> AddOneToEach(std::vector<int> vec);
void AddOneToEachRef(std::vector<int>& vec);

// cached
constexpr uint64_t Fibonacci(const uint64_t number)
{
    return (number < 2) ? 1 : (Fibonacci(number - 1) + Fibonacci(number - 2));
}

void FibonacciRef(uint64_t& number);

// cached
constexpr double Average(const double num1, const double num2, const double num3, const double num4,
    const double num5, const double num6, const double num7, const double num8, const double num9,
    const double num10)
{
    return (num1 + num2 + num3 + num4 + num5 + num6 + num7 + num8 + num9 + num10) / 10.00;
}

double StdDev(double num1, double num2, double num3, double num4, double num5, double num6,
    double num7, double num8, double num9, double num10);

void SquareRootRef(double& num1, double& num2, double& num3, double& num4, double& num5,
    double& num6, double& num7, double& num8, double& num9, double& num10);

// cached
template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}

void SquareArray(std::array<int, 12>& arr);
void RemoveFromList(
    std::forward_list<std::string>& list, const std::string& str, bool case_sensitive);
std::map<char, unsigned> CharacterMap(std::string_view str);
size_t CountResidents(const std::multimap<int, std::string>& registry, int floor_num);
std::unordered_set<std::string> GetUniqueNames(const std::vector<std::string>& names);
std::vector<uint64_t> GenRandInts(uint64_t min, uint64_t max, size_t num_ints);
std::string HashComplex(const ComplexObject& cx_obj);
void HashComplexRef(ComplexObject& cx_obj, std::string& hashStr);

template<typename Serial>
class TestServer final : public rpc_hpp::server_interface<Serial>
{
public:
    using bytes_t = typename rpc_hpp::server_interface<Serial>::bytes_t;

    TestServer(asio::io_context& io_ctx, const uint16_t port)
        : m_accept(io_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
    }

    bytes_t receive() override
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
            throw asio::system_error(error);
        }

        return { data.data(), data.data() + len };
    }

    void send(bytes_t&& bytes) override
    {
        RPC_HPP_PRECONDITION(m_socket.has_value());
        write(m_socket.value(), asio::buffer(std::move(bytes), bytes.size()));
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    std::string GetConnectionInfo()
    {
        std::stringstream ss;

        ss << "Server name: MyServer\n";
        ss << "Client name: " << this->template call_callback<std::string>("GetClientName") << '\n';

        return ss.str();
    }
#endif

    void Run()
    {
        while (RUNNING)
        {
            m_socket = m_accept.accept();

            try
            {
                while (RUNNING)
                {
                    auto bytes = receive();

                    if (std::size(bytes) == 0)
                    {
                        break;
                    }

                    auto response = this->handle_bytes(std::move(bytes));
                    send(response.to_bytes());
                }
            }
            catch (const std::exception& ex)
            {
                std::fprintf(stderr, "Exception in thread: %s\n", ex.what());
            }

            m_socket.reset();
        }
    }

private:
    asio::ip::tcp::acceptor m_accept;
    std::optional<asio::ip::tcp::socket> m_socket{};
};
} //namespace test_server
