///@file rpc.server.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Example implementation of an RPC server
///@version 0.2.0.0
///@date 09-09-2020
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020, Jackson Harmer
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

#include "rpc.hpp"

#include <asio.hpp>

#include <iostream>
#include <thread>

#if defined(RPC_HPP_NJSON_ENABLED)
#    include "rpc_adapters/rpc_njson.hpp"
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
#    include "rpc_adapters/rpc_rapidjson.hpp"
#endif

#include "rpc_dispatch_helper.hpp"

#include "test_structs.hpp"

using asio::ip::tcp;

constexpr int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

inline size_t StrLen(const std::string& str)
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

inline void AddOneToEachRef(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

inline void ChangeNumber(TestObject& obj, const int index, const int value)
{
    obj.numbers[index] = value;
}

constexpr void PtrSum(int* const n1, const int n2)
{
    *n1 += n2;
}

RPC_DEFAULT_DISPATCH(SimpleSum, StrLen, AddOneToEach, AddOneToEachRef, ChangeNumber)

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
constexpr uint16_t PORT_N_SERIAL = 5001;
constexpr uint16_t PORT_RAPIDJSON = 5002;

[[noreturn]] void server(asio::io_context& io_context)
{
#if defined(RPC_HPP_NJSON_ENABLED)
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), PORT_NJSON));
    std::cout << "Running njson server on port " << PORT_NJSON << "...\n";

#    if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
    tcp::acceptor b(io_context, tcp::endpoint(tcp::v4(), PORT_N_SERIAL));
    std::cout << "Running nlohmann/serial_type server on port " << PORT_N_SERIAL << "...\n";
#    endif
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
    tcp::acceptor c(io_context, tcp::endpoint(tcp::v4(), PORT_RAPIDJSON));
    std::cout << "Running rapidjson server on port " << PORT_RAPIDJSON << "...\n";
#endif

    while (true)
    {
#if defined(RPC_HPP_NJSON_ENABLED)
        std::thread(session<njson>, a.accept()).detach();
#    if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
        std::thread(session<generic_serial_t>, b.accept()).detach();
#    endif
#endif

#if defined(RPC_HPP_RAPIDJSON_ENABLED)
        std::thread(session<rpdjson_doc>, c.accept()).detach();
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
