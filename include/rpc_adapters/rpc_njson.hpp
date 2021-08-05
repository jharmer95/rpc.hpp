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
    using njson = nlohmann::json;
    using njson_adapter = details::serial_adapter<njson, std::string>;

    template<typename T>
    void push_arg(T&& arg, njson& arg_arr)
    {
        using no_ref_t = std::remove_cvref_t<T>;

        if constexpr (details::arithmetic<no_ref_t> || std::same_as<no_ref_t, std::string>)
        {
            arg_arr.push_back(std::forward<T>(arg));
        }
        else if constexpr (details::container<no_ref_t>)
        {
            njson arr = njson::array();

            for (auto&& val : arg)
            {
                push_arg(std::forward<decltype(val)>(val), arr);
            }

            arg_arr.push_back(std::move(arr));
        }
        else if constexpr (details::serializable<no_ref_t, njson_adapter>)
        {
            arg_arr.push_back(no_ref_t::template serialize<njson_adapter>(std::forward<T>(arg)));
        }
        else
        {
            arg_arr.push_back(njson_adapter::template serialize<no_ref_t>(std::forward<T>(arg)));
        }
    }

    template<typename T>
    std::remove_cvref_t<T> parse_arg(const njson& arg_arr, unsigned index)
    {
        using no_ref_t = std::remove_cvref_t<T>;

        const auto& arg = arg_arr.is_array() ? arg_arr[index] : arg_arr;

        if constexpr (details::arithmetic<no_ref_t> || std::same_as<no_ref_t, std::string>)
        {
            return arg.get<no_ref_t>();
        }
        else if constexpr (details::container<no_ref_t>)
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
        else if constexpr (details::serializable<no_ref_t, njson_adapter>)
        {
            return no_ref_t::template deserialize<njson_adapter>(arg);
        }
        else
        {
            return njson_adapter::template deserialize<no_ref_t>(arg);
        }
    }
} // namespace adapters

template<>
inline std::string adapters::njson_adapter::to_bytes(adapters::njson serial_obj)
{
    return std::move(serial_obj).dump();
}

template<>
inline adapters::njson adapters::njson_adapter::from_bytes(std::string bytes)
{
    return adapters::njson::parse(std::move(bytes));
}

template<>
template<typename R, typename... Args>
adapters::njson pack_adapter<adapters::njson_adapter>::serialize_pack(const packed_func<R, Args...>& pack)
{
    using namespace adapters;

    njson obj;

    obj["func_name"] = pack.get_func_name();

    auto arg_arr = njson::array();

    const auto argTup = pack.get_args();
    details::for_each_tuple(argTup, [&arg_arr]<typename T>(T&& x) { push_arg(std::forward<T>(x), arg_arr); });

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
packed_func<R, Args...> pack_adapter<adapters::njson_adapter>::deserialize_pack(const adapters::njson& serial_obj)
{
    using namespace adapters;

    unsigned i = 0;

    typename packed_func<R, Args...>::args_t args{ parse_arg<Args>(serial_obj["args"], i++)... };

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
            return packed_func<R, Args...>(serial_obj["func_name"], serial_obj["result"].get<R>(), std::move(args));
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
inline std::string pack_adapter<adapters::njson_adapter>::get_func_name(const adapters::njson& serial_obj)
{
    return serial_obj["func_name"];
}

template<>
inline void pack_adapter<adapters::njson_adapter>::set_err_mesg(adapters::njson& serial_obj, std::string mesg)
{
    serial_obj["err_mesg"] = std::move(mesg);
}
} // namespace rpc
