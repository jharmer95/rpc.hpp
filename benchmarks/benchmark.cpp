#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#define RPC_HPP_CLIENT_IMPL

#include "rpc.client.hpp"
#include "test_structs.hpp"

#include <catch2/catch.hpp>

#if defined(RPC_HPP_ENABLE_BITSERY)
const uint64_t rpc::adapters::bitsery::config::max_func_name_size = 30;
const uint64_t rpc::adapters::bitsery::config::max_string_size = 100;
const uint64_t rpc::adapters::bitsery::config::max_container_size = 100;
#endif

TEST_CASE("By Value (simple)", "[value][simple][cached]")
{
    constexpr uint64_t expected = 10946UL;
    uint64_t test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test = GetClient<njson_adapter>().template call_func<uint64_t>("Fibonacci", 20);
    };

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = GetClient<rapidjson_adapter>().template call_func<uint64_t>("Fibonacci", 20);
    };

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = GetClient<boost_json_adapter>().template call_func<uint64_t>("Fibonacci", 20);
    };

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        test = GetClient<bitsery_adapter>().template call_func<uint64_t>("Fibonacci", 20);
    };

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Value (complex)", "[value][complex][cached]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = GetClient<njson_adapter>().template call_func<std::string>("HashComplex", cx);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = GetClient<rapidjson_adapter>().template call_func<std::string>("HashComplex", cx);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = GetClient<boost_json_adapter>().template call_func<std::string>("HashComplex", cx);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = "";

    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test = GetClient<bitsery_adapter>().template call_func<std::string>("HashComplex", cx);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif
}

TEST_CASE("By Value (many)", "[value][many][cached]")
{
    constexpr double expected = 3313.695594785;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test = GetClient<njson_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
            552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = GetClient<rapidjson_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
            552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = GetClient<boost_json_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
            552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        test = GetClient<bitsery_adapter>().template call_func<double>("StdDev", 55.65, 125.325,
            552.125, 12.767, 2599.6, 1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1);
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif
}

TEST_CASE("By Reference (simple)", "[ref][simple]")
{
    constexpr uint64_t expected = 10946UL;
    uint64_t test = 20;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test = 20;
        GetClient<njson_adapter>().call_func("FibonacciRef", test);
    };

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = 20;
        GetClient<rapidjson_adapter>().call_func("FibonacciRef", test);
    };
#endif

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = 20;
        GetClient<boost_json_adapter>().call_func("FibonacciRef", test);
    };

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        test = 20;
        GetClient<bitsery_adapter>().call_func("FibonacciRef", test);
    };

    REQUIRE(expected == test);
#endif
}

TEST_CASE("By Reference (complex)", "[ref][complex]")
{
    const std::string expected = "467365747274747d315a473a527073796c7e707b85";
    std::string test;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test.clear();
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        GetClient<njson_adapter>().call_func("HashComplexRef", cx, test);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test.clear();
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        GetClient<rapidjson_adapter>().call_func("HashComplexRef", cx, test);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test.clear();
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        GetClient<boost_json_adapter>().call_func("HashComplexRef", cx, test);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        test.clear();
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        GetClient<bitsery_adapter>().call_func("HashComplexRef", cx, test);
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif
}

TEST_CASE("By Reference (many)", "[ref][many]")
{
    constexpr double expected = 313.2216436152;
    double test;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
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
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
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
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
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
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
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
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#endif
}

TEST_CASE("With Container", "[container][cached]")
{
    constexpr double expected = 1731.8635996333;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test =
            GetClient<njson_adapter>().template call_func<double>("AverageContainer<double>", vec);
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = GetClient<rapidjson_adapter>().template call_func<double>(
            "AverageContainer<double>", vec);
    };
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = GetClient<boost_json_adapter>().template call_func<double>(
            "AverageContainer<double>", vec);
    };
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = GetClient<bitsery_adapter>().template call_func<double>(
            "AverageContainer<double>", vec);
    };
#endif
}

TEST_CASE("Sequential", "[sequential][cached]")
{
    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        auto vec = GetClient<njson_adapter>().template call_func<std::vector<uint64_t>>(
            "GenRandInts", 5, 30, 1'000);

        for (auto& val : vec)
        {
            val = GetClient<njson_adapter>().template call_func<uint64_t>("Fibonacci", val);
        }

        return GetClient<njson_adapter>().template call_func<double>(
            "AverageContainer<uint64_t>", vec);
    };

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        auto vec = GetClient<rapidjson_adapter>().template call_func<std::vector<uint64_t>>(
            "GenRandInts", 5, 30, 1'000);

        for (auto& val : vec)
        {
            val = GetClient<rapidjson_adapter>().template call_func<uint64_t>("Fibonacci", val);
        }

        return GetClient<rapidjson_adapter>().template call_func<double>(
            "AverageContainer<uint64_t>", vec);
    };
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    BENCHMARK("rpc.hpp (asio::tcp, bjson)")
    {
        auto vec = GetClient<boost_json_adapter>().template call_func<std::vector<uint64_t>>(
            "GenRandInts", 5, 30, 1'000);

        for (auto& val : vec)
        {
            val = GetClient<boost_json_adapter>().template call_func<uint64_t>("Fibonacci", val);
        }

        return GetClient<boost_json_adapter>().template call_func<double>(
            "AverageContainer<uint64_t>", vec);
    };
#endif

#if defined(RPC_HPP_ENABLE_BITSERY)
    BENCHMARK("rpc.hpp (asio::tcp, bitsery)")
    {
        auto vec = GetClient<bitsery_adapter>().template call_func<std::vector<uint64_t>>(
            "GenRandInts", 5, 30, 1'000);

        for (auto& val : vec)
        {
            val = GetClient<bitsery_adapter>().template call_func<uint64_t>("Fibonacci", val);
        }

        return GetClient<bitsery_adapter>().template call_func<double>(
            "AverageContainer<uint64_t>", vec);
    };
#endif
}

TEST_CASE("KillServer",
    "[!mayfail][value][simple][cached][ref][complex][sequential][pointer][many][container]")
{
    auto& client = GetClient<njson_adapter>();

    try
    {
        client.call_func("KillServer");
    }
    catch (...)
    {
    }
}
