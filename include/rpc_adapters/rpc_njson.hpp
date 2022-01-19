///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann's JSON library (https://github.com/nlohmann/json)
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

#include <nlohmann/json.hpp>

namespace rpc
{
namespace adapters
{
    using njson_adapter = details::serial_adapter<nlohmann::json, std::string>;

    namespace njson
    {
        using njson_t = nlohmann::json;

        namespace details
        {
            template<typename T>
            [[nodiscard]] constexpr bool validate_arg(const njson_t& arg)
            {
                if constexpr (std::is_same_v<T, bool>)
                {
                    return arg.is_boolean();
                }
                else if constexpr (std::is_integral_v<T>)
                {
                    return arg.is_number() && !arg.is_number_float();
                }
                else if constexpr (std::is_floating_point_v<T>)
                {
                    return arg.is_number_float();
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

            template<typename T>
            void push_args(T&& arg, njson_t& obj_arr);

            template<typename T>
            void push_arg(T&& arg, njson_t& obj)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if constexpr (std::is_arithmetic_v<
                                  no_ref_t> || std::is_same_v<no_ref_t, std::string>)
                {
                    obj = std::forward<T>(arg);
                }
                else if constexpr (rpc::details::is_container_v<no_ref_t>)
                {
                    obj = njson_t::array();
                    obj.get_ref<njson_t::array_t&>().reserve(arg.size());

                    for (auto&& val : arg)
                    {
                        push_args(std::forward<decltype(val)>(val), obj);
                    }
                }
                else if constexpr (rpc::details::is_serializable_v<njson_adapter, no_ref_t>)
                {
                    obj = no_ref_t::template serialize<njson_adapter>(std::forward<T>(arg));
                }
                else
                {
                    obj = njson_adapter::template serialize<no_ref_t>(std::forward<T>(arg));
                }
            }

            template<typename T>
            void push_args(T&& arg, njson_t& obj_arr)
            {
                njson_t tmp{};
                push_arg(std::forward<T>(arg), tmp);
                obj_arr.push_back(std::move(tmp));
            }

            template<typename T>
            std::remove_cv_t<std::remove_reference_t<T>> parse_args(
                const njson_t& arg_arr, unsigned& index);

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_arg(const njson_t& arg)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if (!validate_arg<T>(arg))
                {
                    throw exceptions::function_mismatch(std::string{ "njson expected type: " }
                        + std::string{ typeid(no_ref_t).name() } + std::string{ ", got type: " }
                        + std::string{ arg.type_name() });
                }

                if constexpr (std::is_arithmetic_v<
                                  no_ref_t> || std::is_same_v<no_ref_t, std::string>)
                {
                    return arg.get<no_ref_t>();
                }
                else if constexpr (rpc::details::is_container_v<no_ref_t>)
                {
                    using value_t = typename no_ref_t::value_type;

                    no_ref_t container{};
                    container.reserve(arg.size());

                    unsigned j = 0;

                    for (const auto& val : arg)
                    {
                        container.push_back(parse_args<value_t>(val, j));
                    }

                    return container;
                }
                else if constexpr (rpc::details::is_serializable_v<njson_adapter, no_ref_t>)
                {
                    return no_ref_t::template deserialize<njson_adapter>(arg);
                }
                else
                {
                    return njson_adapter::template deserialize<no_ref_t>(arg);
                }
            }

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_args(
                const njson_t& arg_arr, unsigned& index)
            {
                if (index >= arg_arr.size())
                {
                    throw exceptions::function_mismatch("Argument count mismatch");
                }

                const auto& arg = arg_arr.is_array() ? arg_arr[index++] : arg_arr;
                return parse_arg<T>(arg);
            }
        } // namespace details
    }     // namespace njson
} // namespace adapters

template<>
[[nodiscard]] inline std::string adapters::njson_adapter::to_bytes(
    adapters::njson::njson_t&& serial_obj)
{
    return std::move(serial_obj).dump();
}

template<>
[[nodiscard]] inline adapters::njson::njson_t adapters::njson_adapter::from_bytes(
    std::string&& bytes)
{
    return adapters::njson::njson_t::parse(std::move(bytes));
}

template<>
template<typename R, typename... Args>
[[nodiscard]] adapters::njson::njson_t pack_adapter<adapters::njson_adapter>::serialize_pack(
    const ::rpc::details::packed_func<R, Args...>& pack)
{
    using namespace adapters::njson;

    njson_t obj{};
    obj["func_name"] = pack.get_func_name();
    obj["args"] = njson_t::array();
    auto& arg_arr = obj["args"];
    arg_arr.get_ref<njson_t::array_t&>().reserve(sizeof...(Args));

    const auto& argTup = pack.get_args();
    rpc::details::for_each_tuple(argTup,
        [&arg_arr](auto&& x)
        { adapters::njson::details::push_args(std::forward<decltype(x)>(x), arg_arr); });

    if (!pack)
    {
        obj["except_type"] = pack.get_except_type();
        obj["err_mesg"] = pack.get_err_mesg();
    }
    else
    {
        if constexpr (!std::is_void_v<R>)
        {
            obj["result"] = {};
            adapters::njson::details::push_arg(pack.get_result(), obj["result"]);
        }
    }

    return obj;
}

template<>
template<typename R, typename... Args>
[[nodiscard]] ::rpc::details::packed_func<R, Args...> pack_adapter<
    adapters::njson_adapter>::deserialize_pack(const adapters::njson::njson_t& serial_obj)
{
    using namespace adapters::njson;

    [[maybe_unused]] unsigned i = 0;

    if constexpr (std::is_void_v<R>)
    {
        ::rpc::details::packed_func<void, Args...> pack(serial_obj["func_name"],
            { adapters::njson::details::parse_args<Args>(serial_obj["args"], i)... });

        if (serial_obj.contains("except_type"))
        {
            pack.set_exception(
                serial_obj["err_mesg"], static_cast<exceptions::Type>(serial_obj["except_type"]));
        }

        return pack;
    }
    else
    {
        if (serial_obj.contains("result") && !serial_obj["result"].is_null())
        {
            return ::rpc::details::packed_func<R, Args...>(serial_obj["func_name"],
                adapters::njson::details::parse_arg<R>(serial_obj["result"]),
                { adapters::njson::details::parse_args<Args>(serial_obj["args"], i)... });
        }

        ::rpc::details::packed_func<R, Args...> pack(serial_obj["func_name"], std::nullopt,
            { adapters::njson::details::parse_args<Args>(serial_obj["args"], i)... });

        if (serial_obj.contains("except_type"))
        {
            pack.set_exception(
                serial_obj["err_mesg"], static_cast<exceptions::Type>(serial_obj["except_type"]));
        }

        return pack;
    }
}

template<>
[[nodiscard]] inline std::string pack_adapter<adapters::njson_adapter>::get_func_name(
    const adapters::njson::njson_t& serial_obj)
{
    return serial_obj["func_name"];
}

template<>
inline void pack_adapter<adapters::njson_adapter>::set_exception(
    adapters::njson::njson_t& serial_obj, const rpc::exceptions::rpc_exception& ex)
{
    serial_obj["except_type"] = ex.get_type();
    serial_obj["err_mesg"] = ex.what();
}
} // namespace rpc
