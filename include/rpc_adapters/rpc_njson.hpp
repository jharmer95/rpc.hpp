///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.2.0
///@date 09-10-2020
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020, Jackson Harmer
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
using njson_adapter = rpc::serial_adapter<njson>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> njson_adapter::to_packed_func(const njson& serial_obj)
{
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<njson, Args>(
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
void push_arg(T arg, njson& arg_list)
{
    if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
    {
        arg_list.push_back(arg);
    }
    else if constexpr (rpc::is_container_v<T>)
    {
        njson arr = njson::array();

        for (const auto& val : arg)
        {
            push_arg(val, arr);
        }

        arg_list.push_back(arr);
    }
    else
    {
        arg_list.push_back(rpc::serialize<njson, T>(arg));
    }
}

template<>
template<typename R, typename... Args>
njson njson_adapter::from_packed_func(const packed_func<R, Args...>& pack)
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

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { push_arg(x, ret_j["args"]); });

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
template<typename Container>
void njson_adapter::populate_array(const njson& obj, Container& container)
{
    static_assert(is_container_v<Container>, "Container type must have begin and end iterators");
    static_assert(is_container_v<Container>, "Type is not a container!");
    using value_t = typename Container::value_type;

    for (const auto& val : obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}

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

using generic_serial_t = std::vector<uint8_t>;
using generic_serial_adapter = rpc::serial_adapter<generic_serial_t>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> generic_serial_adapter::to_packed_func(
    const generic_serial_t& serial_obj)
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
generic_serial_t generic_serial_adapter::from_packed_func(const packed_func<R, Args...>& pack)
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

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { push_arg(x, ret_j["args"]); });

    return generic_serial_t(to_func(ret_j));
}

template<>
inline std::string generic_serial_adapter::to_string(const generic_serial_t& serial_obj)
{
    return from_func(serial_obj).dump();
}

template<>
inline generic_serial_t generic_serial_adapter::from_string(const std::string& str)
{
    return to_func(njson::parse(str));
}

template<>
inline std::string generic_serial_adapter::extract_func_name(const generic_serial_t& obj)
{
    return from_func(obj)["func_name"].get<std::string>();
}

template<>
inline generic_serial_t generic_serial_adapter::make_sub_object(
    const generic_serial_t& obj, const unsigned index)
{
    return to_func(from_func(obj)[index]);
}

template<>
inline generic_serial_t generic_serial_adapter::make_sub_object(
    const generic_serial_t& obj, const std::string& name)
{
    return to_func(from_func(obj)[name]);
}

template<>
template<typename T>
T generic_serial_adapter::get_value(const generic_serial_t& obj)
{
    return from_func(obj).get<T>();
}

template<>
template<typename Container>
void generic_serial_adapter::populate_array(const generic_serial_t& obj, Container& container)
{
    static_assert(is_container_v<Container>, "Container type must have begin and end iterators");
    using value_t = typename Container::value_type;
    njson j_obj = from_func(obj);

    for (const auto& val : j_obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}
#    endif
#endif
