///@file rpc_adapters/rpc_rapidjson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting rapidjson (https://github.com/Tencent/rapidjson)
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
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

#ifndef RPC_ADAPTERS_RAPIDJSON_HPP
#define RPC_ADAPTERS_RAPIDJSON_HPP

#include "../rpc.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace rpc_hpp::adapters
{
namespace detail_rapidjson
{
    class serializer;
    class deserializer;

    // invariants: none
    struct adapter_impl
    {
        using bytes_t = std::string;
        using serial_t = rapidjson::Document;
        using serializer_t = serializer;
        using deserializer_t = deserializer;
        using config = void;
    };

    // invariants: none
    class serial_adapter : public serial_adapter_base<adapter_impl>
    {
    public:
        [[nodiscard]] static auto is_empty(const serial_t& serial_obj) noexcept -> bool;
        [[nodiscard]] static auto from_bytes(bytes_t&& bytes) -> serial_t;
        [[nodiscard]] static auto to_bytes(const serial_t& serial_obj) -> bytes_t;
        [[nodiscard]] static auto to_bytes(serial_t&& serial_obj) -> bytes_t;
        [[nodiscard]] static auto get_func_name(const serial_t& serial_obj) -> std::string;
        [[nodiscard]] static auto get_type(const serial_t& serial_obj) -> rpc_type;

        template<bool IsCallback, typename R>
        [[nodiscard]] static auto get_result(const serial_t& serial_obj)
            -> detail::rpc_result<IsCallback, R>;

        template<bool IsCallback, typename R>
        [[nodiscard]] static auto serialize_result(const detail::rpc_result<IsCallback, R>& result)
            -> serial_t;

        template<bool IsCallback, typename R, typename... Args>
        [[nodiscard]] static auto get_result_w_bind(const serial_t& serial_obj)
            -> detail::rpc_result_w_bind<IsCallback, R, Args...>;

        template<bool IsCallback, typename R, typename... Args>
        [[nodiscard]] static auto serialize_result_w_bind(
            const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) -> serial_t;

        template<bool IsCallback, typename... Args>
        [[nodiscard]] static auto get_request(const serial_t& serial_obj)
            -> detail::rpc_request<IsCallback, Args...>;

        template<bool IsCallback, typename... Args>
        [[nodiscard]] static auto serialize_request(
            const detail::rpc_request<IsCallback, Args...>& request) -> serial_t;

        template<bool IsCallback>
        [[nodiscard]] static auto get_error(const serial_t& serial_obj)
            -> detail::rpc_error<IsCallback>;

        template<bool IsCallback>
        [[nodiscard]] static auto serialize_error(const detail::rpc_error<IsCallback>& error)
            -> serial_t;

        [[nodiscard]] static auto get_callback_install(const serial_t& serial_obj)
            -> callback_install_request;

        [[nodiscard]] static auto serialize_callback_install(
            const callback_install_request& callback_req) -> serial_t;

        [[nodiscard]] static auto has_bound_args(const serial_t& serial_obj) -> bool;

    private:
        [[nodiscard]] static auto verify_type(const serial_t& serial_obj, rpc_type type) -> bool;
    };

    // invariants:
    //   1.) m_json must not be empty when fetched via 'object()'
    class serializer : public serializer_base<serial_adapter, false>
    {
    public:
        serializer() noexcept = default;

        [[nodiscard]] auto object() const& noexcept -> const rapidjson::Document&
        {
            RPC_HPP_PRECONDITION(m_json.IsObject() ? !m_json.ObjectEmpty()
                                                   : (m_json.IsArray() ? !m_json.Empty() : true));

            return m_json;
        }

        [[nodiscard]] auto object() && noexcept -> rapidjson::Document&&
        {
            RPC_HPP_PRECONDITION(m_json.IsObject() ? !m_json.ObjectEmpty()
                                                   : (m_json.IsArray() ? !m_json.Empty() : true));

            return std::move(m_json);
        }

        template<typename T>
        void as_bool(const std::string_view key, const T& val)
        {
            subobject(key).SetBool(val);
        }

        template<typename T>
        void as_float(const std::string_view key, const T& val)
        {
            subobject(key).Set<T>(val);
        }

        template<typename T>
        void as_int(const std::string_view key, const T& val)
        {
            if constexpr (std::is_enum_v<T>)
            {
                subobject(key).SetInt(static_cast<int>(val));
            }
            else if constexpr (sizeof(T) < sizeof(int))
            {
                if constexpr (std::is_unsigned_v<T>)
                {
                    subobject(key).SetUint(val);
                }
                else
                {
                    subobject(key).SetInt(val);
                }
            }
            else
            {
                subobject(key).Set<T>(val);
            }
        }

        template<typename T>
        void as_string(const std::string_view key, const T& val)
        {
            subobject(key).SetString(std::data(val), allocator());
        }

        template<typename T>
        void as_array(const std::string_view key, const T& val)
        {
            auto& arr = subobject(key).SetArray();

            if constexpr (detail::has_size<T>::value)
            {
                arr.Reserve(static_cast<rapidjson::SizeType>(val.size()), allocator());
            }

            for (const auto& subval : val)
            {
                if constexpr (detail::is_stringlike_v<detail::remove_cvref_t<decltype(subval)>>)
                {
                    arr.PushBack(
                        rapidjson::Value{}.SetString(std::data(subval),
                            static_cast<rapidjson::SizeType>(std::size(subval)), allocator()),
                        allocator());
                }
                else
                {
                    arr.PushBack(subval, allocator());
                }
            }
        }

        template<typename T>
        void as_map(const std::string_view key, const T& val)
        {
            auto& obj = subobject(key).SetObject();

            for (const auto& [k, v] : val)
            {
                const auto key_str = key_string(k);
                serializer ser{};
                ser.serialize_object(v);
                obj.AddMember(rapidjson::Value{}.SetString(key_str.c_str(), allocator()),
                    std::move(ser).object(), allocator());
            }
        }

        template<typename T>
        void as_multimap(const std::string_view key, const T& val)
        {
            auto& obj = subobject(key).SetObject();

            for (const auto& [k, v] : val)
            {
                const auto key_str = key_string(k);

                if (!obj.HasMember(key_str.c_str()))
                {
                    obj.AddMember(rapidjson::Value{}.SetString(key_str.c_str(), allocator()),
                        rapidjson::Value{}.SetArray(), allocator());
                }

                serializer ser{};
                ser.serialize_object(v);
                obj[key_str.c_str()].PushBack(std::move(ser).object(), allocator());
            }
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, const std::pair<T1, T2>& val)
        {
            auto& obj = subobject(key).SetObject();
            serializer ser1{};
            ser1.serialize_object(val.first);
            obj.AddMember("first", std::move(ser1).object(), allocator());
            serializer ser2{};
            ser2.serialize_object(val.second);
            obj.AddMember("second", std::move(ser2).object(), allocator());
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, const std::tuple<Args...>& val)
        {
            auto& arg_arr = subobject(key).SetArray();
            arg_arr.Reserve(static_cast<rapidjson::SizeType>(sizeof...(Args)), allocator());
            detail::for_each_tuple(val,
                [this, &arg_arr](auto&& elem)
                { push_args(std::forward<decltype(elem)>(elem), arg_arr, allocator()); });
        }

        template<typename T>
        void as_optional(const std::string_view key, const std::optional<T>& val)
        {
            if (val.has_value())
            {
                push_arg(val.value(), subobject(key), allocator());
            }
            else
            {
                subobject(key).SetNull();
            }
        }

        template<typename T>
        void as_object(const std::string_view key, const T& val)
        {
            push_arg(val, subobject(key), allocator());
        }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) -> rapidjson::Value&
        {
            if (key.empty())
            {
                return m_json;
            }

            if (!m_json.IsObject())
            {
                m_json.SetObject();
            }

            const std::string key_str{ key };
            m_json.AddMember(rapidjson::Value{}.SetString(key_str.c_str(), allocator()),
                rapidjson::Value{}, allocator());

            return m_json[key_str.c_str()];
        }

        template<typename K>
        [[nodiscard]] auto key_string(const K& key_val) -> std::string
        {
            serializer ser{};
            ser.serialize_object(key_val);
            rapidjson::StringBuffer buffer{};
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            std::move(ser).object().Accept(writer);
            return buffer.GetString();
        }

        template<typename T>
        static void push_arg(
            T&& arg, rapidjson::Value& obj, rapidjson::MemoryPoolAllocator<>& alloc)
        {
            serializer ser{};
            ser.serialize_object(std::forward<T>(arg));
            obj.CopyFrom(std::move(ser).object(), alloc);
        }

        template<typename T>
        static void push_args(
            T&& arg, rapidjson::Value& obj_arr, rapidjson::MemoryPoolAllocator<>& alloc)
        {
            rapidjson::Value tmp{};
            push_arg(std::forward<T>(arg), tmp, alloc);
            obj_arr.PushBack(std::move(tmp), alloc);
        }

        [[nodiscard]] auto allocator() -> rapidjson::MemoryPoolAllocator<>&
        {
            return m_json.GetAllocator();
        }

        rapidjson::Document m_json{};
    };

    // invariants:
    //   1.) m_json cannot be empty
    //   2.) m_json must be an object if a subval is accessed
    class deserializer : public serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const rapidjson::Value& obj) noexcept : m_json(obj)
        {
            RPC_HPP_POSTCONDITION(m_json.IsObject() ? !m_json.ObjectEmpty()
                                                    : (m_json.IsArray() ? !m_json.Empty() : true));
        }

        template<typename T>
        void as_bool(const std::string_view key, T& val) const
        {
            val = subobject(key).GetBool();
        }

        template<typename T>
        void as_float(const std::string_view key, T& val) const
        {
            static_assert(sizeof(T) <= sizeof(double), "long double not supported");
            val = subobject(key).Get<T>();
        }

        template<typename T>
        void as_int(const std::string_view key, T& val) const
        {
            if constexpr (std::is_enum_v<T>)
            {
                val = static_cast<T>(subobject(key).GetInt());
            }
            else if constexpr (sizeof(T) < sizeof(int))
            {
                // Rapidjson does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
                if constexpr (std::is_unsigned_v<T>)
                {
                    val = static_cast<T>(subobject(key).GetUint());
                }
                else
                {
                    val = static_cast<T>(subobject(key).GetInt());
                }
            }
            else
            {
                val = subobject(key).Get<T>();
            }
        }

        template<typename T>
        void as_string(const std::string_view key, T& val) const
        {
            val = subobject(key).GetString();
        }

        template<typename T>
        void as_array(const std::string_view key, T& val) const
        {
            RPC_HPP_PRECONDITION(std::size(val) == 0);

            const auto& arr = subobject(key).GetArray();
            std::transform(arr.begin(), arr.end(), std::inserter(val, val.end()),
                yield_value<detail::remove_cvref_t<typename T::value_type>>);
        }

        template<typename T, size_t N>
        void as_array(const std::string_view key, std::array<T, N>& val) const
        {
            const auto& arr = subobject(key).GetArray();

            if (arr.Size() != N)
            {
                throw std::out_of_range{ "rapidjson error: array out of bounds" };
            }

            std::transform(
                arr.begin(), arr.end(), val.begin(), yield_value<detail::remove_cvref_t<T>>);
        }

        template<typename T, typename Alloc>
        void as_array(const std::string_view key, std::forward_list<T, Alloc>& val) const
        {
            RPC_HPP_PRECONDITION(val.empty());

            using rev_iter_t = std::reverse_iterator<
                rapidjson::GenericArray<true, rapidjson::Value>::ValueIterator>;

            const auto& arr = subobject(key).GetArray();
            const rev_iter_t arr_rbegin{ arr.end() };
            const rev_iter_t arr_rend{ arr.begin() };

            for (auto it = arr_rbegin; it != arr_rend; ++it)
            {
                val.push_front(yield_value<detail::remove_cvref_t<T>>(*it));
            }
        }

        template<typename T>
        void as_map(const std::string_view key, T& val) const
        {
            RPC_HPP_PRECONDITION(std::size(val) == 0);

            const auto& obj = subobject(key).GetObj();
            const auto mem_end = obj.MemberEnd();

            for (auto it = obj.MemberBegin(); it != mem_end; ++it)
            {
                rapidjson::Document doc{};
                doc.Parse(it->name.GetString());
                val.insert({ yield_value<typename T::key_type>(doc),
                    yield_value<typename T::mapped_type>(it->value) });
            }
        }

        template<typename T>
        void as_multimap(const std::string_view key, T& val) const
        {
            RPC_HPP_PRECONDITION(std::size(val) == 0);

            const auto& obj = subobject(key).GetObj();
            const auto mem_end = obj.MemberEnd();

            for (auto it = obj.MemberBegin(); it != mem_end; ++it)
            {
                rapidjson::Document doc{};
                doc.Parse(it->name.GetString());
                const auto& val_arr = it->value.GetArray();

                for (const auto& subval : val_arr)
                {
                    val.insert({ yield_value<typename T::key_type>(doc),
                        yield_value<typename T::mapped_type>(subval) });
                }
            }
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, std::pair<T1, T2>& val) const
        {
            const auto& obj = subobject(key).GetObj();
            val.first = parse_arg<T1>(obj["first"]);
            val.second = parse_arg<T2>(obj["second"]);
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, std::tuple<Args...>& val) const
        {
            if (subobject(key).GetArray().Size() != sizeof...(Args))
            {
                throw function_mismatch_error{ "rapidjson error: invalid number of args" };
            }

            [[maybe_unused]] rapidjson::SizeType arg_counter = 0;
            val = { parse_args<Args>(subobject(key), arg_counter)... };
        }

        template<typename T>
        void as_optional(const std::string_view key, std::optional<T>& val) const
        {
            const auto& obj = subobject(key);
            val = obj.IsNull() ? std::optional<T>{ std::nullopt }
                               : std::optional<T>{ std::in_place, parse_arg<T>(obj) };
        }

        template<typename T>
        void as_object(const std::string_view key, T& val) const
        {
            val = parse_arg<T>(subobject(key));
        }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) const -> const rapidjson::Value&
        {
            RPC_HPP_PRECONDITION(key.empty() || m_json.IsObject());

            if (key.empty())
            {
                return m_json;
            }

            return m_json[std::string{ key }.c_str()];
        }

        template<typename T>
        RPC_HPP_NODISCARD("this function is pointless without checking the bool")
        static constexpr auto validate_arg(const rapidjson::Value& arg) noexcept -> bool
        {
            if constexpr (detail::is_optional_v<T>)
            {
                return arg.IsNull() || validate_arg<typename T::value_type>(arg);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return arg.IsBool();
            }
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                {
                    if constexpr (sizeof(T) < sizeof(int64_t))
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
                    if constexpr (sizeof(T) < sizeof(uint64_t))
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
            else if constexpr (detail::is_stringlike_v<T>)
            {
                return arg.IsString();
            }
            else if constexpr (detail::is_map_v<T>)
            {
                return arg.IsObject();
            }
            else if constexpr (rpc_hpp::detail::is_container_v<T>)
            {
                return arg.IsArray();
            }
            else
            {
                return !arg.IsNull();
            }
        }

        static auto mismatch_message(
            const std::string_view expect_type, const rapidjson::Value& obj) -> std::string
        {
            const auto get_type_str = [&obj]() noexcept
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
            };

            return std::string{ "rapidjson error: expected type: " }
                .append(expect_type)
                .append(", got type: ")
                .append(get_type_str());
        }

        template<typename T>
        RPC_HPP_NODISCARD(
            "parsing can be expensive, and it makes no sense to not use the parsed result")
        static auto parse_arg(const rapidjson::Value& arg)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

            if (!validate_arg<no_ref_t>(arg))
            {
                throw function_mismatch_error{ mismatch_message(typeid(no_ref_t).name(), arg) };
            }

            no_ref_t out_val;
            deserializer ser{ arg };
            ser.deserialize_object(out_val);
            return out_val;
        }

        template<typename T>
        RPC_HPP_NODISCARD(
            "parsing can be expensive, and it makes no sense to not use the parsed result")
        static auto parse_args(const rapidjson::Value& arg_arr, rapidjson::SizeType& index)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            if (!arg_arr.IsArray())
            {
                return parse_arg<T>(arg_arr);
            }

            const auto& arr = arg_arr.GetArray();

            if (index >= arr.Size())
            {
                throw function_mismatch_error("rapidjson error: argument count mismatch");
            }

            const auto old_idx = index;
            ++index;
            return parse_arg<T>(arr[old_idx]);
        }

        template<typename T>
        static auto yield_value(const rapidjson::Value& val) -> T
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                return val.Get<T>();
            }
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (sizeof(T) < sizeof(int))
                {
                    if constexpr (std::is_unsigned_v<T>)
                    {
                        return static_cast<T>(val.GetUint());
                    }
                    else
                    {
                        return static_cast<T>(val.GetInt());
                    }
                }
                else
                {
                    return val.Get<T>();
                }
            }
            else if constexpr (detail::is_stringlike_v<T>)
            {
                return val.GetString();
            }
            else
            {
                T tmp_val;
                deserializer ser{ val };
                ser.deserialize_object(tmp_val);
                return tmp_val;
            }
        }

        const rapidjson::Value& m_json;
    };

    inline auto serial_adapter::is_empty(const rapidjson::Document& serial_obj) noexcept -> bool
    {
        return (!serial_obj.IsObject()) || serial_obj.ObjectEmpty();
    }

    inline auto serial_adapter::from_bytes(std::string&& bytes) -> rapidjson::Document
    {
        rapidjson::Document doc{};
        doc.SetObject();
        doc.Parse(std::move(bytes).c_str());

        if (doc.HasParseError())
        {
            throw deserialization_error{ "rapidjson error: parsing error occurred" };
        }

        if (const auto fname_it = doc.FindMember("func_name");
            (fname_it == doc.MemberEnd()) || (!fname_it->value.IsString()))
        {
            throw deserialization_error{ R"(rapidjson error: field "func_name" not found)" };
        }

        RPC_HPP_POSTCONDITION(!is_empty(doc));
        return doc;
    }

    inline auto serial_adapter::to_bytes(const rapidjson::Document& serial_obj) -> std::string
    {
        rapidjson::StringBuffer buffer{};
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        serial_obj.Accept(writer);
        return buffer.GetString();
    }

    inline auto serial_adapter::to_bytes(rapidjson::Document&& serial_obj) -> std::string
    {
        rapidjson::StringBuffer buffer{};
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        std::move(serial_obj).Accept(writer);
        return buffer.GetString();
    }

    inline auto serial_adapter::get_func_name(const rapidjson::Document& serial_obj) -> std::string
    {
        RPC_HPP_PRECONDITION(!is_empty(serial_obj));
        return serial_obj["func_name"].GetString();
    }

    inline auto serial_adapter::get_type(const rapidjson::Document& serial_obj) -> rpc_type
    {
        RPC_HPP_PRECONDITION(!is_empty(serial_obj));
        return static_cast<rpc_type>(serial_obj["type"].GetInt());
    }

    template<bool IsCallback, typename R>
    auto serial_adapter::get_result(const rapidjson::Document& serial_obj)
        -> detail::rpc_result<IsCallback, R>
    {
        RPC_HPP_PRECONDITION(verify_type(
            serial_obj, IsCallback ? rpc_type::callback_result : rpc_type::func_result));

        detail::rpc_result<IsCallback, R> result;
        deserializer ser{ serial_obj };
        ser.deserialize_object(result);
        return result;
    }

    template<bool IsCallback, typename R>
    auto serial_adapter::serialize_result(const detail::rpc_result<IsCallback, R>& result)
        -> rapidjson::Document
    {
        serializer ser{};
        ser.serialize_object(result);
        return std::move(ser).object();
    }

    template<bool IsCallback, typename R, typename... Args>
    auto serial_adapter::get_result_w_bind(const rapidjson::Document& serial_obj)
        -> detail::rpc_result_w_bind<IsCallback, R, Args...>
    {
        RPC_HPP_PRECONDITION(verify_type(serial_obj,
            IsCallback ? rpc_type::callback_result_w_bind : rpc_type::func_result_w_bind));

        detail::rpc_result_w_bind<IsCallback, R, Args...> result;
        deserializer ser{ serial_obj };
        ser.deserialize_object(result);
        return result;
    }

    template<bool IsCallback, typename R, typename... Args>
    auto serial_adapter::serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) -> rapidjson::Document
    {
        serializer ser{};
        ser.serialize_object(result);
        return std::move(ser).object();
    }

    template<bool IsCallback, typename... Args>
    auto serial_adapter::get_request(const rapidjson::Document& serial_obj)
        -> detail::rpc_request<IsCallback, Args...>
    {
        RPC_HPP_PRECONDITION(verify_type(serial_obj,
                                 IsCallback ? rpc_type::callback_request : rpc_type::func_request)
            || verify_type(serial_obj,
                IsCallback ? rpc_type::callback_result_w_bind : rpc_type::func_result_w_bind));

        detail::rpc_request<IsCallback, Args...> request;
        deserializer ser{ serial_obj };
        ser.deserialize_object(request);
        return request;
    }

    template<bool IsCallback, typename... Args>
    auto serial_adapter::serialize_request(const detail::rpc_request<IsCallback, Args...>& request)
        -> rapidjson::Document
    {
        serializer ser{};
        ser.serialize_object(request);
        return std::move(ser).object();
    }

    template<bool IsCallback>
    auto serial_adapter::get_error(const rapidjson::Document& serial_obj)
        -> detail::rpc_error<IsCallback>
    {
        RPC_HPP_PRECONDITION(
            verify_type(serial_obj, IsCallback ? rpc_type::callback_error : rpc_type::func_error));

        detail::rpc_error<IsCallback> error;
        deserializer ser{ serial_obj };
        ser.deserialize_object(error);
        return error;
    }

    template<bool IsCallback>
    auto serial_adapter::serialize_error(const detail::rpc_error<IsCallback>& error)
        -> rapidjson::Document
    {
        serializer ser{};
        ser.serialize_object(error);
        return std::move(ser).object();
    }

    inline auto serial_adapter::get_callback_install(const rapidjson::Document& serial_obj)
        -> callback_install_request
    {
        RPC_HPP_PRECONDITION(verify_type(serial_obj, rpc_type::callback_install_request));

        callback_install_request cbk_req;
        deserializer ser{ serial_obj };
        ser.deserialize_object(cbk_req);
        return cbk_req;
    }

    inline auto serial_adapter::serialize_callback_install(
        const callback_install_request& callback_req) -> rapidjson::Document
    {
        serializer ser{};
        ser.serialize_object(callback_req);
        return std::move(ser).object();
    }

    inline auto serial_adapter::has_bound_args(const rapidjson::Document& serial_obj) -> bool
    {
        RPC_HPP_PRECONDITION(!is_empty(serial_obj));
        return serial_obj["bind_args"].GetBool();
    }

    inline auto serial_adapter::verify_type(const serial_t& serial_obj, const rpc_type type) -> bool
    {
        return get_type(serial_obj) == type;
    }
} //namespace detail_rapidjson

using rapidjson_adapter = detail_rapidjson::serial_adapter;
} //namespace rpc_hpp::adapters
#endif
