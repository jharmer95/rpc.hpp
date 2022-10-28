///@file rpc.server.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of an RPC server for testing
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

#define RPC_HPP_ASSERT_NONE

#include "server.hpp"
#include "../bench_funcs.hpp"

#if defined(RPC_HPP_ENABLE_NJSON)
#  include <rpc_adapters/rpc_njson.hpp>

using rpc_hpp::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  include <rpc_adapters/rpc_rapidjson.hpp>

using rpc_hpp::adapters::rapidjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  include <rpc_adapters/rpc_boost_json.hpp>

using rpc_hpp::adapters::boost_json_adapter;
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <rpc_adapters/rpc_bitsery.hpp>

using rpc_hpp::adapters::bitsery_adapter;

constexpr size_t bitsery_adapter::config::max_func_name_size = 30;
constexpr size_t bitsery_adapter::config::max_string_size = 2'048;
constexpr size_t bitsery_adapter::config::max_container_size = 1'000;
#endif

#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

std::atomic<bool> RUNNING{ false };

namespace
{
// NOTE: This function is only for testing purposes. Obviously you would not want this in a production server!
void KillServer() noexcept
{
    std::puts("\nShutting down from remote KillServer call...");
    ::RUNNING = false;
}

template<typename Serial>
static void BindFuncs(TestServer<Serial>& server)
{
    server.bind("KillServer", &KillServer);
    server.bind("GenRandInts", &GenRandInts);

    // Cached
    server.bind("Fibonacci", &Fibonacci);
    server.bind("Average", &Average);
    server.bind("StdDev", &StdDev);
    server.bind("AverageContainer<uint64_t>", &AverageContainer<uint64_t>);
    server.bind("AverageContainer<double>", &AverageContainer<double>);
    server.bind("HashComplex", &HashComplex);
}
} //namespace

int main(const int argc, const char* argv[])
{
    if ((argc > 1) && (std::strcmp(argv[1], "--help") == 0))
    {
        return 0;
    }

    try
    {
        asio::io_context io_ctx{};
        ::RUNNING = true;

        std::vector<std::thread> threads;

#if defined(RPC_HPP_ENABLE_NJSON)
        TestServer<njson_adapter> njson_server{ io_ctx, 5000U };
        BindFuncs(njson_server);
        threads.emplace_back(&TestServer<njson_adapter>::Run, &njson_server);
        std::puts("Running njson server on port 5000...");
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
        TestServer<rapidjson_adapter> rapidjson_server{ io_ctx, 5001U };
        BindFuncs(rapidjson_server);
        threads.emplace_back(&TestServer<rapidjson_adapter>::Run, &rapidjson_server);
        std::puts("Running rapidjson server on port 5001...");
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
        TestServer<boost_json_adapter> bjson_server{ io_ctx, 5002U };
        BindFuncs(bjson_server);
        threads.emplace_back(&TestServer<boost_json_adapter>::Run, &bjson_server);
        std::puts("Running Boost.JSON server on port 5002...");
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
        TestServer<bitsery_adapter> bitsery_server{ io_ctx, 5003U };
        BindFuncs(bitsery_server);
        threads.emplace_back(&TestServer<bitsery_adapter>::Run, &bitsery_server);
        std::puts("Running Bitsery server on port 5003...");
#endif

        for (auto& thrd : threads)
        {
            thrd.join();
        }

        std::puts("Exited normally");
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::fprintf(stderr, "Exception: %s\n", ex.what());
        return 1;
    }
}
