#define RPC_HPP_ASSERT_THROW

#include <doctest/doctest.h>
#include <rpc.hpp>

#if defined(RPC_HPP_ENABLE_BITSERY)
#  include <rpc_adapters/rpc_bitsery.hpp>

constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_func_name_size = 30;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_string_size = 2'048;
constexpr size_t rpc_hpp::adapters::bitsery_adapter::config::max_container_size = 1'000;
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  include <rpc_adapters/rpc_boost_json.hpp>
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#  include <rpc_adapters/rpc_njson.hpp>
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  include <rpc_adapters/rpc_rapidjson.hpp>
#endif

// TODO: Clean this up somehow
#if defined(RPC_HPP_ENABLE_BITSERY)
#  if defined(TEST_USE_COMMA)
#    define TEST_BITSERY_T , rpc_hpp::adapters::bitsery_adapter
#  else
#    define TEST_BITSERY_T rpc_hpp::adapters::bitsery_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_BITSERY_T
#endif

#if defined(RPC_HPP_ENABLE_BOOST_JSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_BOOST_JSON_T , rpc_hpp::adapters::boost_json_adapter
#  else
#    define TEST_BOOST_JSON_T rpc_hpp::adapters::boost_json_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_BOOST_JSON_T
#endif

#if defined(RPC_HPP_ENABLE_NJSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_NJSON_T , rpc_hpp::adapters::njson_adapter
#  else
#    define TEST_NJSON_T rpc_hpp::adapters::njson_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_NJSON_T
#endif

#if defined(RPC_HPP_ENABLE_RAPIDJSON)
#  if defined(TEST_USE_COMMA)
#    define TEST_RAPIDJSON_T , rpc_hpp::adapters::rapidjson_adapter
#  else
#    define TEST_RAPIDJSON_T rpc_hpp::adapters::rapidjson_adapter
#    define TEST_USE_COMMA
#  endif
#else
#  define TEST_RAPIDJSON_T
#endif

#if !defined(TEST_USE_COMMA)
#  error At least one adapter must be enabled for testing
#endif

#define RPC_TEST_TYPES TEST_BITSERY_T TEST_BOOST_JSON_T TEST_NJSON_T TEST_RAPIDJSON_T

namespace rpc_hpp::constexpr_tests
{
static_assert(!validate_rpc_type(static_cast<rpc_type>(20)), "validate_rpc_type failed");

static_assert(!validate_rpc_type(static_cast<rpc_type>(-1)), "validate_rpc_type failed");

static_assert(validate_rpc_type(static_cast<rpc_type>(5)), "validate_rpc_type failed");

static_assert(validate_rpc_type(rpc_type::func_error), "validate_rpc_type failed");
} //namespace rpc_hpp::constexpr_tests

namespace rpc_hpp::tests
{
TEST_CASE_TEMPLATE("rpc_object::rpc_object(rpc_result)", TestType, RPC_TEST_TYPES)
{
    detail::func_result<std::string> result{ "test_func", "hello, world!" };
    rpc_object<TestType> obj1{ result };

    REQUIRE(obj1.get_type() == rpc_type::func_result);
    REQUIRE(obj1.get_func_name() == "test_func");
    REQUIRE_FALSE(result.result.empty());

    rpc_object<TestType> obj2{ std::move(result) };
    REQUIRE(obj2.get_type() == rpc_type::func_result);
    REQUIRE(obj2.get_func_name() == "test_func");
    REQUIRE(result.result.empty());
}

TEST_CASE("rpc_object::rpc_object(rpc_request)") {}

TEST_CASE("rpc_object::rpc_object(rpc_error)") {}

TEST_CASE("rpc_object::rpc_object(rpc_result_w_bind)") {}

TEST_CASE("rpc_object::rpc_object(callback_install_request)") {}

TEST_CASE_TEMPLATE("rpc_object::parse_bytes", TestType, RPC_TEST_TYPES)
{
#if defined(RPC_HPP_ENABLE_BITSERY)
    if constexpr (!std::is_same_v<TestType, adapters::bitsery_adapter>)
    {
#endif
        const auto opt_obj = rpc_object<TestType>::parse_bytes(
            R"({ "bind_args": false, "type": 2, "func_name": "test_func", "args": [1, 2] })");

        REQUIRE(opt_obj.has_value());

        const auto& parsed_obj = opt_obj.value();
        REQUIRE_FALSE(parsed_obj.is_error());
        REQUIRE(parsed_obj.get_func_name() == "test_func");
        REQUIRE(parsed_obj.get_type() == rpc_type::callback_request);
        REQUIRE_FALSE(parsed_obj.has_bound_args());

        const auto args = parsed_obj.template get_args<int, int>();
        REQUIRE(std::get<0>(args) == 1);
        REQUIRE(std::get<1>(args) == 2);

        // missing func_name
        {
            const auto bad_obj = rpc_object<TestType>::parse_bytes(
                R"({ "bind_args": false, "type": 2, "args": [1, 2] })");

            REQUIRE_FALSE(bad_obj.has_value());
        }

        // empty func_name
        {
            const auto bad_obj = rpc_object<TestType>::parse_bytes(
                R"({ "bind_args": false, "type": 2, "func_name": "", "args": [1, 2] })");

            REQUIRE_FALSE(bad_obj.has_value());
        }

        // missing type
        {
            const auto bad_obj = rpc_object<TestType>::parse_bytes(
                R"({ "bind_args": false, "func_name": "test_func", "args": [1, 2] })");

            REQUIRE_FALSE(bad_obj.has_value());
        }

        // bad type
        {
            const auto bad_obj = rpc_object<TestType>::parse_bytes(
                R"({ "bind_args": false, "type": 22, "func_name": "test_func", "args": [1, 2] })");

            REQUIRE_FALSE(bad_obj.has_value());
        }
#if defined(RPC_HPP_ENABLE_BITSERY)
    }
#endif
}

TEST_CASE("rpc_object::to_bytes") {}

TEST_CASE_TEMPLATE("rpc_object::get_func_name", TestType, RPC_TEST_TYPES)
{
    const auto fname = rpc_object<TestType>{
        detail::func_result<std::string>{ "test_func", "hello, world!" }
    }.get_func_name();

    REQUIRE_FALSE(fname.empty());
    REQUIRE_EQ(fname, "test_func");
}

TEST_CASE("rpc_object::get_result") {}

TEST_CASE("rpc_object::get_args") {}

TEST_CASE("rpc_object::is_callback_uninstall") {}

TEST_CASE("rpc_object::get_error_type") {}

TEST_CASE("rpc_object::get_error_mesg") {}

TEST_CASE("rpc_object::has_bound_args") {}

TEST_CASE("rpc_object::is_error") {}

TEST_CASE_TEMPLATE("rpc_object::get_type", TestType, RPC_TEST_TYPES)
{
    rpc_type type = rpc_object<TestType>{ callback_install_request{ "test_func" } }.get_type();
    REQUIRE(type == rpc_type::callback_install_request);

    type = rpc_object<TestType>{
        detail::callback_error{ "", exception_type::callback_install, "" }
    }.get_type();
    REQUIRE(type == rpc_type::callback_error);

    type = rpc_object<TestType>{ detail::callback_request<>{ "test_func", {} } }.get_type();
    REQUIRE(type == rpc_type::callback_request);

    type = rpc_object<TestType>{ detail::callback_result<void>{ "test_func" } }.get_type();
    REQUIRE(type == rpc_type::callback_result);

    type =
        rpc_object<TestType>{ detail::callback_result_w_bind<void>{ "test_func", {} } }.get_type();
    REQUIRE(type == rpc_type::callback_result_w_bind);

    type = rpc_object<TestType>{
        detail::func_error{ "", exception_type::func_signature_mismatch, "" }
    }.get_type();
    REQUIRE(type == rpc_type::func_error);

    type = rpc_object<TestType>{ detail::func_request<>{ "test_func", {} } }.get_type();
    REQUIRE(type == rpc_type::func_request);

    type = rpc_object<TestType>{ detail::func_result<void>{ "test_func" } }.get_type();
    REQUIRE(type == rpc_type::func_result);

    type = rpc_object<TestType>{ detail::func_result_w_bind<void>{ "test_func", {} } }.get_type();
    REQUIRE(type == rpc_type::func_result_w_bind);
}
} //namespace rpc_hpp::tests
