#include <rpc.hpp>

#include <string>
#include <type_traits>

namespace rpc_hpp::constexpr_tests
{
static_assert(std::is_same_v<detail::remove_cvref_t<int&>, int>, "remove_cvref failed");
static_assert(std::is_same_v<detail::remove_cvref_t<const int&>, int>, "remove_cvref failed");
static_assert(std::is_same_v<detail::remove_cvref_t<const int>, int>, "remove_cvref failed");
static_assert(
    std::is_same_v<detail::remove_cvref_t<const volatile int&>, int>, "remove_cvref failed");

static_assert(detail::is_boolean_testable_v<bool>, "is_boolean_testable failed");
static_assert(detail::is_boolean_testable_v<decltype((1 == 2))>, "is_boolean_testable failed");

static_assert(detail::is_stringlike_v<std::string>, "is_stringlike failed");
static_assert(detail::is_stringlike_v<const std::string&>, "is_stringlike failed");
static_assert(detail::is_stringlike_v<std::string&&>, "is_stringlike failed");
static_assert(detail::is_stringlike_v<std::string_view>, "is_stringlike failed");
static_assert(detail::is_stringlike_v<const std::string_view>, "is_stringlike failed");
static_assert(detail::is_stringlike_v<const char*>, "is_stringlike failed");
} //namespace rpc_hpp::constexpr_tests
