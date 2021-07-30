///@file rpc_adapters/rpc_boost_json.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting Boost.JSON (https://github.com/boostorg/json)
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

#if !defined(RPC_HPP_ENABLE_BOOST_JSON)
#    error 'rpc_boost_json' was included without defining 'RPC_HPP_ENABLE_BOOST_JSON' Please define this macro or do not include this header!
#endif

#include "../rpc.hpp"

#include <cassert>

#include <boost/json/src.hpp>

namespace rpc
{
namespace adapters
{
    namespace bjson = boost::json;
    using bjson_obj = bjson::object;
    using bjson_val = bjson::value;
    using bjson_adapter = details::serial_adapter<bjson_val, std::string>;

    template<typename T>
    void push_arg(T&& arg, bjson::array& arg_arr)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        if constexpr (std::is_arithmetic_v<no_ref_t>)
        {
            arg_arr.push_back(std::forward<T>(arg));
        }
        else if constexpr (std::is_same_v<no_ref_t, std::string>)
        {
            arg_arr.push_back(bjson::string{ arg.c_str() });
        }
        else if constexpr (details::is_container_v<no_ref_t>)
        {
            bjson::array arr;

            for (auto&& val : arg)
            {
                push_arg(std::forward<decltype(val)>(val), arr);
            }

            arg_arr.push_back(arr);
        }
        else if constexpr (details::is_serializable_v<bjson_adapter, no_ref_t>)
        {
            arg_arr.push_back(no_ref_t::template serialize<bjson_adapter>(std::forward<T>(arg)));
        }
        else
        {
            arg_arr.push_back(bjson_adapter::template serialize<no_ref_t>(std::forward<T>(arg)));
        }
    }

    template<typename T>
    std::remove_cv_t<std::remove_reference_t<T>> parse_arg(
        const bjson_val& arg_arr, unsigned& index)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        const auto& arg = arg_arr.is_array() ? arg_arr.as_array().at(index++) : arg_arr;

        if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
        {
            return bjson::value_to<no_ref_t>(arg);
        }
        else if constexpr (details::is_container_v<no_ref_t>)
        {
            using value_t = typename no_ref_t::value_type;

            auto& arr = arg.as_array();
            no_ref_t container;
            container.reserve(arr.size());

            unsigned j = 0;

            for (const auto& val : arr)
            {
                container.push_back(parse_arg<value_t>(val, j));
            }

            return container;
        }
        else if constexpr (details::is_serializable_v<bjson_adapter, no_ref_t>)
        {
            return no_ref_t::template deserialize<bjson_adapter>(arg);
        }
        else
        {
            return bjson_adapter::template deserialize<no_ref_t>(arg);
        }
    }
} // namespace adapters

template<>
inline std::string adapters::bjson_adapter::to_bytes(const adapters::bjson_val& serial_obj)
{
    return adapters::bjson::serialize(serial_obj);
}

template<>
inline adapters::bjson_val adapters::bjson_adapter::from_bytes(const std::string& bytes)
{
    return adapters::bjson::parse(bytes);
}

template<>
inline std::string adapters::bjson_adapter::to_bytes(adapters::bjson_val&& serial_obj)
{
    return adapters::bjson::serialize(std::move(serial_obj));
}

template<>
inline adapters::bjson_val adapters::bjson_adapter::from_bytes(std::string&& bytes)
{
    return adapters::bjson::parse(std::move(bytes));
}

template<>
template<typename R, typename... Args>
adapters::bjson_val pack_adapter<adapters::bjson_adapter>::serialize_pack(
    const packed_func<R, Args...>& pack)
{
    using namespace adapters;

    bjson_obj ret_j;

    ret_j["func_name"] = pack.get_func_name();

    const auto err_mesg = pack.get_err_mesg();

    if (!err_mesg.empty())
    {
        ret_j["err_mesg"] = err_mesg;
    }

    if constexpr (!std::is_void_v<R>)
    {
        ret_j["result"] = nullptr;
        auto& result = ret_j["result"];

        if (pack)
        {
            if constexpr (std::is_arithmetic_v<R> || std::is_same_v<R, std::string>)
            {
                result = pack.get_result().value();
            }
            else if constexpr (details::is_container_v<R>)
            {
                result = bjson::array{};
                const auto container = pack.get_result().value();

                for (const auto& val : container)
                {
                    result.as_array().push_back(val);
                }
            }
            else if constexpr (details::is_serializable_v<bjson_adapter, R>)
            {
                result = R::template serialize<bjson_adapter>(pack.get_result().value());
            }
            else
            {
                result = bjson_adapter::template serialize<R>(pack.get_result().value());
            }
        }
    }

    bjson::array args{};

    const auto& argTup = pack.get_args();
    for_each_tuple(argTup, [&args](auto&& x) { push_arg(std::forward<decltype(x)>(x), args); });

    ret_j["args"] = std::move(args);
    return ret_j;
}

template<>
template<typename R, typename... Args>
packed_func<R, Args...> pack_adapter<adapters::bjson_adapter>::deserialize_pack(
    const adapters::bjson_val& serial_obj)
{
    using namespace adapters;

    assert(serial_obj.is_object());
    const auto& obj = serial_obj.as_object();
    unsigned i = 0;

    auto& args_val = obj.at("args");

    typename packed_func<R, Args...>::args_t args{ parse_arg<Args>(args_val, i)... };

    if constexpr (std::is_void_v<R>)
    {
        packed_func<void, Args...> pack(obj.at("func_name").get_string().c_str(), std::move(args));

        if (obj.contains("err_mesg"))
        {
            pack.set_err_mesg(obj.at("err_mesg").get_string().c_str());
        }

        return pack;
    }
    else
    {
        if (obj.contains("result") && !obj.at("result").is_null())
        {
            const auto& result = obj.at("result");

            if constexpr (std::is_arithmetic_v<R>)
            {
                return packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
                    bjson::value_to<R>(result), std::move(args));
            }
            else if constexpr (std::is_same_v<R, std::string>)
            {
                return packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
                    result.get_string().c_str(), std::move(args));
            }
            else if constexpr (details::is_container_v<R>)
            {
                using value_t = typename R::value_type;

                auto& arr = result.as_array();
                R container;
                container.reserve(arr.size());

                unsigned j = 0;

                for (const auto& val : arr)
                {
                    container.push_back(parse_arg<value_t>(val, j));
                }

                return packed_func<R, Args...>(
                    obj.at("func_name").get_string().c_str(), container, std::move(args));
            }
            else if constexpr (details::is_serializable_v<bjson_adapter, R>)
            {
                return packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
                    R::template deserialize<bjson_adapter>(result), std::move(args));
            }
            else
            {
                return packed_func<R, Args...>(obj.at("func_name").get_string().c_str(),
                    bjson_adapter::template deserialize<R>(result), std::move(args));
            }
        }

        packed_func<R, Args...> pack(
            obj.at("func_name").get_string().c_str(), std::nullopt, std::move(args));

        if (obj.contains("err_mesg"))
        {
            pack.set_err_mesg(obj.at("err_mesg").get_string().c_str());
        }

        return pack;
    }
}

template<>
inline std::string pack_adapter<adapters::bjson_adapter>::get_func_name(
    const adapters::bjson_val& serial_obj)
{
    assert(serial_obj.is_object());
    return serial_obj.at("func_name").get_string().c_str();
}

template<>
inline void pack_adapter<adapters::bjson_adapter>::set_err_mesg(
    adapters::bjson_val& serial_obj, std::string&& mesg)
{
    assert(serial_obj.is_object());
    auto& obj = serial_obj.as_object();
    obj["err_mesg"] = std::move(mesg);
}
} // namespace rpc
