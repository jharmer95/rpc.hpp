///@file rpc_adapters/rpc_rapidjson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting rapidjson (https://github.com/Tencent/rapidjson)
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

namespace rpc
{
namespace adapters
{
    using rapidjson_adapter = details::serial_adapter<rapidjson::Document, std::string>;

    namespace rapidjson
    {
        using doc_t = ::rapidjson::Document;
        using value_t = ::rapidjson::Value;

        namespace details
        {
            template<typename T>
            [[nodiscard]] constexpr bool validate_arg(const value_t& arg)
            {
                if constexpr (std::is_same_v<T, bool>)
                {
                    return arg.IsBool();
                }
                else if constexpr (std::is_integral_v<T>)
                {
                    if constexpr (std::is_signed_v<T>)
                    {
                        if constexpr (sizeof(T) < 8)
                        {
                            return arg.IsInt();
                        }
                        else
                        {
                            return arg.IsInt64();
                        }
                    }
                    else
                    {
                        if constexpr (sizeof(T) < 8)
                        {
                            return arg.IsUint();
                        }
                        else
                        {
                            return arg.IsUint64();
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, float>)
                {
                    return arg.IsFloat();
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    return arg.IsDouble();
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return arg.IsString();
                }
                else if constexpr (rpc::details::is_container_v<T>)
                {
                    return arg.IsArray();
                }
                else
                {
                    return !arg.IsNull();
                }
            }

            [[noreturn]] inline void throw_mismatch(
                std::string&& expect_type, const value_t& obj) noexcept(false)
            {
                std::string type_str = [&obj]
                {
                    if (obj.IsNull())
                    {
                        return "null";
                    }

                    if (obj.IsInt64())
                    {
                        return "int64";
                    }

                    if (obj.IsInt())
                    {
                        return "int";
                    }

                    if (obj.IsUint64())
                    {
                        return "uint64";
                    }

                    if (obj.IsUint())
                    {
                        return "uint";
                    }

                    if (obj.IsDouble())
                    {
                        return "double";
                    }

                    if (obj.IsFloat())
                    {
                        return "float";
                    }

                    if (obj.IsBool())
                    {
                        return "bool";
                    }

                    if (obj.IsString())
                    {
                        return "string";
                    }

                    if (obj.IsArray())
                    {
                        return "array";
                    }

                    if (obj.IsObject())
                    {
                        return "object";
                    }

                    return "unknown";
                }();

                throw exceptions::function_mismatch("rapidjson expected type: "
                    + std::move(expect_type) + ", got type: " + std::move(type_str));
            }

            template<typename T>
            void push_args(T&& arg, value_t& obj_arr, ::rapidjson::MemoryPoolAllocator<>& alloc);

            template<typename T>
            void push_arg(T&& arg, value_t& obj, ::rapidjson::MemoryPoolAllocator<>& alloc)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if constexpr (std::is_same_v<no_ref_t, std::string>)
                {
                    obj.SetString(arg.c_str(), alloc);
                }
                else if constexpr (std::is_arithmetic_v<no_ref_t>)
                {
                    // Rapidjson is silly and does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
                    if constexpr (
                        std::is_same_v<no_ref_t,
                            char> || std::is_same_v<no_ref_t, int8_t> || std::is_same_v<no_ref_t, int16_t>)
                    {
                        obj.SetInt(arg);
                    }
                    else if constexpr (std::is_same_v<no_ref_t,
                                           uint8_t> || std::is_same_v<no_ref_t, uint16_t>)
                    {
                        obj.SetUint(arg);
                    }
                    else
                    {
                        obj.Set<no_ref_t>(arg);
                    }
                }
                else if constexpr (rpc::details::is_container_v<no_ref_t>)
                {
                    obj.SetArray();
                    obj.Reserve(static_cast<::rapidjson::SizeType>(arg.size()), alloc);

                    for (auto&& val : arg)
                    {
                        push_args(std::forward<decltype(val)>(val), obj, alloc);
                    }
                }
                else if constexpr (rpc::details::is_serializable_v<rapidjson_adapter, no_ref_t>)
                {
                    doc_t serialized =
                        no_ref_t::template serialize<rapidjson_adapter>(std::forward<T>(arg));

                    obj.CopyFrom(serialized, alloc);
                }
                else
                {
                    doc_t serialized =
                        rapidjson_adapter::template serialize<no_ref_t>(std::forward<T>(arg));

                    obj.CopyFrom(serialized, alloc);
                }
            }

            template<typename T>
            void push_args(T&& arg, value_t& obj_arr, ::rapidjson::MemoryPoolAllocator<>& alloc)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                value_t tmp{};
                push_arg(std::forward<T>(arg), tmp, alloc);
                obj_arr.PushBack(std::move(tmp), alloc);
            }

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_args(
                const value_t& arg_arr, unsigned& index);

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_arg(const value_t& arg)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if (!validate_arg<no_ref_t>(arg))
                {
                    throw_mismatch(typeid(no_ref_t).name(), arg);
                }

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
                        return static_cast<no_ref_t>(arg.GetInt());
                    }
                    else if constexpr (std::is_same_v<no_ref_t,
                                           uint8_t> || std::is_same_v<no_ref_t, uint16_t>)
                    {
                        return static_cast<no_ref_t>(arg.GetUint());
                    }
                    else
                    {
                        return arg.Get<no_ref_t>();
                    }
                }
                else if constexpr (rpc::details::is_container_v<no_ref_t>)
                {
                    using subvalue_t = typename no_ref_t::value_type;

                    no_ref_t container{};
                    container.reserve(arg.Size());
                    unsigned j = 0;

                    for (const auto& val : arg.GetArray())
                    {
                        container.push_back(parse_args<subvalue_t>(val, j));
                    }

                    return container;
                }
                else if constexpr (rpc::details::is_serializable_v<rapidjson_adapter, no_ref_t>)
                {
                    doc_t d{};
                    d.CopyFrom(arg, d.GetAllocator());
                    return no_ref_t::template deserialize<rapidjson_adapter>(d);
                }
                else
                {
                    doc_t d{};
                    d.CopyFrom(arg, d.GetAllocator());
                    return rapidjson_adapter::template deserialize<no_ref_t>(d);
                }
            }

            template<typename T>
            [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> parse_args(
                const value_t& arg_arr, unsigned& index)
            {
                using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

                if (arg_arr.IsArray() && index >= arg_arr.GetArray().Size())
                {
                    throw exceptions::function_mismatch("Argument count mismatch");
                }

                const value_t& arg = arg_arr.IsArray() ? arg_arr.GetArray()[index++] : arg_arr;
                return parse_arg<T>(arg);
            }
        } // namespace details
    }     // namespace rapidjson
} // namespace adapters

template<>
[[nodiscard]] inline std::string adapters::rapidjson_adapter::to_bytes(
    adapters::rapidjson::doc_t&& serial_obj)
{
    rapidjson::StringBuffer buffer{};
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    std::move(serial_obj).Accept(writer);
    return buffer.GetString();
}

template<>
[[nodiscard]] inline adapters::rapidjson::doc_t adapters::rapidjson_adapter::from_bytes(
    std::string&& bytes)
{
    adapters::rapidjson::doc_t d{};
    d.SetObject();
    d.Parse(std::move(bytes).c_str());
    return d;
}

template<>
template<typename R, typename... Args>
[[nodiscard]] adapters::rapidjson::doc_t pack_adapter<adapters::rapidjson_adapter>::serialize_pack(
    const ::rpc::details::packed_func<R, Args...>& pack)
{
    using namespace adapters::rapidjson;

    doc_t d{};
    auto& alloc = d.GetAllocator();
    d.SetObject();

    d.AddMember("func_name", value_t{}.SetString(pack.get_func_name().c_str(), alloc), alloc);

    if constexpr (!std::is_void_v<R>)
    {
        value_t result{};

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
            else if constexpr (rpc::details::is_container_v<R>)
            {
                const auto& container = pack.get_result();
                result.SetArray();
                result.Reserve(static_cast<::rapidjson::SizeType>(container.size()), alloc);

                for (const auto& val : container)
                {
                    result.PushBack(val, alloc);
                }
            }
            else if constexpr (rpc::details::is_serializable_v<adapters::rapidjson_adapter, R>)
            {
                doc_t tmp = R::template serialize<adapters::rapidjson_adapter>(pack.get_result());
                result.CopyFrom(tmp, alloc);
            }
            else
            {
                doc_t tmp = adapters::rapidjson_adapter::template serialize<R>(pack.get_result());
                result.CopyFrom(tmp, alloc);
            }
        }
        else
        {
            result.SetNull();
        }

        d.AddMember("result", result, alloc);
    }

    value_t args{};
    args.SetArray();
    args.Reserve(static_cast<::rapidjson::SizeType>(sizeof...(Args)), alloc);
    const auto& argTup = pack.get_args();

    rpc::details::for_each_tuple(argTup,
        [&args, &alloc](auto&& x)
        { adapters::rapidjson::details::push_args(std::forward<decltype(x)>(x), args, alloc); });

    d.AddMember("args", std::move(args), alloc);
    return d;
}

template<>
template<typename R, typename... Args>
[[nodiscard]] ::rpc::details::packed_func<R, Args...> pack_adapter<
    adapters::rapidjson_adapter>::deserialize_pack(const adapters::rapidjson::doc_t& serial_obj)
{
    using namespace adapters::rapidjson;

    [[maybe_unused]] unsigned i = 0;

    typename ::rpc::details::packed_func<R, Args...>::args_t args{
        adapters::rapidjson::details::parse_args<Args>(serial_obj["args"], i)...
    };

    if constexpr (std::is_void_v<R>)
    {
        ::rpc::details::packed_func<void, Args...> pack(
            serial_obj["func_name"].GetString(), std::move(args));

        if (serial_obj.HasMember("except_type"))
        {
            pack.set_exception(serial_obj["err_mesg"].GetString(),
                static_cast<rpc::exceptions::Type>(serial_obj["except_type"].GetInt()));
        }

        return pack;
    }
    else
    {
        if (serial_obj.HasMember("result") && !serial_obj["result"].IsNull())
        {
            const value_t& result = serial_obj["result"];

            return ::rpc::details::packed_func<R, Args...>(serial_obj["func_name"].GetString(),
                adapters::rapidjson::details::parse_arg<R>(result), std::move(args));
        }

        ::rpc::details::packed_func<R, Args...> pack(
            serial_obj["func_name"].GetString(), std::nullopt, std::move(args));

        if (serial_obj.HasMember("except_type"))
        {
            pack.set_exception(serial_obj["err_mesg"].GetString(),
                static_cast<rpc::exceptions::Type>(serial_obj["except_type"].GetInt()));
        }

        return pack;
    }
}

template<>
[[nodiscard]] inline std::string pack_adapter<adapters::rapidjson_adapter>::get_func_name(
    const adapters::rapidjson::doc_t& serial_obj)
{
    return serial_obj["func_name"].GetString();
}

template<>
inline void pack_adapter<adapters::rapidjson_adapter>::set_exception(
    adapters::rapidjson::doc_t& serial_obj, const rpc::exceptions::rpc_exception& ex)
{
    auto& alloc = serial_obj.GetAllocator();

    if (serial_obj.HasMember("except_type"))
    {
        serial_obj["except_type"].SetInt(static_cast<int>(ex.get_type()));
        serial_obj["err_mesg"].SetString(ex.what(), alloc);
    }
    else
    {
        serial_obj.AddMember("except_type", static_cast<int>(ex.get_type()), alloc);
        serial_obj.AddMember(
            "err_mesg", adapters::rapidjson::value_t{}.SetString(ex.what(), alloc), alloc);
    }
}
} // namespace rpc
