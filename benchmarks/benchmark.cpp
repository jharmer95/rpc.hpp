#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#define RPC_HPP_CLIENT_IMPL

#include "client.hpp"
#include "test_structs.hpp"

#include <catch2/catch.hpp>

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#    include <rpc_adapters/rpc_boost_json.hpp>

using rpc::adapters::bjson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#    include <rpc_adapters/rpc_njson.hpp>

using rpc::adapters::njson_adapter;
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#    include <rpc_adapters/rpc_rapidjson.hpp>

using rpc::adapters::rapidjson_adapter;
#endif

template<typename Serial>
TestClient<Serial>& GetClient();

template<>
TestClient<njson_adapter>& GetClient()
{
    static TestClient<njson_adapter> client("127.0.0.1", "5000");
    return client;
}

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
template<>
TestClient<rapidjson_adapter>& GetClient()
{
    static TestClient<rapidjson_adapter> client("127.0.0.1", "5001");
    return client;
}
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
template<>
TestClient<bjson_adapter>& GetClient()
{
    static TestClient<bjson_adapter> client("127.0.0.1", "5002");
    return client;
}
#endif

TEST_CASE("By Value (simple)", "[value][simple][cached]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        test = GetClient<njson_adapter>()
                   .template call_func<uint64_t>("Fibonacci", 20)
                   .get_result()
                   .value();
    };

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = GetClient<rapidjson_adapter>()
                   .template call_func<uint64_t>("Fibonacci", 20)
                   .get_result()
                   .value();
    };

    REQUIRE(expected == test);
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = GetClient<bjson_adapter>()
                   .template call_func<uint64_t>("Fibonacci", 20)
                   .get_result()
                   .value();
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

        test = GetClient<njson_adapter>()
                   .template call_func<std::string>("HashComplex", cx)
                   .get_result()
                   .value();
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

        test = GetClient<rapidjson_adapter>()
                   .template call_func<std::string>("HashComplex", cx)
                   .get_result()
                   .value();
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

        test = GetClient<bjson_adapter>()
                   .template call_func<std::string>("HashComplex", cx)
                   .get_result()
                   .value();
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
        test = GetClient<njson_adapter>()
                   .template call_func<double>("StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6,
                       1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                   .get_result()
                   .value();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        test = GetClient<rapidjson_adapter>()
                   .template call_func<double>("StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6,
                       1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                   .get_result()
                   .value();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        test = GetClient<bjson_adapter>()
                   .template call_func<double>("StdDev", 55.65, 125.325, 552.125, 12.767, 2599.6,
                       1245.125663, 9783.49, 125.12, 553.3333333333, 2266.1)
                   .get_result()
                   .value();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinRel(expected));
#endif
}

TEST_CASE("By Reference (simple)", "[ref][simple]")
{
    constexpr uint64_t expected = 10946ULL;
    uint64_t test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        uint64_t num = 20;
        test = GetClient<njson_adapter>().call_func("FibonacciRef", num).template get_arg<0>();
    };

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        uint64_t num = 20;
        test = GetClient<rapidjson_adapter>().call_func("FibonacciRef", num).template get_arg<0>();
    };
#endif

    REQUIRE(expected == test);

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        uint64_t num = 20;
        test = GetClient<bjson_adapter>().call_func("FibonacciRef", num).template get_arg<0>();
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
        ComplexObject cx;
        cx.flag1 = false;
        cx.flag2 = true;
        cx.id = 24;
        cx.name = "Franklin D. Roosevelt";
        cx.vals = { 0, 1, 4, 6, 7, 8, 11, 15, 17, 22, 25, 26 };

        test =
            GetClient<njson_adapter>().call_func("HashComplexRef", cx, test).template get_arg<1>();
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

        test = GetClient<rapidjson_adapter>()
                   .call_func("HashComplexRef", cx, test)
                   .template get_arg<1>();
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

        test =
            GetClient<bjson_adapter>().call_func("HashComplexRef", cx, test).template get_arg<1>();
    };

    REQUIRE_THAT(expected, Catch::Matchers::Equals(test));
#endif
}

TEST_CASE("By Reference (many)", "[ref][many]")
{
    constexpr double expected = 313.2216436152;
    double test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
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

        const auto pack = GetClient<njson_adapter>().call_func(
            "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

        n1 = pack.template get_arg<0>();
        n2 = pack.template get_arg<1>();
        n3 = pack.template get_arg<2>();
        n4 = pack.template get_arg<3>();
        n5 = pack.template get_arg<4>();
        n6 = pack.template get_arg<5>();
        n7 = pack.template get_arg<6>();
        n8 = pack.template get_arg<7>();
        n9 = pack.template get_arg<8>();
        n10 = pack.template get_arg<9>();
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
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

        const auto pack = GetClient<rapidjson_adapter>().call_func(
            "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

        n1 = pack.template get_arg<0>();
        n2 = pack.template get_arg<1>();
        n3 = pack.template get_arg<2>();
        n4 = pack.template get_arg<3>();
        n5 = pack.template get_arg<4>();
        n6 = pack.template get_arg<5>();
        n7 = pack.template get_arg<6>();
        n8 = pack.template get_arg<7>();
        n9 = pack.template get_arg<8>();
        n10 = pack.template get_arg<9>();
        test = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10;
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
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

        const auto pack = GetClient<bjson_adapter>().call_func(
            "SquareRootRef", n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

        n1 = pack.template get_arg<0>();
        n2 = pack.template get_arg<1>();
        n3 = pack.template get_arg<2>();
        n4 = pack.template get_arg<3>();
        n5 = pack.template get_arg<4>();
        n6 = pack.template get_arg<5>();
        n7 = pack.template get_arg<6>();
        n8 = pack.template get_arg<7>();
        n9 = pack.template get_arg<8>();
        n10 = pack.template get_arg<9>();
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

        test = GetClient<njson_adapter>()
                   .template call_func<double>("AverageContainer<double>", vec)
                   .get_result()
                   .value();
    };

    REQUIRE_THAT(test, Catch::Matchers::WithinAbs(expected, 0.001));

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = GetClient<rapidjson_adapter>()
                   .template call_func<double>("AverageContainer<double>", vec)
                   .get_result()
                   .value();
    };
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    test = 1.0;

    BENCHMARK("rpc.hpp (asio::tcp, Boost.JSON)")
    {
        const std::vector<double> vec{ 55.65, 125.325, 552.125, 12.767, 2599.6, 1245.125663,
            9783.49, 125.12, 553.3333333333, 2266.1 };

        test = GetClient<bjson_adapter>()
                   .template call_func<double>("AverageContainer<double>", vec)
                   .get_result()
                   .value();
    };
#endif
}

TEST_CASE("Sequential", "[sequential][cached]")
{
    BENCHMARK("rpc.hpp (asio::tcp, njson)")
    {
        auto vec = GetClient<njson_adapter>()
                       .template call_func<std::vector<uint64_t>>("RandInt", 5, 30, 1000)
                       .get_result()
                       .value();

        for (auto& val : vec)
        {
            val = GetClient<njson_adapter>()
                      .template call_func<uint64_t>("Fibonacci", val)
                      .get_result()
                      .value();
        }

        return GetClient<njson_adapter>()
            .template call_func<double>("AverageContainer<uint64_t>", vec)
            .get_result()
            .value();
    };

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
    BENCHMARK("rpc.hpp (asio::tcp, rapidjson)")
    {
        auto vec = GetClient<rapidjson_adapter>()
                       .template call_func<std::vector<uint64_t>>("RandInt", 5, 30, 1000)
                       .get_result()
                       .value();

        for (auto& val : vec)
        {
            val = GetClient<rapidjson_adapter>()
                      .template call_func<uint64_t>("Fibonacci", val)
                      .get_result()
                      .value();
        }

        return GetClient<rapidjson_adapter>()
            .template call_func<double>("AverageContainer<uint64_t>", vec)
            .get_result()
            .value();
    };
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
    BENCHMARK("rpc.hpp (asio::tcp, bjson)")
    {
        auto vec = GetClient<bjson_adapter>()
                       .template call_func<std::vector<uint64_t>>("RandInt", 5, 30, 1000)
                       .get_result()
                       .value();

        for (auto& val : vec)
        {
            val = GetClient<bjson_adapter>()
                      .template call_func<uint64_t>("Fibonacci", val)
                      .get_result()
                      .value();
        }

        return GetClient<bjson_adapter>()
            .template call_func<double>("AverageContainer<uint64_t>", vec)
            .get_result()
            .value();
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
