///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.4.0
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

#if !defined(RPC_HPP_ENABLE_JSON) || !defined(RPC_HPP_JSON_USE_NJSON)
#    error 'rpc_njson' was included without defining 'RPC_HPP_ENABLE_JSON' AND 'RPC_HPP_JSON_USE_JSON'! Please define both of these macros or do not include this header!
#else

#    if defined(RPC_HPP_JSON_CHECK)
#        error Only one JSON adapter may be used at a time, please define exactly one of 'RPC_HPP_JSON_USE_XXX'!
#    else
#        define RPC_HPP_JSON_CHECK
#    endif

#    include "../rpc.hpp"

#    include <nlohmann/json.hpp>

namespace rpc
{
namespace adapters
{
    using njson = nlohmann::json;

    template<typename T>
    void push_arg(T arg, njson& arg_arr)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
        {
            arg_arr.push_back(arg);
        }
        else if constexpr (details::is_container_v<no_ref_t>)
        {
            njson arr = njson::array();

            for (const auto& val : arg)
            {
                push_arg(val, arr);
            }

            arg_arr.push_back(arr);
        }
        else
        {
            byte_vec serialized = [&arg]() {
                if constexpr (details::is_serializable_v<serial_t::json, no_ref_t>)
                {
                    return no_ref_t::serialize<serial_t::json>(arg);
                }
                else
                {
                    return serialize<serial_t::json, no_ref_t>(arg);
                }
            }();

            arg_arr.push_back(njson::parse(std::move(serialized)));
        }
    }

    template<typename T>
    std::remove_cv_t<std::remove_reference_t<T>> parse_arg(const njson& arg_arr, unsigned& index)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        const auto& arg = arg_arr.is_array() ? arg_arr[index++] : arg_arr;

        if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
        {
            return arg.get<no_ref_t>();
        }
        else if constexpr (details::is_container_v<no_ref_t>)
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
        else
        {
            auto obj_str = arg.dump();
            byte_vec bytes(obj_str.begin(), obj_str.end());

            if constexpr (details::is_serializable_v<serial_t::json, no_ref_t>)
            {
                return no_ref_t::deserialize<serial_t::json>(std::move(bytes));
            }
            else
            {
                return deserialize<serial_t::json, no_ref_t>(std::move(bytes));
            }
        }
    }
} // namespace adapters

using json_adapter = details::serial_adapter<serial_t::json>;

template<>
template<typename R, typename... Args>
byte_vec json_adapter::serialize_pack(const packed_func<R, Args...>& pack)
{
    using namespace adapters;
    njson obj;

    obj["func_name"] = pack.get_func_name();

    auto arg_arr = njson::array();

    const auto& argTup = pack.get_args();
    for_each_tuple(argTup, [&arg_arr](auto x) { push_arg(x, arg_arr); });

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

    auto obj_str = obj.dump();
    return byte_vec(obj_str.begin(), obj_str.end());
}

template<>
template<typename R, typename... Args>
details::packed_func<R, Args...> json_adapter::deserialize_pack(const byte_vec& bytes)
{
    using namespace adapters;

    const auto obj = njson::parse(bytes);
    unsigned i = 0;

    typename packed_func<R, Args...>::args_t args{ parse_arg<Args>(obj["args"], i)... };

    if constexpr (std::is_void_v<R>)
    {
        packed_func<void, Args...> pack(obj["func_name"], std::move(args));

        if (obj.contains("err_mesg"))
        {
            pack.set_err_mesg(obj["err_mesg"]);
        }

        return pack;
    }
    else
    {
        if (obj.contains("result") && !obj["result"].is_null())
        {
            return packed_func<R, Args...>(
                obj["func_name"], obj["result"].get<R>(), std::move(args));
        }

        packed_func<R, Args...> pack(obj["func_name"], std::nullopt, std::move(args));

        pack.set_err_mesg(obj["err_mesg"]);
        return pack;
    }
}

template<>
inline std::string json_adapter::get_func_name(const byte_vec& bytes)
{
    using namespace adapters;

    const auto obj = njson::parse(bytes);
    return obj["func_name"];
}
} // namespace rpc
#endif
