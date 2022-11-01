#include <doctest/doctest.h>
#include <rpc.hpp>

namespace rpc_hpp::constexpr_tests
{
static_assert(
    !validate_exception_type(static_cast<exception_type>(20)), "validate_exception_type failed");

static_assert(
    !validate_exception_type(static_cast<exception_type>(-1)), "validate_exception_type failed");

static_assert(
    validate_exception_type(static_cast<exception_type>(5)), "validate_exception_type failed");

static_assert(
    validate_exception_type(exception_type::remote_exec), "validate_exception_type failed");
} //namespace rpc_hpp::constexpr_tests
