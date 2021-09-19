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

#if !defined(RPC_HPP_ENABLE_NJSON)
#    error 'rpc_njson' was included without defining 'RPC_HPP_ENABLE_NJSON' Please define this macro or do not include this header!
#endif

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
            void push_arg(T&& arg, njson_t& arg_arr)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if constexpr (std::is_arithmetic_v<
                                  no_ref_t> || std::is_same_v<no_ref_t, std::string>)
                {
                    arg_arr.push_back(std::forward<T>(arg));
                }
                else if constexpr (rpc::details::is_map_v<no_ref_t>)
                {
                    njson_t obj;
                    njson_t val_arr = njson_t::array();

                    for (const auto& [key, val] : arg)
                    {
                        push_arg(val, val_arr);
                        obj[njson_t{ key }.dump()] = val_arr.back();
                    }

                    arg_arr.push_back(obj);
                }
                else if constexpr (rpc::details::is_multimap_v<no_ref_t>)
                {
                    njson_t obj;
                    njson_t val_arr = njson_t::array();

                    for (const auto& [key, val] : arg)
                    {
                        push_arg(val, val_arr);
                        const auto key_str = njson_t{ key }.dump();

                        if (obj.find(key_str) == obj.end())
                        {
                            obj[key_str] = njson_t::array();
                        }

                        obj[key_str].push_back(val_arr.back());
                    }

                    const std::string sstr = obj.dump();

                    arg_arr.push_back(obj);
                }
                else if constexpr (
                    rpc::details::is_deque_v<
                        no_ref_t> || rpc::details::is_list_v<no_ref_t> || rpc::details::is_forward_list_v<no_ref_t> || rpc::details::is_set_v<no_ref_t> || rpc::details::is_vector_v<no_ref_t>)
                {
                    njson_t arr = njson_t::array();

                    for (auto&& val : arg)
                    {
                        push_arg(std::forward<decltype(val)>(val), arr);
                    }

                    arg_arr.push_back(std::move(arr));
                }
                else if constexpr (rpc::details::is_serializable_v<njson_adapter, no_ref_t>)
                {
                    arg_arr.push_back(
                        no_ref_t::template serialize<njson_adapter>(std::forward<T>(arg)));
                }
                else
                {
                    arg_arr.push_back(
                        njson_adapter::template serialize<no_ref_t>(std::forward<T>(arg)));
                }
            }

            template<typename T>
            std::remove_cv_t<std::remove_reference_t<T>> parse_arg(
                const njson_t& arg_arr, unsigned& index)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                const auto& arg = arg_arr.is_array() ? arg_arr[index++] : arg_arr;

                if constexpr (std::is_arithmetic_v<
                                  no_ref_t> || std::is_same_v<no_ref_t, std::string>)
                {
                    return arg.get<no_ref_t>();
                }
                else if constexpr (rpc::details::is_map_v<no_ref_t>)
                {
                    using key_t = typename no_ref_t::key_type;
                    using value_t = typename no_ref_t::mapped_type;

                    no_ref_t map;

                    for (const auto& [key, val] : arg.items())
                    {
                        unsigned i = 0;
                        map[njson_t::parse(key).front().template get<key_t>()] =
                            parse_arg<value_t>(val, i);
                    }

                    return map;
                }
                else if constexpr (rpc::details::is_multimap_v<no_ref_t>)
                {
                    using key_t = typename no_ref_t::key_type;
                    using value_t = typename no_ref_t::mapped_type;

                    no_ref_t mmap;

                    for (const auto& [key, val] : arg.items())
                    {
                        for (const auto& subval : val)
                        {
                            unsigned i = 0;
                            mmap.insert({ njson_t::parse(key).front().template get<key_t>(),
                                parse_arg<value_t>(subval, i) });
                        }
                    }

                    return mmap;
                }
                else if constexpr (rpc::details::is_set_v<no_ref_t>)
                {
                    using value_t = typename no_ref_t::value_type;

                    no_ref_t container;

                    unsigned j = 0;

                    for (const auto& val : arg)
                    {
                        container.insert(parse_arg<value_t>(val, j));
                    }

                    return container;
                }
                else if constexpr (
                    rpc::details::is_deque_v<
                        no_ref_t> || rpc::details::is_list_v<no_ref_t> || rpc::details::is_vector_v<no_ref_t>)
                {
                    using value_t = typename no_ref_t::value_type;

                    no_ref_t container;
                    container.reserve(arg.size());

                    unsigned j = 0;

                    for (const auto& val : arg)
                    {
                        container.push_back(parse_arg<value_t>(val, j));
                    }

                    return container;
                }
                else if constexpr (rpc::details::is_forward_list_v<no_ref_t>)
                {
                    using value_t = typename no_ref_t::value_type;

                    no_ref_t container;
                    container.reserve(arg.size());

                    unsigned j = 0;

                    for (auto it = arg.rbegin(); it != arg.rend(); ++it)
                    {
                        container.push_front(parse_arg<value_t>(*it, j));
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
        } // namespace details
    }     // namespace njson
} // namespace adapters

template<>
inline std::string adapters::njson_adapter::to_bytes(const adapters::njson::njson_t& serial_obj)
{
    return serial_obj.dump();
}

template<>
inline std::string adapters::njson_adapter::to_bytes(adapters::njson::njson_t&& serial_obj)
{
    return std::move(serial_obj).dump();
}

template<>
inline adapters::njson::njson_t adapters::njson_adapter::from_bytes(const std::string& bytes)
{
    return adapters::njson::njson_t::parse(bytes);
}

template<>
inline adapters::njson::njson_t adapters::njson_adapter::from_bytes(std::string&& bytes)
{
    return adapters::njson::njson_t::parse(std::move(bytes));
}

template<>
template<typename R, typename... Args>
adapters::njson::njson_t pack_adapter<adapters::njson_adapter>::serialize_pack(
    const packed_func<R, Args...>& pack)
{
    using namespace adapters::njson;

    njson_t obj;

    obj["func_name"] = pack.get_func_name();

    auto arg_arr = njson_t::array();

    const auto& argTup = pack.get_args();
    rpc::details::for_each_tuple(argTup,
        [&arg_arr](auto&& x)
        { adapters::njson::details::push_arg(std::forward<decltype(x)>(x), arg_arr); });

    obj["args"] = std::move(arg_arr);

    if (!pack)
    {
        obj["err_mesg"] = pack.get_err_mesg();
    }
    else
    {
        if constexpr (!std::is_void_v<R>)
        {
            obj["result"] = pack.get_result();
        }
    }

    return obj;
}

template<>
template<typename R, typename... Args>
packed_func<R, Args...> pack_adapter<adapters::njson_adapter>::deserialize_pack(
    const adapters::njson::njson_t& serial_obj)
{
    using namespace adapters::njson;

    [[maybe_unused]] unsigned i = 0;

    typename packed_func<R, Args...>::args_t args{ adapters::njson::details::parse_arg<Args>(
        serial_obj["args"], i)... };

    if constexpr (std::is_void_v<R>)
    {
        packed_func<void, Args...> pack(serial_obj["func_name"], std::move(args));

        if (serial_obj.contains("err_mesg"))
        {
            pack.set_err_mesg(serial_obj["err_mesg"]);
        }

        return pack;
    }
    else
    {
        if (serial_obj.contains("result") && !serial_obj["result"].is_null())
        {
            return packed_func<R, Args...>(
                serial_obj["func_name"], serial_obj["result"].get<R>(), std::move(args));
        }

        packed_func<R, Args...> pack(serial_obj["func_name"], std::nullopt, std::move(args));

        if (serial_obj.contains("err_mesg"))
        {
            pack.set_err_mesg(serial_obj["err_mesg"]);
        }

        return pack;
    }
}

template<>
inline std::string pack_adapter<adapters::njson_adapter>::get_func_name(
    const adapters::njson::njson_t& serial_obj)
{
    return serial_obj["func_name"];
}

template<>
inline void pack_adapter<adapters::njson_adapter>::set_err_mesg(
    adapters::njson::njson_t& serial_obj, std::string mesg)
{
    serial_obj["err_mesg"] = std::move(mesg);
}
} // namespace rpc
