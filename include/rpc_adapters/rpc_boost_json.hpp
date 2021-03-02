///@file rpc_adapters/rpc_boost_json.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting Boost.JSON (https://github.com/boostorg/json)
///@version 0.3.1
///@date 03-02-2021
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

#include <boost/json/src.hpp>

#if !defined(RPC_HPP_BOOST_JSON_ENABLED)
static_assert(false,
    R"(rpc_boost_json.hpp included without defining RPC_HPP_BOOST_JSON_ENABLED!
Please define this macro or do not include this header!)")
#else

namespace bjson = boost::json;
using bjson_obj = bjson::object;
using bjson_val = bjson::value;

using bjson_serial_t = rpc::serial_t<bjson_val>;
using bjson_adapter = rpc::serial_adapter<bjson_serial_t>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> bjson_adapter::to_packed_func(const bjson_val& serial_obj)
{
    assert(serial_obj.is_object());
    const auto& obj = serial_obj.as_object();
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<bjson_serial_t, Args>(
        obj, i)... };

    if constexpr (std::is_void_v<R>)
    {
        return packed_func<void, Args...>(obj.at("func_name").get_string().c_str(), args);
    }
    else
    {
        if (obj.contains("result") && !obj.at("result").is_null())
        {
            const auto& result = obj.at("result");

            if constexpr (std::is_arithmetic_v<R>)
            {
                return packed_func<R, Args...>(
                    obj.at("func_name").get_string().c_str(), bjson::value_to<R>(result), args);
            }
            else if constexpr (std::is_same_v<R, std::string>)
            {
                return packed_func<R, Args...>(
                    obj.at("func_name").get_string().c_str(), result.get_string().c_str(), args);
            }
            else if constexpr (details::is_container_v<R>)
            {
                R container;
                populate_array(result, container);
                return packed_func<R, Args...>(
                    obj.at("func_name").get_string().c_str(), container, args);
            }
            else
            {
                return packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
                    deserialize<bjson_serial_t, R>(result), args);
            }
        }

        return packed_func<R, Args...>(
            obj.at("func_name").get_string().c_str(), std::nullopt, args);
    }
}

template<typename T>
void push_arg(T arg, bjson::array& arg_list, const size_t arg_sz)
{
    if constexpr (std::is_pointer_v<T>)
    {
        bjson_obj obj_j;
        obj_j["c"] = static_cast<int64_t>(arg_sz);

        if constexpr (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>)
        {
            // special case for char*
            obj_j["d"] = std::string(arg);
        }
        else
        {
            obj_j["d"] = bjson::array{};
            auto& data = obj_j["d"].as_array();

            for (size_t i = 0; i < arg_sz; ++i)
            {
                push_arg(arg[i], data, 0);
            }
        }

        arg_list.push_back(obj_j);
    }
    else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
    {
        arg_list.push_back(bjson::value_from<T>(std::move(arg)));
    }
    else if constexpr (rpc::details::is_container_v<T>)
    {
        bjson::array arr;

        for (const auto& val : arg)
        {
            push_arg(val, arr, 0);
        }

        arg_list.push_back(arr);
    }
    else
    {
        arg_list.push_back(rpc::serialize<bjson_serial_t, T>(arg));
    }
}

template<>
template<typename R, typename... Args>
bjson_val bjson_adapter::from_packed_func(packed_func<R, Args...>&& pack)
{
    bjson_obj ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;
    auto& result = ret_j["result"];

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            if constexpr (std::is_arithmetic_v<R> || std::is_same_v<R, std::string>)
            {
                result = *pack.get_result();
            }
            else if constexpr (details::is_container_v<R>)
            {
                result = bjson::array{};
                const auto container = *pack.get_result();

                for (const auto& val : container)
                {
                    result.as_array().push_back(val);
                }
            }
            else
            {
                result = serialize<bjson_serial_t, R>(*pack.get_result());
            }
        }
    }

    ret_j["args"] = bjson::array{};
    auto& args = ret_j["args"].as_array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

#    if defined(RPC_HPP_ENABLE_POINTERS)
    i = 0;

    details::for_each_tuple(argTup, [&args, &pack, &i](auto x) {
        const auto arg_sz = pack.get_arg_arr_sz(i++);
        push_arg(x, args, arg_sz);
    });
#    else
    details::for_each_tuple(argTup, [&args](auto x) { push_arg(x, args, 0); });
#    endif

    return ret_j;
}

template<>
inline std::string bjson_adapter::to_string(const bjson_val& serial_obj)
{
    return bjson::serialize(serial_obj);
}

template<>
inline bjson_val bjson_adapter::from_string(const std::string& str)
{
    return bjson::parse(str);
}

template<>
inline std::string bjson_adapter::extract_func_name(const bjson_val& obj)
{
    assert(obj.is_object());
    return obj.at("func_name").get_string().c_str();
}

template<>
inline bjson_val bjson_adapter::make_sub_object(const bjson_val& obj, const unsigned index)
{
    assert(obj.is_array());
    return obj.as_array()[index];
}

template<>
inline bjson_val bjson_adapter::make_sub_object(const bjson_val& obj, const std::string& name)
{
    assert(obj.is_object());
    return obj.as_object().at(name);
}

template<>
template<typename T>
T bjson_adapter::get_value(const bjson_val& obj)
{
    return bjson::value_to<T>(obj);
}

template<>
template<typename R>
void bjson_adapter::set_result(bjson_val& serial_obj, R val)
{
    assert(serial_obj.is_object());
    auto& result = serial_obj.at("result");

    if constexpr (std::is_arithmetic_v<R> || std::is_same_v<R, std::string>)
    {
        result = val;
    }
    else if constexpr (details::is_container_v<R>)
    {
        result = bjson::array{};
        const auto container = val;

        for (const auto& v : container)
        {
            result.as_array().push_back(v);
        }
    }
    else
    {
        result = serialize<bjson_serial_t, R>(val);
    }
}

template<>
template<typename Container>
void bjson_adapter::populate_array(const bjson_val& obj, Container& container)
{
    static_assert(
        details::is_container_v<Container>, "Container type must have begin and end iterators");

    using value_t = typename Container::value_type;

    assert(obj.is_array());

    for (const auto& val : obj.as_array())
    {
        container.push_back(details::arg_from_serial<bjson_serial_t, value_t>(val));
    }
}

template<>
inline size_t bjson_adapter::get_num_args(const bjson_val& obj)
{
    assert(obj.is_object());
    return obj.as_object().at("args").as_array().size();
}

#    if defined(RPC_HPP_ENABLE_POINTERS)
template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> bjson_adapter::to_packed_func_w_ptr(
    const bjson_val& serial_obj, const std::array<std::any, sizeof...(Args)>& arg_arr)
{
    assert(serial_obj.is_object());
    auto& obj = serial_obj.as_object();
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{
        details::args_from_serial_w_ptr<bjson_serial_t, Args>(serial_obj, arg_arr, i)...
    };

    std::unique_ptr<packed_func<R, Args...>> pack_ptr;

    if constexpr (std::is_void_v<R>)
    {
        pack_ptr = std::make_unique<packed_func<void, Args...>>(
            obj.at("func_name").get_string().c_str(), args);
    }
    else
    {
        if (obj.contains("result") && !obj.at("result").is_null())
        {
            const auto& result = obj.at("result");

            if constexpr (std::is_arithmetic_v<R>)
            {
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    obj.at("func_name").get_string().c_str(), bjson::value_to<R>(result), args);
            }
            else if constexpr (std::is_same_v<R, std::string>)
            {
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    obj.at("func_name").get_string().c_str(), result.get_string().c_str(), args);
            }
            else if constexpr (details::is_container_v<R>)
            {
                R container;
                populate_array(result, container);
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    obj.at("func_name").get_string().c_str(), container, args);
            }
            else
            {
                pack_ptr = std::make_unique<packed_func<R, Args...>>(
                    obj.at("func_name").get_string().c_str(),
                    deserialize<bjson_serial_t, R>(result), args);
            }
        }
        else
        {
            pack_ptr = std::make_unique<packed_func<R, Args...>>(
                obj.at("func_name").get_string().c_str(), std::nullopt, args);
        }
    }

    pack_ptr->update_arg_arr(arg_arr);
    return *pack_ptr;
}

template<>
template<typename Value>
rpc::details::dyn_array<Value> bjson_adapter::parse_arg_arr(const bjson_val& arg_obj)
{
    assert(arg_obj.is_object());
    const auto& obj = arg_obj.as_object();
    const auto cap = static_cast<size_t>(obj.at("c").get_int64());
    details::dyn_array<Value> arg_arr(cap);

    if constexpr (std::is_same_v<Value, char>)
    {
        const auto data = obj.at("d").as_string();

        for (const auto c : data)
        {
            arg_arr.push_back(c);
        }

        arg_arr.push_back('\0');
    }
    else
    {
        const auto& data = obj.at("d");
        assert(data.is_array());

        for (const auto& val : data.as_array())
        {
            arg_arr.push_back(details::arg_from_serial<bjson_serial_t, Value>(val));
        }
    }

    return arg_arr;
}
#    endif

#endif
