#if defined(RPC_HPP_BENCH_GRPC)
#    include "grpc/client.hpp"

static gRPC_Client& GetGrpcClient()
{
    static gRPC_Client client{};
    return client;
}
#endif

#if defined(RPC_HPP_BENCH_RPCLIB)
#    include <rpc/client.h>

static rpc::client& GetRpclibClient()
{
    static rpc::client client("127.0.0.1", 5100);
    return client;
}
#endif

#define RPC_HPP_CLIENT_IMPL
#include "test_client/rpc.client.hpp"
#include "test_structs.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

namespace nanobench = ankerl::nanobench;

#if defined(RPC_HPP_ENABLE_BITSERY)
constexpr uint64_t bitsery_adapter::config::max_func_name_size = 30;
constexpr uint64_t bitsery_adapter::config::max_string_size = 2'048;
constexpr uint64_t bitsery_adapter::config::max_container_size = 1'000;
#endif

template<typename T, typename... Args>
void bench_rpc(
    nanobench::Bench& bench, const T& expected, const std::string& func_name, Args&&... args)
{
    T test_val{};

    bench.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            nanobench::doNotOptimizeAway(
                test_val = GetClient<njson_adapter>().template call_func<T>(
                    func_name, std::forward<Args>(args)...));
        });

    if constexpr (std::is_floating_point_v<T>)
    {
        REQUIRE(test_val == doctest::Approx(expected));
    }
    else
    {
        REQUIRE(test_val == expected);
    }

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    bench.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            nanobench::doNotOptimizeAway(
                test_val = GetClient<rapidjson_adapter>().template call_func<T>(
                    func_name, std::forward<Args>(args)...));
        });

    if constexpr (std::is_floating_point_v<T>)
    {
        REQUIRE(test_val == doctest::Approx(expected));
    }
    else
    {
        REQUIRE(test_val == expected);
    }
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    bench.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            nanobench::doNotOptimizeAway(
                test_val = GetClient<boost_json_adapter>().template call_func<T>(
                    func_name, std::forward<Args>(args)...));
        });

    if constexpr (std::is_floating_point_v<T>)
    {
        REQUIRE(test_val == doctest::Approx(expected));
    }
    else
    {
        REQUIRE(test_val == expected);
    }
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    bench.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            nanobench::doNotOptimizeAway(
                test_val = GetClient<bitsery_adapter>().template call_func<T>(
                    func_name, std::forward<Args>(args)...));
        });

    if constexpr (std::is_floating_point_v<T>)
    {
        REQUIRE(test_val == doctest::Approx(expected));
    }
    else
    {
        REQUIRE(test_val == expected);
    }
#endif

#if defined(RPC_HPP_BENCH_RPCLIB)
    bench.run("rpclib",
        [&]
        {
            nanobench::doNotOptimizeAway(
                test_val = GetRpclibClient()
                               .call(func_name, std::forward<Args>(args)...)
                               .template as<T>());
        });

    if constexpr (std::is_floating_point_v<T>)
    {
        REQUIRE(test_val == doctest::Approx(expected));
    }
    else
    {
        REQUIRE(test_val == expected);
    }
#endif
}

#if defined(RPC_HPP_BENCH_GRPC)
template<typename T, typename F, typename... Args>
void bench_grpc(nanobench::Bench& bench, const T& expected, F member_func, Args&&... args)
{
    T test_val{};

    bench.run("gRPC",
        [&]
        {
            nanobench::doNotOptimizeAway(
                test_val = std::invoke(member_func, GetGrpcClient(), std::forward<Args>(args)...));
        });

    if constexpr (std::is_floating_point_v<T>)
    {
        REQUIRE(test_val == doctest::Approx(expected));
    }
    else
    {
        REQUIRE(test_val == expected);
    }
}
#endif

TEST_CASE("By Value (simple)")
{
    static constexpr uint64_t expected = 10946;
    static constexpr uint64_t input = 20;

    nanobench::Bench b;
    b.title("By Value (simple)").warmup(1).relative(true).minEpochIterations(2'000);
    bench_rpc<uint64_t>(b, expected, "Fibonacci", input);

#if defined(RPC_HPP_BENCH_GRPC)
    bench_grpc(b, expected, &gRPC_Client::Fibonacci, input);
#endif
}

TEST_CASE("By Value (complex)")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    const ComplexObject cx{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    nanobench::Bench b;
    b.title("By Value (complex)").warmup(1).relative(true).minEpochIterations(2'000);
    bench_rpc<std::string>(b, expected, "HashComplex", cx);

#if defined(RPC_HPP_BENCH_GRPC)
    bench_grpc(b, expected, &gRPC_Client::HashComplex, cx);
#endif
}

TEST_CASE("By Value (many)")
{
    static constexpr double expected = 3313.695594785;

    nanobench::Bench b;
    b.title("By Value (many)").warmup(1).relative(true).minEpochIterations(2'000);

    bench_rpc<double>(b, expected, "StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
        9783.49, 125.12, 553.3333333333, 2266.1);

#if defined(RPC_HPP_BENCH_GRPC)
    bench_grpc(b, expected, &gRPC_Client::StdDev, 55.65, 125.325, 552.125, 12.767, 2599.6,
        1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
#endif
}

TEST_CASE("With Container")
{
    static constexpr double expected = 1731.8635996333;
    const std::vector<double> input{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    nanobench::Bench b;
    b.title("With Container").warmup(1).relative(true).minEpochIterations(2'000);
    bench_rpc<double>(b, expected, "AverageContainer<double>", input);

#if defined(RPC_HPP_BENCH_GRPC)
    bench_grpc(b, expected, &gRPC_Client::AverageContainer_double, input);
#endif
}

TEST_CASE("Sequential")
{
    static constexpr uint64_t min_num = 5;
    static constexpr uint64_t max_num = 30;
    static constexpr size_t num_rands = 1'000;

    nanobench::Bench b;
    b.title("Sequential").warmup(1).relative(true).minEpochIterations(3);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            auto vec = GetClient<njson_adapter>().template call_func<std::vector<uint64_t>>(
                "GenRandInts", min_num, max_num, num_rands);

            for (auto& val : vec)
            {
                val = GetClient<njson_adapter>().template call_func<uint64_t>("Fibonacci", val);
            }

            nanobench::doNotOptimizeAway(GetClient<njson_adapter>().template call_func<double>(
                "AverageContainer<uint64_t>", vec));
        });

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            auto vec = GetClient<rapidjson_adapter>().template call_func<std::vector<uint64_t>>(
                "GenRandInts", min_num, max_num, num_rands);

            for (auto& val : vec)
            {
                val = GetClient<rapidjson_adapter>().template call_func<uint64_t>("Fibonacci", val);
            }

            nanobench::doNotOptimizeAway(GetClient<rapidjson_adapter>().template call_func<double>(
                "AverageContainer<uint64_t>", vec));
        });
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            auto vec = GetClient<boost_json_adapter>().template call_func<std::vector<uint64_t>>(
                "GenRandInts", min_num, max_num, num_rands);

            for (auto& val : vec)
            {
                val =
                    GetClient<boost_json_adapter>().template call_func<uint64_t>("Fibonacci", val);
            }

            nanobench::doNotOptimizeAway(GetClient<boost_json_adapter>().template call_func<double>(
                "AverageContainer<uint64_t>", vec));
        });
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            auto vec = GetClient<bitsery_adapter>().template call_func<std::vector<uint64_t>>(
                "GenRandInts", min_num, max_num, num_rands);

            for (auto& val : vec)
            {
                val = GetClient<bitsery_adapter>().template call_func<uint64_t>("Fibonacci", val);
            }

            nanobench::doNotOptimizeAway(GetClient<bitsery_adapter>().template call_func<double>(
                "AverageContainer<uint64_t>", vec));
        });
#endif

#if defined(RPC_HPP_BENCH_RPCLIB)
    b.run("rpclib",
        [&]
        {
            auto vec = GetRpclibClient()
                           .call("GenRandInts", min_num, max_num, num_rands)
                           .as<std::vector<uint64_t>>();

            for (auto& val : vec)
            {
                val = GetRpclibClient().call("Fibonacci", val).as<uint64_t>();
            }

            nanobench::doNotOptimizeAway(
                GetRpclibClient().call("AverageContainer<uint64_t>", vec).as<double>());
        });
#endif

#if defined(RPC_HPP_BENCH_GRPC)
    b.run("gRPC",
        [&]
        {
            auto vec = GetGrpcClient().GenRandInts(min_num, max_num, num_rands);

            for (auto& val : vec)
            {
                val = GetGrpcClient().Fibonacci(val);
            }

            nanobench::doNotOptimizeAway(GetGrpcClient().AverageContainer_uint64(vec));
        });
#endif
}

TEST_CASE("KillServer")
{
#if defined(RPC_HPP_BENCH_RPCLIB)
    try
    {
        GetRpclibClient().async_call("KillServer");
    }
    catch (...)
    {
        // Exception is expected so continue
    }
#endif

#if defined(RPC_HPP_BENCH_GRPC)
    try
    {
        GetGrpcClient().KillServer();
    }
    catch (...)
    {
        // Exception is expected so continue
    }
#endif

    auto& client = GetClient<njson_adapter>();

    try
    {
        client.call_func("KillServer");
    }
    catch (...)
    {
        // Exception is expected so continue
    }
}
