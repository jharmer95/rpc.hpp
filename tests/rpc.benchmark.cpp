
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

std::uint64_t Fibonacci1(std::uint64_t number)
{
    return number < 2 ? 1 : Fibonacci1(number - 1) + Fibonacci1(number - 2);
}

uint64_t Fibonacci2(uint64_t i)
{
    ++i;
    // we'll only use two values on stack,
    // initialized with F(1) and F(2)
    uint64_t a[2] = { 1, 1 };

    // We do not enter loop if initial i was 1 or 2
    while (i-- > 2)
    {
        // A bitwise AND allows switching the storing of the new value
        // from index 0 to index 1.
        a[i & 1] = a[0] + a[1];
    }

    // since the last value of i was 0 (decrementing i),
    // the return value is always in a[0 & 1] => a[0].
    return a[0];
}

TEST_CASE("Fibonacci")
{
    REQUIRE(Fibonacci1(0) == 1);
    REQUIRE(Fibonacci1(5) == 8);
    REQUIRE(Fibonacci2(0) == 1);
    REQUIRE(Fibonacci2(5) == 8);

    BENCHMARK("Fibonacci1 10") { return Fibonacci1(10); };
    BENCHMARK("Fibonacci1 20") { return Fibonacci1(20); };
    //BENCHMARK("Fibonacci1 30") { return Fibonacci1(30); };

    BENCHMARK("Fibonacci2 10") { return Fibonacci2(10); };
    BENCHMARK("Fibonacci2 20") { return Fibonacci2(20); };
    //BENCHMARK("Fibonacci2 30") { return Fibonacci2(30); };
}
