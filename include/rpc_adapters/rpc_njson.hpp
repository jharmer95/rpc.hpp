///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.1.0.0
///@date 08-25-2020
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

#include <nlohmann/json.hpp>

#include "../rpc.hpp"

using njson = nlohmann::json;

using njson_adapter = rpc::serial_adapter<njson>;

template<>
template<typename T>
T njson_adapter::packArg(const njson& obj, unsigned& i)
{
    // TODO: Address cases where pointer, container, or custom type is used
    return obj["args"][i++].get<T>();
}

template<>
template<typename T, typename R, typename... Args>
T njson_adapter::unpackArg(const packed_func<R, Args...>& pack, unsigned& i)
{
    // TODO: Address cases where pointer, container, or custom type is used
    return pack.template get_arg<T>(i++);
}

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> njson_adapter::to_packed_func(const njson& serial_obj)
{
    unsigned i = 0;

    // TODO: Address cases where pointer, container, or custom type is used
    std::array<std::any, sizeof...(Args)> args{ packArg<Args>(serial_obj, i)... };

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

template<>
template<typename R, typename... Args>
njson njson_adapter::from_packed_func(const packed_func<R, Args...>& pack)
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

    // TODO: Address cases where pointer, container, or custom type is used
    std::tuple<Args...> argTup{ unpackArg<Args, R, Args...>(pack, i)... };
    rpc::for_each_tuple(argTup, [&ret_j](auto x) { ret_j["args"].push_back(x); });

    return ret_j;
}

template<>
std::string njson_adapter::to_string(const njson& serial_obj)
{
    return serial_obj.dump();
}

template<>
njson njson_adapter::from_string(const std::string& str)
{
    return njson::parse(str);
}

template<>
std::string njson_adapter::extract_func_name(const njson& obj)
{
    return obj["func_name"].get<std::string>();
}
