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

#include <boost/json.hpp>

#include <cassert>

namespace rpc_hpp
{
namespace adapters
{
    struct boost_json_adapter;

    template<>
    struct serial_traits<boost_json_adapter>
    {
        using serial_t = boost::json::object;
        using bytes_t = std::string;
    };

    struct boost_json_adapter : public detail::serial_adapter_base<boost_json_adapter>
    {
    private:
        template<typename T>
        [[nodiscard]] static constexpr bool validate_arg(const boost::json::value& arg)
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                return arg.is_bool();
            }
            else if constexpr (std::is_integral_v<T>)
            {
                return arg.is_int64() || arg.is_uint64();
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                return arg.is_double();
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return arg.is_string();
            }
            else if constexpr (rpc_hpp::detail::is_container_v<T>)
            {
                return arg.is_array();
            }
            else
            {
                return arg.is_object();
            }
        }

        static std::string mismatch_string(std::string&& expect_type, const boost::json::value& obj)
        {
            std::string type_str = [&obj]
            {
                switch (obj.kind())
                {
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

                    default:
                    case boost::json::kind::null:
                        return "null";
                }
            }();

            return { "Boost.JSON expected type: " + std::move(expect_type)
                + ", got type: " + std::move(type_str) };
        }

        template<typename T>
        static void push_arg(T&& arg, boost::json::value& obj)
        {
            using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

            if constexpr (std::is_arithmetic_v<no_ref_t>)
            {
                obj = arg;
            }
            else if constexpr (std::is_same_v<no_ref_t, std::string>)
            {
                obj = boost::json::string{ arg.c_str() };
            }
            else if constexpr (rpc_hpp::detail::is_container_v<no_ref_t>)
            {
                obj = boost::json::array{};
                auto& arr = obj.get_array();
                arr.reserve(arg.size());

                for (auto&& val : arg)
                {
                    push_args(std::forward<decltype(val)>(val), arr);
                }
            }
            else if constexpr (rpc_hpp::detail::is_serializable_v<boost_json_adapter, no_ref_t>)
            {
                obj = no_ref_t::template serialize<boost_json_adapter>(std::forward<T>(arg));
            }
            else
            {
                obj = serialize<no_ref_t>(std::forward<T>(arg));
            }
        }

        template<typename T>
        static void push_args(T&& arg, boost::json::array& obj_arr)
        {
            boost::json::value tmp{};
            push_arg(std::forward<T>(arg), tmp);
            obj_arr.push_back(std::move(tmp));
        }

        template<typename T>
        [[nodiscard]] static std::remove_cv_t<std::remove_reference_t<T>> parse_arg(
            const boost::json::value& arg)
        {
            using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

            if (!validate_arg<no_ref_t>(arg))
            {
                throw function_mismatch(mismatch_string(typeid(no_ref_t).name(), arg));
            }

            if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
            {
                return boost::json::value_to<no_ref_t>(arg);
            }
            else if constexpr (rpc_hpp::detail::is_container_v<no_ref_t>)
            {
                using subvalue_t = typename no_ref_t::value_type;

                auto& arr = arg.get_array();
                no_ref_t container{};
                container.reserve(arr.size());
                unsigned arg_counter = 0;

                for (const auto& val : arr)
                {
                    container.push_back(parse_args<subvalue_t>(val, arg_counter));
                }

                return container;
            }
            else if constexpr (rpc_hpp::detail::is_serializable_v<boost_json_adapter, no_ref_t>)
            {
                return no_ref_t::template deserialize<boost_json_adapter>(arg.get_object());
            }
            else
            {
                return deserialize<no_ref_t>(arg.get_object());
            }
        }

        template<typename T>
        [[nodiscard]] static std::remove_cv_t<std::remove_reference_t<T>> parse_args(
            const boost::json::value& arg_arr, unsigned& index)
        {
            if (!arg_arr.is_array())
            {
                return parse_arg<T>(arg_arr);
            }

            const auto& arr = arg_arr.get_array();

            if (index >= arr.size())
            {
                throw function_mismatch("Argument count mismatch");
            }

            return parse_arg<T>(arr[index++]);
        }

    public:
        [[nodiscard]] static std::string to_bytes_impl(boost::json::value&& serial_obj)
        {
            return boost::json::serialize(std::move(serial_obj));
        }

        [[nodiscard]] static std::optional<boost::json::object> from_bytes_impl(std::string&& bytes)
        {
            boost::system::error_code ec;
            boost::json::value val = boost::json::parse(std::move(bytes), ec);

            if (ec)
            {
                return std::nullopt;
            }

            if (!val.is_object())
            {
                return std::nullopt;
            }

            const auto& obj = val.get_object();
            const auto ex_it = obj.find("except_type");

            if (ex_it != obj.end())
            {
                const auto& ex_val = ex_it->value();

                if (!ex_val.is_int64())
                {
                    return std::nullopt;
                }

                if (ex_val.get_int64() != 0 && !obj.contains("err_mesg"))
                {
                    return std::nullopt;
                }

                // Objects with exceptions can be otherwise empty
                return obj;
            }

            const auto fname_it = obj.find("func_name");

            if (fname_it == obj.end())
            {
                return std::nullopt;
            }

            const auto& fname_val = fname_it->value();

            if (!fname_val.is_string() || fname_val.get_string().empty())
            {
                return std::nullopt;
            }

            const auto args_it = obj.find("args");

            if (args_it == obj.end())
            {
                return std::nullopt;
            }

            if (!args_it->value().is_array())
            {
                return std::nullopt;
            }

            return obj;
        }

        static boost::json::object empty_object_impl() { return boost::json::object{}; }

        template<typename R, typename... Args>
        [[nodiscard]] static boost::json::object serialize_pack_impl(
            const detail::packed_func<R, Args...>& pack)
        {
            boost::json::object obj{};
            obj["func_name"] = pack.get_func_name();
            auto& args = obj["args"].emplace_array();
            args.reserve(sizeof...(Args));

            const auto& arg_tup = pack.get_args();
            rpc_hpp::detail::for_each_tuple(arg_tup,
                [&args](auto&& elem) { push_args(std::forward<decltype(elem)>(elem), args); });

            if (!pack)
            {
                obj["except_type"] = static_cast<int>(pack.get_except_type());
                obj["err_mesg"] = pack.get_err_mesg();
                return obj;
            }

            if constexpr (!std::is_void_v<R>)
            {
                obj["result"] = {};
                push_arg(pack.get_result(), obj["result"]);
            }

            return obj;
        }

        template<typename R, typename... Args>
        [[nodiscard]] static detail::packed_func<R, Args...> deserialize_pack_impl(
            const boost::json::object& serial_obj)
        {
            auto& args_val = serial_obj.at("args");
            [[maybe_unused]] unsigned arg_counter = 0;

            typename rpc_hpp::detail::packed_func<R, Args...>::args_t args{ parse_args<Args>(
                args_val, arg_counter)... };

            if constexpr (std::is_void_v<R>)
            {
                rpc_hpp::detail::packed_func<void, Args...> pack(
                    serial_obj.at("func_name").get_string().c_str(), std::move(args));

                if (serial_obj.contains("except_type"))
                {
                    pack.set_exception(serial_obj.at("err_mesg").get_string().c_str(),
                        static_cast<exception_type>(serial_obj.at("except_type").get_int64()));
                }

                return pack;
            }
            else
            {
                if (serial_obj.contains("result") && !serial_obj.at("result").is_null())
                {
                    return rpc_hpp::detail::packed_func<R, Args...>(
                        serial_obj.at("func_name").get_string().c_str(),
                        parse_arg<R>(serial_obj.at("result")), std::move(args));
                }

                rpc_hpp::detail::packed_func<R, Args...> pack(
                    serial_obj.at("func_name").get_string().c_str(), std::nullopt, std::move(args));

                if (serial_obj.contains("except_type"))
                {
                    pack.set_exception(serial_obj.at("err_mesg").get_string().c_str(),
                        static_cast<exception_type>(serial_obj.at("except_type").get_int64()));
                }

                return pack;
            }
        }

        [[nodiscard]] static std::string get_func_name_impl(const boost::json::object& serial_obj)
        {
            return { serial_obj.at("func_name").get_string().c_str() };
        }

        static rpc_hpp::rpc_exception extract_exception_impl(const boost::json::object& serial_obj)
        {
            return rpc_hpp::rpc_exception{ serial_obj.at("err_mesg").as_string().c_str(),
                static_cast<rpc_hpp::exception_type>(serial_obj.at("except_type").as_int64()) };
        }

        static void set_exception_impl(boost::json::object& serial_obj, const rpc_exception& ex)
        {
            serial_obj["except_type"] = static_cast<int>(ex.get_type());
            serial_obj["err_mesg"] = boost::json::string{ ex.what() };
        }

        template<typename T>
        static boost::json::object serialize(const T& val);

        template<typename T>
        static T deserialize(const boost::json::object& serial_obj);
    };
} // namespace adapters
} // namespace rpc_hpp
