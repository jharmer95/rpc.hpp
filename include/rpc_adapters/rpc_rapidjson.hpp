///@file rpc_adapters/rpc_rapidjson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting rapidjson (https://github.com/Tencent/rapidjson)
///@version 0.2.4
///@date 03-01-2021
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

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#if !defined(RPC_HPP_RAPIDJSON_ENABLED)
static_assert(false,
    R"(rpc_rapidjson.hpp included without defining RPC_HPP_RAPIDJSON_ENABLED!
Please define this macro or do not include this header!)")
#else

using rpdjson_doc = rapidjson::Document;
using rpdjson_val = rapidjson::Value;
using rpdjson_serial_t = typename rpc::serial_t<rpdjson_val, rpdjson_doc>;
using rpdjson_adapter = rpc::serial_adapter<rpdjson_serial_t>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> rpdjson_adapter::to_packed_func(const rpdjson_doc& serial_obj)
{
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<rpdjson_serial_t, Args>(
        serial_obj, i)... };

    if constexpr (std::is_void_v<R>)
    {
        return packed_func<void, Args...>(serial_obj["func_name"].GetString(), args);
    }
    else
    {
        if (serial_obj.HasMember("result"))
        {
            const rpdjson_val& result = serial_obj["result"];

            if (result.IsNull())
            {
                return packed_func<R, Args...>(
                    serial_obj["func_name"].GetString(), std::nullopt, args);
            }

            if constexpr (std::is_same_v<R, std::string>)
            {
                return packed_func<R, Args...>(
                    serial_obj["func_name"].GetString(), result.GetString(), args);
            }
            else if constexpr (details::is_container_v<R>)
            {
                R container;
                populate_array(result, container);
                return packed_func<R, Args...>(
                    serial_obj["func_name"].GetString(), container, args);
            }
            else
            {
                return packed_func<R, Args...>(
                    serial_obj["func_name"].GetString(), result.Get<R>(), args);
            }
        }

        return packed_func<R, Args...>(serial_obj["func_name"].GetString(), std::nullopt, args);
    }
}

template<typename T>
void push_arg(
    T arg, rpdjson_val& arg_list, const size_t arg_sz, rapidjson::MemoryPoolAllocator<>& alloc)
{
    if constexpr (std::is_same_v<T, std::string>)
    {
        arg_list.PushBack(rpdjson_val{}.SetString(arg.c_str(), alloc), alloc);
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        // Arrays have a capacity: "c" and the current data: "d"
        rpdjson_val val;
        val.SetObject();
        rpdjson_val val_c;
        val_c.SetUint64(arg_sz);
        rpdjson_val val_d;

        if constexpr (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>)
        {
            // special case for char*
            val_d.SetString(arg, alloc);
        }
        else
        {
            val_d.SetArray();

            for (size_t i = 0; i < arg_sz; ++i)
            {
                push_arg(arg[i], val_d, 0, alloc);
            }
        }

        val.AddMember("c", val_c, alloc);
        val.AddMember("d", val_d, alloc);
        arg_list.PushBack(val, alloc);
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
        // Rapidjson is silly and does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
        if constexpr (std::is_same_v<T,
                          char> || std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>)
        {
            arg_list.PushBack(rpdjson_val().SetInt(arg), alloc);
        }
        else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>)
        {
            arg_list.PushBack(rpdjson_val().SetUint(arg), alloc);
        }
        else
        {
            arg_list.PushBack(rpdjson_val().Set<T>(arg), alloc);
        }
    }
    else if constexpr (rpc::details::is_container_v<T>)
    {
        rpdjson_val sub_arr;
        sub_arr.SetArray();

        for (auto it = arg.begin(); it != arg.end(); ++it)
        {
            push_arg(*it, sub_arr, 0, alloc);
        }

        arg_list.PushBack(sub_arr, alloc);
    }
    else
    {
        rpdjson_doc serialized = rpc::serialize<rpdjson_serial_t, T>(arg);
        rpdjson_val arr_val;
        arr_val.CopyFrom(serialized, alloc);
        arg_list.PushBack(arr_val, alloc);
    }
}

template<>
template<typename R, typename... Args>
rpdjson_doc rpdjson_adapter::from_packed_func(packed_func<R, Args...>&& pack)
{
    rpdjson_doc d;
    auto& alloc = d.GetAllocator();
    d.SetObject();

    rpdjson_val func_name;
    func_name.SetString(pack.get_func_name().c_str(), alloc);
    d.AddMember("func_name", func_name, alloc);

    rpdjson_val result;

    if constexpr (std::is_void_v<R>)
    {
        result.SetNull();
    }
    else
    {
        if (pack)
        {
            if constexpr (std::is_arithmetic_v<R>)
            {
                result.Set<R>(*pack.get_result());
            }
            else if constexpr (std::is_same_v<R, std::string>)
            {
                result.SetString(pack.get_result()->c_str(), alloc);
            }
            else if constexpr (details::is_container_v<R>)
            {
                const R container = *pack.get_result();
                result.SetArray();

                for (const auto& val : container)
                {
                    result.PushBack(val, alloc);
                }
            }
            else
            {
                result = serialize<rpdjson_serial_t, R>(*pack.get_result());
            }
        }
        else
        {
            result.SetNull();
        }
    }

    d.AddMember("result", result, alloc);

    rpdjson_val args;
    args.SetArray();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

#    if defined(RPC_HPP_ENABLE_POINTERS)
    i = 0;

    details::for_each_tuple(argTup, [&args, &alloc, &pack, &i](auto x) {
        const auto arg_sz = pack.get_arg_arr_sz(i++);
        push_arg(x, args, arg_sz, alloc);
    });
#    else
    details::for_each_tuple(argTup, [&args, &alloc](auto x) { push_arg(x, args, 0, alloc); });
#    endif

    d.AddMember("args", args, alloc);
    return d;
}

template<>
inline std::string rpdjson_adapter::to_string(const rpdjson_doc& serial_obj)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serial_obj.Accept(writer);
    return buffer.GetString();
}

template<>
inline rpdjson_doc rpdjson_adapter::from_string(const std::string& str)
{
    rpdjson_doc d;
    d.SetObject();
    d.Parse(str.c_str());
    return d;
}

template<>
inline std::string rpdjson_adapter::extract_func_name(const rpdjson_val& obj)
{
    return obj["func_name"].GetString();
}

template<>
inline rpdjson_doc rpdjson_adapter::make_sub_object(const rpdjson_val& obj, const unsigned index)
{
    rpdjson_doc d;
    d.SetObject();
    const auto& val = obj[index];
    d.CopyFrom(val, d.GetAllocator());
    return d;
}

template<>
inline rpdjson_doc rpdjson_adapter::make_sub_object(const rpdjson_val& obj, const std::string& name)
{
    rpdjson_doc d;
    d.SetObject();
    const auto& val = obj[name.c_str()];
    d.CopyFrom(val, d.GetAllocator());
    return d;
}

template<>
template<typename T>
T rpdjson_adapter::get_value(const rpdjson_val& obj)
{
    if constexpr (std::is_same_v<T, std::string>)
    {
        return std::string(obj.GetString());
    }
    else if constexpr (details::is_container_v<T>)
    {
        T container;
        populate_array(obj, container);
        return container;
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
        // Rapidjson is silly and does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
        if constexpr (std::is_same_v<T,
                          char> || std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>)
        {
            return obj.GetInt();
        }
        else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>)
        {
            return obj.GetUint();
        }
        else
        {
            return obj.Get<T>();
        }
    }
    else
    {
        return rpc::deserialize<rpdjson_serial_t, T>(obj);
    }
}

template<>
template<typename Container>
void rpdjson_adapter::populate_array(const rpdjson_val& obj, Container& container)
{
    static_assert(details::is_container_v<Container>, "Type is not a container!");
    using value_t = typename Container::value_type;

    const auto& arr = obj.GetArray();

    for (const auto& val : arr)
    {
        container.push_back(details::arg_from_serial<rpdjson_serial_t, value_t>(val));
    }
}

template<>
inline size_t rpdjson_adapter::get_num_args(const rpdjson_val& obj)
{
    return obj["args"].GetArray().Size();
}

#    if defined(RPC_HPP_ENABLE_POINTERS)
template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> rpdjson_adapter::to_packed_func_w_ptr(
    const rpdjson_val& serial_obj, const std::array<std::any, sizeof...(Args)>& arg_arr)
{
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{
        details::args_from_serial_w_ptr<rpdjson_serial_t, Args>(serial_obj, arg_arr, i)...
    };

    std::unique_ptr<packed_func<R, Args...>> pack_ptr;

    if constexpr (std::is_void_v<R>)
    {
        pack_ptr =
            std::make_unique<packed_func<void, Args...>>(serial_obj["func_name"].GetString(), args);
    }
    else
    {
        if (serial_obj.HasMember("result") && !serial_obj["result"].IsNull())
        {
            const rpdjson_val& result = serial_obj["result"];

            if constexpr (std::is_same_v<R, std::string>)
            {
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    serial_obj["func_name"].GetString(), result.GetString(), args);
            }
            else if constexpr (details::is_container_v<R>)
            {
                R container;
                populate_array(result, container);
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    serial_obj["func_name"].GetString(), container, args);
            }
            else
            {
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    serial_obj["func_name"].GetString(), result.Get<R>(), args);
            }
        }
        else
        {
            pack_ptr = std::make_unique<packed_func<R, Args...>>(
                serial_obj["func_name"].GetString(), std::nullopt, args);
        }
    }

    pack_ptr->update_arg_arr(arg_arr);
    return *pack_ptr;
}

template<>
template<typename Value>
rpc::details::dyn_array<Value> rpdjson_adapter::parse_arg_arr(const rpdjson_val& arg_obj)
{
    const auto cap = arg_obj["c"].GetUint64();
    details::dyn_array<Value> arg_arr(cap);

    if constexpr (std::is_same_v<Value, char>)
    {
        const auto* data = arg_obj["d"].GetString();
        const auto sz = strlen(data);

        for (size_t i = 0; i < sz; ++i)
        {
            arg_arr.push_back(data[i]);
        }

        arg_arr.push_back('\0');
    }
    else
    {
        const auto& data = arg_obj["d"].GetArray();

        for (const auto& obj : data)
        {
            arg_arr.push_back(details::arg_from_serial<rpdjson_serial_t, Value>(obj));
        }
    }

    return arg_arr;
}
#    endif
#endif
