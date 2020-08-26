///@file rpc_adapters/rpc_rapidjson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting rapidjson (https://github.com/Tencent/rapidjson)
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

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using rpdjson_doc = rapidjson::Document;
using rpdjson_val = rapidjson::Value;
using rpdjson_adapter = rpc::serial_adapter<rpdjson_doc>;

template<>
template<typename T>
T rpdjson_adapter::packArg(const rpdjson_doc& doc, unsigned& i)
{
    const auto& docArgs = doc.FindMember("args")->value.GetArray();
    return docArgs[i++].Get<T>();
}

template<>
template<typename T, typename R, typename... Args>
T rpdjson_adapter::unpackArg(const packed_func<R, Args...>& pack, unsigned& i)
{
    return pack.template get_arg<T>(i++);
}

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> rpdjson_adapter::to_packed_func(const rpdjson_doc& serial_obj)
{
    unsigned i = 0;

    // TODO: Address cases where pointer, container, or custom type is used
    std::array<std::any, sizeof...(Args)> args{ packArg<Args>(serial_obj, i)... };

    if (serial_obj.HasMember("result"))
    {
        const rpdjson_val& result = serial_obj["result"];
        return packed_func<R, Args...>(serial_obj["func_name"].GetString(), result.Get<R>(), args);
    }

    return packed_func<R, Args...>(serial_obj["func_name"].GetString(), std::nullopt, args);
}

template<>
template<typename R, typename... Args>
rpdjson_doc rpdjson_adapter::from_packed_func(const packed_func<R, Args...>& pack)
{
    rpdjson_doc d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    rpdjson_val func_name;
    func_name.SetString(pack.get_func_name().c_str(), alloc);
    d.AddMember("func_name", func_name, alloc);

    rpdjson_val result;

    if (pack)
    {
        result.Set<R>(*pack.get_result());
    }
    else
    {
        result.Set<R>({});
    }

    d.AddMember("result", result, alloc);

    rpdjson_val args;
    args.SetArray();
    unsigned i = 0;

    // TODO: Address cases where pointer, container, or custom type is used
    std::tuple<Args...> argTup{ unpackArg<Args, R, Args...>(pack, i)... };
    rpc::for_each_tuple(argTup, [&args, &alloc](auto x) {
        args.PushBack(rpdjson_val().Set<decltype(x)>(x), alloc);
    });

    d.AddMember("args", args, alloc);
    return d;
}

template<>
std::string rpdjson_adapter::to_string(const rpdjson_doc& serial_obj)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serial_obj.Accept(writer);
    return buffer.GetString();
}

template<>
rpdjson_doc rpdjson_adapter::from_string(const std::string& str)
{
    rpdjson_doc d;
    d.Parse(str.c_str());
    return d;
}
