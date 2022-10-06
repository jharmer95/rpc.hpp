#include <doctest/doctest.h>
#include <rpc.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace rpc_hpp::tests
{
TEST_CASE("for_each_tuple")
{
    const auto process = [](auto&& val)
    {
        using T = detail::remove_cvref_t<decltype(val)>;

        if constexpr (std::is_same_v<T, int>)
        {
            REQUIRE(val > 14);
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            REQUIRE(val > 0.0f);
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            REQUIRE(val[1] == 'e');
        }
        else if constexpr (std::is_same_v<T, std::vector<unsigned>>)
        {
            REQUIRE(val.size() == 4);
        }
        else
        {
            REQUIRE(false);
        }
    };

    std::tuple<int, std::string, float, std::vector<unsigned>> tup{ 15, "Hello, world!", 1.34f,
        { 16, 166, 886, 4 } };

    detail::for_each_tuple(tup, process);
}

template<typename... Args>
void helper(Args&&... args)
{
    auto tup = std::forward_as_tuple(args...);

    const auto process = [](auto&& val)
    {
        using T = std::remove_reference_t<decltype(val)>;

        if constexpr (std::is_same_v<T, int>)
        {
            std::forward<decltype(val)>(val) += 1;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            std::forward<decltype(val)>(val) = -1.0f;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            std::forward<decltype(val)>(val) = "Alabama";
        }
        else
        {
        }
    };

    detail::for_each_tuple(tup, process);
    detail::tuple_bind(tup, args...);
}

TEST_CASE("tuple_bind")
{
    int x{ 0 };
    float y{ -1.0f };
    std::string s{ "Bad value" };

    helper(x, y, s);

    REQUIRE(x == 1);
    REQUIRE(y < 0.0f);
    REQUIRE(s == "Alabama");

    const std::string s2{ "const value" };

    helper(x, y, s2);

    REQUIRE(x == 2);
    REQUIRE(y < 0.0f);
    REQUIRE(s2 == "const value");
}
} //namespace rpc_hpp::tests
