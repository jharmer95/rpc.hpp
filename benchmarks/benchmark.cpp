#define RPC_HPP_CLIENT_IMPL
#include "test_client/rpc.client.hpp"
#include "test_structs.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#if defined(RPC_HPP_ENABLE_BITSERY)
const uint64_t rpc::adapters::bitsery::config::max_func_name_size = 30;
const uint64_t rpc::adapters::bitsery::config::max_string_size = 2048;
const uint64_t rpc::adapters::bitsery::config::max_container_size = 1'000;
#endif

TEST_CASE("By Value (simple)")
{
    static constexpr uint64_t expected = 10946UL;
    static constexpr uint64_t input = 20;
    uint64_t test = 1;

    ankerl::nanobench::Bench b;
    b.title("By Value (simple)").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        { test = GetClient<njson_adapter>().template call_func<uint64_t>("Fibonacci", input); });

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1;

    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&] {
            test = GetClient<rapidjson_adapter>().template call_func<uint64_t>("Fibonacci", input);
        });

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1;

    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&] {
            test = GetClient<boost_json_adapter>().template call_func<uint64_t>("Fibonacci", input);
        });

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = 1;

    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        { test = GetClient<bitsery_adapter>().template call_func<uint64_t>("Fibonacci", input); });

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Value (complex)")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;

    const ComplexObject cx{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    ankerl::nanobench::Bench b;
    b.title("By Value (complex)").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        { test = GetClient<njson_adapter>().template call_func<std::string>("HashComplex", cx); });

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = "";

    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&] {
            test =
                GetClient<rapidjson_adapter>().template call_func<std::string>("HashComplex", cx);
        });

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = "";

    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&] {
            test =
                GetClient<boost_json_adapter>().template call_func<std::string>("HashComplex", cx);
        });

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = "";

    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&] {
            test = GetClient<bitsery_adapter>().template call_func<std::string>("HashComplex", cx);
        });

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Value (many)")
{
    static constexpr double expected = 3313.695594785;
    double test = 1.0;

    ankerl::nanobench::Bench b;
    b.title("By Value (many)").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            test = GetClient<njson_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
                552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
        });

    REQUIRE(test == doctest::Approx(expected));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            test =
                GetClient<rapidjson_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
                    552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
        });

    REQUIRE(test == doctest::Approx(expected));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            test =
                GetClient<boost_json_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
                    552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
        });

    REQUIRE(test == doctest::Approx(expected));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = 1.0;

    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            test = GetClient<bitsery_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
                552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
        });

    REQUIRE(test == doctest::Approx(expected));
#endif
}

TEST_CASE("By Reference (simple)")
{
    static constexpr uint64_t expected = 10946UL;
    static constexpr uint64_t input = 20;
    uint64_t test{};

    ankerl::nanobench::Bench b;
    b.title("By Reference (simple)").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            test = input;
            GetClient<njson_adapter>().call_func("FibonacciRef", test);
        });

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            test = input;
            GetClient<rapidjson_adapter>().call_func("FibonacciRef", test);
        });
#endif

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            test = input;
            GetClient<boost_json_adapter>().call_func("FibonacciRef", test);
        });

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            test = input;
            GetClient<bitsery_adapter>().call_func("FibonacciRef", test);
        });

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Reference (complex)")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    const ComplexObject input{ 24, "Franklin D. Roosevelt", false, true,
        { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 } };

    ComplexObject cx{};
    std::string test;

    ankerl::nanobench::Bench b;
    b.title("By Reference (complex)").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            test.clear();
            cx = input;

            GetClient<njson_adapter>().call_func("HashComplexRef", cx, test);
        });

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            test.clear();
            cx = input;

            GetClient<rapidjson_adapter>().call_func("HashComplexRef", cx, test);
        });

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            test.clear();
            cx = input;

            GetClient<boost_json_adapter>().call_func("HashComplexRef", cx, test);
        });

    REQUIRE_THAT(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            test.clear();
            cx = input;

            GetClient<bitsery_adapter>().call_func("HashComplexRef", cx, test);
        });

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Reference (many)")
{
    static constexpr double expected = 313.2216436152;
    double test{};

    ankerl::nanobench::Bench b;
    b.title("By Reference (many)").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            test = 1.0;
            double n1 = 55.65;
            double n2 = 125.325;
            double n3 = 552.125;
            double n4 = 12.767;
            double n5 = 2599.6;
            double n6 = 1245.125663;
            double n7 = 9783.49;
            double n8 = 125.12;
            double n9 = 553.3333333333;
            double n10 = 2266.1;

            GetClient<njson_adapter>().call_func(
                "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

            test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            test = 1.0;
            double n1 = 55.65;
            double n2 = 125.325;
            double n3 = 552.125;
            double n4 = 12.767;
            double n5 = 2599.6;
            double n6 = 1245.125663;
            double n7 = 9783.49;
            double n8 = 125.12;
            double n9 = 553.3333333333;
            double n10 = 2266.1;

            GetClient<rapidjson_adapter>().call_func(
                "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

            test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            test = 1.0;
            double n1 = 55.65;
            double n2 = 125.325;
            double n3 = 552.125;
            double n4 = 12.767;
            double n5 = 2599.6;
            double n6 = 1245.125663;
            double n7 = 9783.49;
            double n8 = 125.12;
            double n9 = 553.3333333333;
            double n10 = 2266.1;

            GetClient<boost_json_adapter>().call_func(
                "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

            test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            test = 1.0;
            double n1 = 55.65;
            double n2 = 125.325;
            double n3 = 552.125;
            double n4 = 12.767;
            double n5 = 2599.6;
            double n6 = 1245.125663;
            double n7 = 9783.49;
            double n8 = 125.12;
            double n9 = 553.3333333333;
            double n10 = 2266.1;

            GetClient<bitsery_adapter>().call_func(
                "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

            test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
#endif
}

TEST_CASE("With Container")
{
    static constexpr double expected = 1731.8635996333;
    const std::vector<double> input{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663, 9783.49,
        125.12, 553.3333333333, 2266.1 };

    double test = 1.0;

    ankerl::nanobench::Bench b;
    b.title("With Container").warmup(1).relative(true).minEpochIterations(1'000);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            const auto& vec = input;
            test = GetClient<njson_adapter>().template call_func<double>(
                "AverageContainer<double>", vec);
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    b.run("rpc.hpp (asio::tcp, rapidjson)",
        [&]
        {
            const auto& vec = input;
            test = GetClient<rapidjson_adapter>().template call_func<double>(
                "AverageContainer<double>", vec);
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    b.run("rpc.hpp (asio::tcp, Boost.JSON)",
        [&]
        {
            const auto& vec = input;
            test = GetClient<boost_json_adapter>().template call_func<double>(
                "AverageContainer<double>", vec);
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = 1.0;

    b.run("rpc.hpp (asio::tcp, bitsery)",
        [&]
        {
            const auto& vec = input;
            test = GetClient<bitsery_adapter>().template call_func<double>(
                "AverageContainer<double>", vec);
        });

    REQUIRE(test == doctest::Approx(expected).epsilon(0.001));
#endif
}

TEST_CASE("Sequential")
{
    static constexpr uint64_t min_num = 5;
    static constexpr uint64_t max_num = 30;
    static constexpr size_t num_rands = 1'000;

    ankerl::nanobench::Bench b;
    b.title("Sequential").warmup(1).relative(true).minEpochIterations(10);

    b.run("rpc.hpp (asio::tcp, njson)",
        [&]
        {
            auto vec = GetClient<njson_adapter>().template call_func<std::vector<uint64_t>>(
                "GenRandInts", min_num, max_num, num_rands);

            for (auto& val : vec)
            {
                val = GetClient<njson_adapter>().template call_func<uint64_t>("Fibonacci", val);
            }

            ankerl::nanobench::doNotOptimizeAway(
                GetClient<njson_adapter>().template call_func<double>(
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

            ankerl::nanobench::doNotOptimizeAway(
                GetClient<rapidjson_adapter>().template call_func<double>(
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

            ankerl::nanobench::doNotOptimizeAway(
                GetClient<boost_json_adapter>().template call_func<double>(
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

            ankerl::nanobench::doNotOptimizeAway(
                GetClient<bitsery_adapter>().template call_func<double>(
                    "AverageContainer<uint64_t>", vec));
        });
#endif
}

TEST_CASE("KillServer")
{
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
