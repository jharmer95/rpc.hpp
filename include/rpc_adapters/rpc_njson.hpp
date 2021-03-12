///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.3.2
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

#include <utility>

#define RPC_HPP_NLOHMANN_SERIAL_CBOR 1
#define RPC_HPP_NLOHMANN_SERIAL_BSON 2
#define RPC_HPP_NLOHMANN_SERIAL_MSGPACK 3
#define RPC_HPP_NLOHMANN_SERIAL_UBJSON 4

#if !defined(RPC_HPP_NJSON_ENABLED)
static_assert(false,
    R"(rpc_njson.hpp included without defining RPC_HPP_NJSON_ENABLED!
Please define this macro or do not include this header!)")
#else

using njson = nlohmann::json;
using njson_serial_t = rpc::serial_t<njson>;
using njson_adapter = rpc::serial_adapter<njson_serial_t>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> njson_adapter::to_packed_func(const njson& serial_obj)
{
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<njson_serial_t, Args>(
        serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (serial_obj.contains("result") && !serial_obj["result"].is_null())
        {
            return packed_func<R, Args...>(
                serial_obj["func_name"], serial_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(serial_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(serial_obj["func_name"], args);
    }
}

template<typename T>
void push_arg(T arg, njson& arg_list, const size_t arg_sz)
{
    if constexpr (std::is_pointer_v<T>)
    {
        // Arrays have a capacity: "c" and the current data: "d"
        njson obj_j;
        obj_j["c"] = arg_sz;

        if constexpr (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>)
        {
            // special case for char*
            obj_j["d"] = std::string(arg);
        }
        else
        {
            obj_j["d"] = njson::array();
            auto& data = obj_j["d"];

            for (size_t i = 0; i < arg_sz; ++i)
            {
                push_arg(arg[i], data, 0);
            }
        }

        arg_list.push_back(obj_j);
    }
    else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
    {
        arg_list.push_back(arg);
    }
    else if constexpr (rpc::details::is_container_v<T>)
    {
        njson arr = njson::array();

        for (const auto& val : arg)
        {
            push_arg(val, arr, 0);
        }

        arg_list.push_back(arr);
    }
    else if constexpr (rpc::details::is_serializable_v<njson_serial_t, T>)
    {
        arg_list.push_back(T::serialize(arg));
    }
    else
    {
        arg_list.push_back(rpc::serialize<njson_serial_t, T>(arg));
    }
}

template<>
template<typename R, typename... Args>
njson njson_adapter::from_packed_func(packed_func<R, Args...>&& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    // TODO: Address use of containers/custom types for result
    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

#    if defined(RPC_HPP_ENABLE_POINTERS)
    i = 0;

    details::for_each_tuple(argTup, [&ret_j, &pack, &i](auto x) {
        const auto arg_sz = pack.get_arg_arr_sz(i++);
        push_arg(x, ret_j["args"], arg_sz);
    });
#    else
    details::for_each_tuple(argTup, [&ret_j](auto x) { push_arg(x, ret_j["args"], 0); });
#    endif

    return ret_j;
}

template<>
inline std::string njson_adapter::to_string(const njson& serial_obj)
{
    return serial_obj.dump();
}

template<>
inline njson njson_adapter::from_string(const std::string& str)
{
    return njson::parse(str);
}

template<>
inline std::string njson_adapter::extract_func_name(const njson& obj)
{
    return obj["func_name"].get<std::string>();
}

template<>
inline njson njson_adapter::make_sub_object(const njson& obj, const unsigned index)
{
    return obj[index];
}

template<>
inline njson njson_adapter::make_sub_object(const njson& obj, const std::string& name)
{
    return obj[name];
}

template<>
template<typename T>
T njson_adapter::get_value(const njson& obj)
{
    return obj.get<T>();
}

template<>
template<typename R>
void njson_adapter::set_result(njson& serial_obj, R val)
{
    // TODO: Address use of containers/custom types for result
    serial_obj["result"] = val;
}

template<>
template<typename Container>
void njson_adapter::populate_array(const njson& obj, Container& container)
{
    static_assert(
        details::is_container_v<Container>, "Container type must have begin and end iterators");

    using value_t = typename Container::value_type;

    for (const auto& val : obj)
    {
        container.push_back(details::arg_from_serial<njson_serial_t, value_t>(val));
    }
}

template<>
inline size_t njson_adapter::get_num_args(const njson& obj)
{
    return obj["args"].size();
}

#    if defined(RPC_HPP_ENABLE_POINTERS)
template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> njson_adapter::to_packed_func_w_ptr(
    const njson& serial_obj, const std::array<std::any, sizeof...(Args)>& arg_arr)
{
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{
        details::args_from_serial_w_ptr<njson_serial_t, Args>(serial_obj, arg_arr, i)...
    };

    std::unique_ptr<packed_func<R, Args...>> pack_ptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (serial_obj.contains("result") && !serial_obj["result"].is_null())
        {
            pack_ptr = std::make_unique<packed_func<R, Args...>>(
                serial_obj["func_name"], serial_obj["result"].get<R>(), args);
        }
        else
        {
            pack_ptr = std::make_unique<packed_func<R, Args...>>(
                serial_obj["func_name"], std::nullopt, args);
        }
    }
    else
    {
        pack_ptr = std::make_unique<packed_func<void, Args...>>(serial_obj["func_name"], args);
    }

    pack_ptr->update_arg_arr(arg_arr);
    return *pack_ptr;
}

template<>
template<typename Value>
rpc::details::dyn_array<Value> njson_adapter::parse_arg_arr(const njson& arg_obj)
{
    const auto cap = arg_obj["c"].get<size_t>();
    details::dyn_array<Value> arg_arr(cap);

    if constexpr (std::is_same_v<Value, char>)
    {
        const auto data = arg_obj["d"].get<std::string>();

        for (const auto c : data)
        {
            arg_arr.push_back(c);
        }

        arg_arr.push_back('\0');
    }
    else
    {
        const auto& data = arg_obj["d"];

        for (const auto& obj : data)
        {
            arg_arr.push_back(details::arg_from_serial<njson_serial_t, Value>(obj));
        }
    }

    return arg_arr;
}
#    endif

#    if defined(RPC_HPP_NLOHMANN_SERIAL_TYPE)
njson from_func(const std::vector<uint8_t>& serial_obj);
std::vector<uint8_t> to_func(const njson& serial_obj);

#        if RPC_HPP_NLOHMANN_SERIAL_TYPE == RPC_HPP_NLOHMANN_SERIAL_CBOR
using ncbor = std::vector<uint8_t>;

inline njson from_func(const std::vector<uint8_t>& serial_obj)
{
    return njson::from_cbor(serial_obj);
}

inline std::vector<uint8_t> to_func(const njson& serial_obj)
{
    return njson::to_cbor(serial_obj);
}

#        elif RPC_HPP_NLOHMANN_SERIAL_TYPE == RPC_HPP_NLOHMANN_SERIAL_BSON
using nbson = std::vector<uint8_t>;

inline njson from_func(const std::vector<uint8_t>& serial_obj)
{
    return njson::from_bson(serial_obj);
}

inline std::vector<uint8_t> to_func(const njson& serial_obj)
{
    return njson::to_bson(serial_obj);
}

#        elif RPC_HPP_NLOHMANN_SERIAL_TYPE == RPC_HPP_NLOHMANN_SERIAL_MSGPACK
using nmsgpack = std::vector<uint8_t>;

inline njson from_func(const std::vector<uint8_t>& serial_obj)
{
    return njson::from_msgpack(serial_obj);
}

inline std::vector<uint8_t> to_func(const njson& serial_obj)
{
    return njson::to_msgpack(serial_obj);
}

#        elif RPC_HPP_NLOHMANN_SERIAL_TYPE == RPC_HPP_NLOHMANN_SERIAL_UBJSON
using nubjson = std::vector<uint8_t>;

inline njson from_func(const std::vector<uint8_t>& serial_obj)
{
    return njson::from_ubjson(serial_obj);
}

inline std::vector<uint8_t> to_func(const njson& serial_obj)
{
    return njson::to_ubjson(serial_obj);
}
#        endif

using byte_vec = std::vector<uint8_t>;
using generic_serial_t = rpc::serial_t<byte_vec>;
using generic_serial_adapter = rpc::serial_adapter<generic_serial_t>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> generic_serial_adapter::to_packed_func(const byte_vec& serial_obj)
{
    unsigned i = 0;
    const njson j_obj = from_func(serial_obj);

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<generic_serial_t, Args>(
        serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (j_obj.contains("result") && !j_obj["result"].is_null())
        {
            return packed_func<R, Args...>(j_obj["func_name"], j_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(j_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(j_obj["func_name"], args);
    }
}

template<>
template<typename R, typename... Args>
byte_vec generic_serial_adapter::from_packed_func(packed_func<R, Args...>&& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

#        if defined(RPC_HPP_ENABLE_POINTERS)
    i = 0;

    details::for_each_tuple(argTup, [&ret_j, &pack, &i](auto x) {
        const auto arg_sz = pack.get_arg_arr_sz(i++);
        push_arg(x, ret_j["args"], arg_sz);
    });
#        else
    details::for_each_tuple(argTup, [&ret_j](auto x) { push_arg(x, ret_j["args"], 0); });
#        endif

    return to_func(ret_j);
}

template<>
inline std::string generic_serial_adapter::to_string(const byte_vec& serial_obj)
{
    return from_func(serial_obj).dump();
}

template<>
inline byte_vec generic_serial_adapter::from_string(const std::string& str)
{
    return to_func(njson::parse(str));
}

template<>
inline std::string generic_serial_adapter::extract_func_name(const byte_vec& obj)
{
    return from_func(obj)["func_name"].get<std::string>();
}

template<>
inline byte_vec generic_serial_adapter::make_sub_object(const byte_vec& obj, const unsigned index)
{
    return to_func(from_func(obj)[index]);
}

template<>
inline byte_vec generic_serial_adapter::make_sub_object(
    const byte_vec& obj, const std::string& name)
{
    return to_func(from_func(obj)[name]);
}

template<>
template<typename T>
T generic_serial_adapter::get_value(const byte_vec& obj)
{
    return from_func(obj).get<T>();
}

template<>
template<typename R>
void generic_serial_adapter::set_result(byte_vec& serial_obj, R val)
{
    auto obj_j = from_func(serial_obj);

    // TODO: Address use of containers/custom types for result
    obj_j["result"] = val;
    serial_obj = to_func(obj_j);
}

template<>
template<typename Container>
void generic_serial_adapter::populate_array(const byte_vec& obj, Container& container)
{
    static_assert(
        details::is_container_v<Container>, "Container type must have begin and end iterators");

    using value_t = typename Container::value_type;
    njson j_obj = from_func(obj);

    for (const auto& val : j_obj)
    {
        container.push_back(details::arg_from_serial<njson_serial_t, value_t>(val));
    }
}

template<>
inline size_t generic_serial_adapter::get_num_args(const byte_vec& obj)
{
    return from_func(obj)["args"].size();
}

#        if defined(RPC_HPP_ENABLE_POINTERS)
template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> generic_serial_adapter::to_packed_func_w_ptr(
    const byte_vec& serial_obj, const std::array<std::any, sizeof...(Args)>& arg_arr)
{
    unsigned i = 0;
    const njson j_obj = from_func(serial_obj);

    std::array<std::any, sizeof...(Args)> args{
        details::args_from_serial_w_ptr<generic_serial_t, Args>(serial_obj, arg_arr, i)...
    };

    std::unique_ptr<packed_func<R, Args...>> pack_ptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (j_obj.contains("result") && !j_obj["result"].is_null())
        {
            pack_ptr = std::make_unique<packed_func<R, Args...>>(
                j_obj["func_name"], j_obj["result"].get<R>(), args);
        }

        pack_ptr =
            std::make_unique<packed_func<R, Args...>>(j_obj["func_name"], std::nullopt, args);
    }
    else
    {
        pack_ptr = std::make_unique<packed_func<void, Args...>>(j_obj["func_name"], args);
    }

    pack_ptr->update_arg_arr(arg_arr);
    return *pack_ptr;
}

template<>
template<typename Value>
rpc::details::dyn_array<Value> generic_serial_adapter::parse_arg_arr(const byte_vec& arg_obj)
{
    const njson j_obj = from_func(arg_obj);
    const auto cap = j_obj["c"].get<size_t>();
    details::dyn_array<Value> arg_arr(cap);

    if constexpr (std::is_same_v<Value, char>)
    {
        const auto data = j_obj["d"].get<std::string>();

        for (const auto c : data)
        {
            arg_arr.push_back(c);
        }

        arg_arr.push_back('\0');
    }
    else
    {
        const auto& data = j_obj["d"];

        for (const auto& obj : data)
        {
            arg_arr.push_back(details::arg_from_serial<njson_serial_t, Value>(obj));
        }
    }

    return arg_arr;
}
#        endif
#    endif
#endif
