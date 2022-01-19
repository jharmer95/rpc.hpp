///@file rpc_adapters/rpc_boost_json.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting Boost.JSON (https://github.com/boostorg/json)
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2021, Jackson Harmer
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

#pragma once

#include "../rpc.hpp"

#include <cassert>

#include <boost/json/src.hpp>

namespace rpc
{
namespace adapters
{
    using boost_json_adapter = rpc::details::serial_adapter<boost::json::value, std::string>;

    namespace boost_json
    {
        using object_t = boost::json::object;
        using value_t = boost::json::value;

        namespace details
        {
            template<typename T>
            [[nodiscard]] constexpr bool validate_arg(const value_t& arg)
            {
                if constexpr (std::is_same_v<T, bool>)
                {
                    return arg.is_bool();
                }
                else if constexpr (std::is_integral_v<T>)
                {
                    if constexpr (std::is_signed_v<T>)
                    {
                        return arg.is_int64();
                    }
                    else
                    {
                        return arg.is_uint64();
                    }
                }
                else if constexpr (std::is_floating_point_v<T>)
                {
                    return arg.is_double();
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return arg.is_string();
                }
                else if constexpr (rpc::details::is_container_v<T>)
                {
                    return arg.is_array();
                }
                else
                {
                    return !arg.is_null();
                }
            }

            [[noreturn]] inline void throw_mismatch(
                std::string&& expect_type, const value_t& obj) noexcept(false)
            {
                std::string type_str = [&obj]
                {
                    switch (obj.kind())
                    {
                        case boost::json::kind::null:
                            return "null";

                        case boost::json::kind::bool_:
                            return "bool";

                        case boost::json::kind::int64:
                            return "int64";

                        case boost::json::kind::uint64:
                            return "uint64";

                        case boost::json::kind::double_:
                            return "double";

                        case boost::json::kind::string:
                            return "string";

                        case boost::json::kind::array:
                            return "array";

                        case boost::json::kind::object:
                            return "object";
                    }
                }();

                throw exceptions::function_mismatch("Boost.JSON expected type: "
                    + std::move(expect_type) + ", got type: " + std::move(type_str));
            }

            template<typename T>
            void push_args(T&& arg, boost::json::array& obj);

            template<typename T>
            void push_arg(T&& arg, value_t& obj)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if constexpr (std::is_arithmetic_v<no_ref_t>)
                {
                    obj = std::forward<T>(arg);
                }
                else if constexpr (std::is_same_v<no_ref_t, std::string>)
                {
                    obj = boost::json::string{ arg.c_str() };
                }
                else if constexpr (rpc::details::is_container_v<no_ref_t>)
                {
                    obj = boost::json::array{};
                    auto& arr = obj.as_array();
                    arr.reserve(arg.size());

                    for (auto&& val : arg)
                    {
                        push_args(std::forward<decltype(val)>(val), arr);
                    }
                }
                else if constexpr (rpc::details::is_serializable_v<adapters::boost_json_adapter,
                                       no_ref_t>)
                {
                    obj = no_ref_t::template serialize<adapters::boost_json_adapter>(
                        std::forward<T>(arg));
                }
                else
                {
                    obj = adapters::boost_json_adapter::template serialize<no_ref_t>(
                        std::forward<T>(arg));
                }
            }

            template<typename T>
            void push_args(T&& arg, boost::json::array& obj_arr)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                value_t tmp{};
                push_arg(std::forward<T>(arg), tmp);
                obj_arr.push_back(std::move(tmp));
            }

            template<typename T>
            std::remove_cv_t<std::remove_reference_t<T>> parse_args(
                const value_t& arg_arr, unsigned& index);

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_arg(const value_t& arg)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if (!validate_arg<T>(arg))
                {
                    throw_mismatch(typeid(no_ref_t).name(), arg);
                }

                if constexpr (std::is_arithmetic_v<
                                  no_ref_t> || std::is_same_v<no_ref_t, std::string>)
                {
                    return boost::json::value_to<no_ref_t>(arg);
                }
                else if constexpr (rpc::details::is_container_v<no_ref_t>)
                {
                    using subvalue_t = typename no_ref_t::value_type;

                    auto& arr = arg.as_array();
                    no_ref_t container{};
                    container.reserve(arr.size());

                    unsigned j = 0;

                    for (const auto& val : arr)
                    {
                        container.push_back(parse_arg<subvalue_t>(val, j));
                    }

                    return container;
                }
                else if constexpr (rpc::details::is_serializable_v<adapters::boost_json_adapter,
                                       no_ref_t>)
                {
                    return no_ref_t::template deserialize<adapters::boost_json_adapter>(arg);
                }
                else
                {
                    return adapters::boost_json_adapter::template deserialize<no_ref_t>(arg);
                }
            }

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_args(
                const value_t& arg_arr, unsigned& index)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if (index >= arg_arr.as_array().size())
                {
                    throw exceptions::function_mismatch("Argument count mismatch");
                }

                const auto& arg = arg_arr.is_array() ? arg_arr.as_array().at(index++) : arg_arr;
                return parse_arg<T>(arg);
            }
        } // namespace details
    }     // namespace boost_json
} // namespace adapters

template<>
[[nodiscard]] inline std::string adapters::boost_json_adapter::to_bytes(
    adapters::boost_json::value_t&& serial_obj)
{
    return boost::json::serialize(std::move(serial_obj));
}

template<>
[[nodiscard]] inline adapters::boost_json::value_t adapters::boost_json_adapter::from_bytes(
    std::string&& bytes)
{
    return boost::json::parse(std::move(bytes));
}

template<>
template<typename R, typename... Args>
[[nodiscard]] adapters::boost_json::value_t pack_adapter<adapters::boost_json_adapter>::
    serialize_pack(const ::rpc::details::packed_func<R, Args...>& pack)
{
    using namespace adapters::boost_json;

    object_t ret_j{};
    ret_j["func_name"] = pack.get_func_name();

    auto& args = ret_j["args"].emplace_array();
    args.reserve(sizeof...(Args));

    const auto& argTup = pack.get_args();
    rpc::details::for_each_tuple(argTup,
        [&args](auto&& x)
        { adapters::boost_json::details::push_args(std::forward<decltype(x)>(x), args); });

    if (!pack)
    {
        obj["except_type"] = static_cast<int>(pack.get_except_type());
        obj["err_mesg"] = pack.get_err_mesg();
    }
    else
    {
        if constexpr (!std::is_void_v<R>)
        {
            ret_j["result"] = {};
            adapters::boost_json::details::push_arg(pack.get_result(), obj.at("result"));
        }
    }

    return ret_j;
}

template<>
template<typename R, typename... Args>
[[nodiscard]] ::rpc::details::packed_func<R, Args...> pack_adapter<
    adapters::boost_json_adapter>::deserialize_pack(const adapters::boost_json::value_t& serial_obj)
{
    using namespace adapters::boost_json;

    RPC_HPP_PRECONDITION(serial_obj.is_object());

    const auto& obj = serial_obj.get_object();
    [[maybe_unused]] unsigned i = 0;

    auto& args_val = obj.at("args");

    if constexpr (std::is_void_v<R>)
    {
        ::rpc::details::packed_func<void, Args...> pack(obj.at("func_name").get_string().c_str(),
            { adapters::boost_json::details::parse_args<Args>(args_val, i)... });

        if (obj.contains("except_type"))
        {
            pack.set_exception(obj.at("err_mesg").get_string().c_str(),
                static_cast<exceptions::Type>(obj.at("except_type").get_int64()));
        }

        return pack;
    }
    else
    {
        if (obj.contains("result") && !obj.["result"].is_null())
        {
            return ::rpc::details::packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
                adapters::boost_json::details::parse_arg<R>(obj.at("result")),
                { adapters::boost_json::details::parse_args<Args>(obj.at("args"), i)... });
        }

        ::rpc::details::packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
            std::nullopt,
            { adapters::boost_json::details::parse_args<Args>(obj.at("args"), i)... });

        if (obj.contains("except_type"))
        {
            pack.set_exception(obj.at("err_mesg").get_string.c_str(),
                static_cast<exceptions::Type>(obj.at("except_type").get_int64()));
        }

        return pack;
    }
}

template<>
[[nodiscard]] inline std::string pack_adapter<adapters::boost_json_adapter>::get_func_name(
    const adapters::boost_json::value_t& serial_obj)
{
    RPC_HPP_PRECONDITION(serial_obj.is_object());

    return { serial_obj.at("func_name").get_string().c_str() };
}

template<>
inline void pack_adapter<adapters::boost_json_adapter>::set_exception(
    adapters::boost_json::value_t& serial_obj, const rpc::exceptions::rpc_exception& ex)
{
    RPC_HPP_PRECONDITION(serial_obj.is_object());

    auto& obj = serial_obj.get_object();
    obj["except_type"] = static_cast<int>(ex.get_type());
    obj["err_mesg"] = boost::json::string{ ex.what() };
}
} // namespace rpc
