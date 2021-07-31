///@file rpc_adapters/rpc_rapidjson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting rapidjson (https://github.com/Tencent/rapidjson)
///@version 0.5.1
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

#if !defined(RPC_HPP_ENABLE_RAPIDJSON)
#    error 'rpc_rapidjson' was included without defining 'RPC_HPP_ENABLE_RAPIDJSON' Please define this macro or do not include this header!
#endif

#include "../rpc.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace rpc
{
namespace adapters
{
    using rapidjson_doc = rapidjson::Document;
    using rapidjson_val = rapidjson::Value;
    using rapidjson_adapter = details::serial_adapter<rapidjson_doc, std::string>;

    template<typename T>
    void push_arg(T&& arg, rapidjson_val& arg_arr, rapidjson::MemoryPoolAllocator<>& alloc)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        if constexpr (std::is_same_v<no_ref_t, std::string>)
        {
            arg_arr.PushBack(rapidjson_val{}.SetString(arg.c_str(), alloc), alloc);
        }
        else if constexpr (std::is_arithmetic_v<no_ref_t>)
        {
            // Rapidjson is silly and does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
            if constexpr (
                std::is_same_v<no_ref_t,
                    char> || std::is_same_v<no_ref_t, int8_t> || std::is_same_v<no_ref_t, int16_t>)
            {
                arg_arr.PushBack(rapidjson_val().SetInt(arg), alloc);
            }
            else if constexpr (std::is_same_v<no_ref_t,
                                   uint8_t> || std::is_same_v<no_ref_t, uint16_t>)
            {
                arg_arr.PushBack(rapidjson_val().SetUint(arg), alloc);
            }
            else
            {
                arg_arr.PushBack(rapidjson_val().Set<no_ref_t>(arg), alloc);
            }
        }
        else if constexpr (rpc::details::is_container_v<no_ref_t>)
        {
            rapidjson_val sub_arr;
            sub_arr.SetArray();

            for (auto&& val : arg)
            {
                push_arg(std::forward<decltype(val)>(val), sub_arr, alloc);
            }

            arg_arr.PushBack(sub_arr, alloc);
        }
        else if constexpr (rpc::details::is_serializable_v<rapidjson_adapter, no_ref_t>)
        {
            rapidjson_doc serialized =
                no_ref_t::template serialize<rapidjson_adapter>(std::forward<T>(arg));
            rapidjson_val arr_val;
            arr_val.CopyFrom(serialized, alloc);
            arg_arr.PushBack(arr_val, alloc);
        }
        else
        {
            rapidjson_doc serialized =
                rapidjson_adapter::template serialize<no_ref_t>(std::forward<T>(arg));
            rapidjson_val arr_val;
            arr_val.CopyFrom(serialized, alloc);
            arg_arr.PushBack(arr_val, alloc);
        }
    }

    template<typename T>
    std::remove_cv_t<std::remove_reference_t<T>> parse_arg(
        const rapidjson_val& arg_arr, unsigned& index)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        const rapidjson_val& arg = arg_arr.IsArray() ? arg_arr.GetArray()[index++] : arg_arr;

        if constexpr (std::is_same_v<no_ref_t, std::string>)
        {
            return arg.GetString();
        }
        else if constexpr (std::is_arithmetic_v<no_ref_t>)
        {
            // Rapidjson is silly and does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
            if constexpr (
                std::is_same_v<no_ref_t,
                    char> || std::is_same_v<no_ref_t, int8_t> || std::is_same_v<no_ref_t, int16_t>)
            {
                return arg.GetInt();
            }
            else if constexpr (std::is_same_v<no_ref_t,
                                   uint8_t> || std::is_same_v<no_ref_t, uint16_t>)
            {
                return arg.GetUint();
            }
            else
            {
                return arg.Get<no_ref_t>();
            }
        }
        else if constexpr (details::is_container_v<no_ref_t>)
        {
            using value_t = typename no_ref_t::value_type;
            no_ref_t container;
            container.reserve(arg.Size());

            unsigned j = 0;

            for (const auto& val : arg.GetArray())
            {
                container.push_back(parse_arg<value_t>(val, j));
            }

            return container;
        }
        else if constexpr (details::is_serializable_v<rapidjson_adapter, no_ref_t>)
        {
            rapidjson_doc d;
            d.CopyFrom(arg, d.GetAllocator());
            return no_ref_t::template deserialize<rapidjson_adapter>(d);
        }
        else
        {
            rapidjson_doc d;
            d.CopyFrom(arg, d.GetAllocator());
            return rapidjson_adapter::template deserialize<no_ref_t>(d);
        }
    }
} // namespace adapters

template<>
inline std::string adapters::rapidjson_adapter::to_bytes(const adapters::rapidjson_doc& serial_obj)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serial_obj.Accept(writer);
    return buffer.GetString();
}

template<>
inline adapters::rapidjson_doc adapters::rapidjson_adapter::from_bytes(const std::string& bytes)
{
    adapters::rapidjson_doc d;
    d.SetObject();
    d.Parse(bytes.c_str());
    return d;
}

template<>
inline std::string adapters::rapidjson_adapter::to_bytes(adapters::rapidjson_doc&& serial_obj)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    std::move(serial_obj).Accept(writer);
    return buffer.GetString();
}

template<>
inline adapters::rapidjson_doc adapters::rapidjson_adapter::from_bytes(std::string&& bytes)
{
    adapters::rapidjson_doc d;
    d.SetObject();
    d.Parse(std::move(bytes).c_str());
    return d;
}

template<>
template<typename R, typename... Args>
adapters::rapidjson_doc pack_adapter<adapters::rapidjson_adapter>::serialize_pack(
    const packed_func<R, Args...>& pack)
{
    using namespace adapters;

    rapidjson_doc d;
    auto& alloc = d.GetAllocator();
    d.SetObject();

    d.AddMember("func_name", rapidjson_val{}.SetString(pack.get_func_name().c_str(), alloc), alloc);

    if constexpr (!std::is_void_v<R>)
    {
        rapidjson_val result;

        if (pack)
        {
            if constexpr (std::is_arithmetic_v<R>)
            {
                result.Set<R>(pack.get_result());
            }
            else if constexpr (std::is_same_v<R, std::string>)
            {
                result.SetString(pack.get_result().c_str(), alloc);
            }
            else if constexpr (details::is_container_v<R>)
            {
                const R container = pack.get_result();
                result.SetArray();

                for (const auto& val : container)
                {
                    result.PushBack(val, alloc);
                }
            }
            else if constexpr (details::is_serializable_v<rapidjson_adapter, R>)
            {
                rapidjson_doc tmp = R::template serialize<rapidjson_adapter>(pack.get_result());
                result.CopyFrom(tmp, alloc);
            }
            else
            {
                rapidjson_doc tmp = rapidjson_adapter::template serialize<R>(pack.get_result());
                result.CopyFrom(tmp, alloc);
            }
        }
        else
        {
            result.SetNull();
        }

        d.AddMember("result", result, alloc);
    }

    rapidjson_val args;
    args.SetArray();

    const auto& argTup = pack.get_args();
    details::for_each_tuple(
        argTup, [&args, &alloc](auto&& x) { push_arg(std::forward<decltype(x)>(x), args, alloc); });

    d.AddMember("args", args, alloc);
    return d;
}

template<>
template<typename R, typename... Args>
packed_func<R, Args...> pack_adapter<adapters::rapidjson_adapter>::deserialize_pack(
    const adapters::rapidjson_doc& serial_obj)
{
    using namespace adapters;

    unsigned i = 0;

    typename packed_func<R, Args...>::args_t args{ parse_arg<Args>(serial_obj["args"], i)... };

    if constexpr (std::is_void_v<R>)
    {
        packed_func<void, Args...> pack(serial_obj["func_name"].GetString(), std::move(args));

        if (serial_obj.HasMember("err_mesg"))
        {
            pack.set_err_mesg(serial_obj["err_mesg"].GetString());
        }

        return pack;
    }
    else
    {
        if (serial_obj.HasMember("result") && !serial_obj["result"].IsNull())
        {
            const rapidjson_val& result = serial_obj["result"];

            if constexpr (std::is_same_v<R, std::string>)
            {
                return packed_func<R, Args...>(
                    serial_obj["func_name"].GetString(), result.GetString(), std::move(args));
            }
            else if constexpr (details::is_container_v<R>)
            {
                using value_t = typename R::value_type;
                R container;
                container.reserve(result.Size());

                unsigned j = 0;

                for (const auto& val : result.GetArray())
                {
                    container.push_back(parse_arg<value_t>(val, j));
                }

                return packed_func<R, Args...>(
                    serial_obj["func_name"].GetString(), container, std::move(args));
            }
            else if constexpr (std::is_arithmetic_v<R>)
            {
                if constexpr (std::is_same_v<R,
                                  char> || std::is_same_v<R, int8_t> || std::is_same_v<R, int16_t>)
                {
                    return packed_func<R, Args...>(
                        serial_obj["func_name"].GetString(), result.GetInt(), std::move(args));
                }
                else if constexpr (std::is_same_v<R, uint8_t> || std::is_same_v<R, uint16_t>)
                {
                    return packed_func<R, Args...>(
                        serial_obj["func_name"].GetString(), result.GetUint(), std::move(args));
                }
                else
                {
                    return packed_func<R, Args...>(
                        serial_obj["func_name"].GetString(), result.Get<R>(), std::move(args));
                }
            }
            else if constexpr (details::is_serializable_v<rapidjson_adapter, R>)
            {
                rapidjson_doc d;
                d.CopyFrom(result, d.GetAllocator());
                return packed_func<R, Args...>(serial_obj["func_name"].GetString(),
                    R::template deserialize<rapidjson_adapter>(d), std::move(args));
            }
            else
            {
                rapidjson_doc d;
                d.CopyFrom(result, d.GetAllocator());
                return packed_func<R, Args...>(serial_obj["func_name"].GetString(),
                    rapidjson_adapter::template deserialize<R>(d), args);
            }
        }

        packed_func<R, Args...> pack(
            serial_obj["func_name"].GetString(), std::nullopt, std::move(args));

        if (serial_obj.HasMember("err_mesg"))
        {
            pack.set_err_mesg(serial_obj["err_mesg"].GetString());
        }

        return pack;
    }
}

template<>
inline std::string pack_adapter<adapters::rapidjson_adapter>::get_func_name(
    const adapters::rapidjson_doc& serial_obj)
{
    return serial_obj["func_name"].GetString();
}

template<>
inline void pack_adapter<adapters::rapidjson_adapter>::set_err_mesg(
    adapters::rapidjson_doc& serial_obj, std::string&& mesg)
{
    auto& alloc = serial_obj.GetAllocator();

    if (serial_obj.HasMember("err_mesg"))
    {
        serial_obj["HasMember"].SetString(std::move(mesg).c_str(), alloc);
    }
    else
    {
        adapters::rapidjson_val v;
        v.SetString(std::move(mesg).c_str(), alloc);
        serial_obj.AddMember("err_mesg", v, alloc);
    }
}
} // namespace rpc
