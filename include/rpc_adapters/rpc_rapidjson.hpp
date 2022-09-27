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

namespace rpc_hpp::adapters
{
class rapidjson_adapter;

template<>
struct serial_traits<rapidjson_adapter>
{
    using serial_t = rapidjson::Document;
    using bytes_t = std::string;
};

class rapidjson_serializer : public serializer<rapidjson_serializer, false>
{
public:
    rapidjson_serializer() = default;
    [[nodiscard]] const rapidjson::Document& object() const& { return m_json; }
    [[nodiscard]] rapidjson::Document&& object() && { return std::move(m_json); }

    template<typename T>
    void as_bool(std::string_view key, T& val)
    {
        subobject(key).SetBool(val);
    }

    template<typename T>
    void as_float(std::string_view key, T& val)
    {
        subobject(key).Set<T>(val);
    }

    template<typename T>
    void as_int(std::string_view key, T& val)
    {
        if constexpr (sizeof(T) < sizeof(int))
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
    void as_string(std::string_view key, T& val)
    {
        subobject(key).SetString(std::data(val), allocator());
    }

    template<typename T>
    void as_array(std::string_view key, T& val)
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
                arr.PushBack(rapidjson::Value{}.SetString(std::data(subval),
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
    void as_map(std::string_view key, T& val)
    {
        auto& obj = subobject(key).SetObject();

        for (const auto& [k, v] : val)
        {
            const auto key_str = key_string(k);

            rapidjson_serializer ser{};
            ser.serialize_object(v);

            obj.AddMember(rapidjson::Value{}.SetString(key_str.c_str(), allocator()),
                std::move(ser).object(), allocator());
        }
    }

    template<typename T>
    void as_multimap(std::string_view key, T& val)
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

            if constexpr (detail::is_stringlike_v<detail::remove_cvref_t<decltype(v)>>)
            {
                obj[key_str.c_str()].PushBack(
                    rapidjson::Value{}.SetString(
                        std::data(v), static_cast<rapidjson::SizeType>(std::size(v)), allocator()),
                    allocator());
            }
            else
            {
                obj[key_str.c_str()].PushBack(v, allocator());
            }
        }
    }

private:
    [[nodiscard]] rapidjson::Value& subobject(std::string_view key)
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
    [[nodiscard]] std::string key_string(const K& key_val)
    {
        rapidjson_serializer ser{};
        ser.serialize_object(key_val);
        rapidjson::StringBuffer buffer{};
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        std::move(ser).object().Accept(writer);
        return buffer.GetString();
    }

    [[nodiscard]] rapidjson::MemoryPoolAllocator<>& allocator() { return m_json.GetAllocator(); }

    rapidjson::Document m_json{};
};

class rapidjson_deserializer : public serializer<rapidjson_deserializer, true>
{
public:
    explicit rapidjson_deserializer(const rapidjson::Value& obj) : m_json(obj) {}
    explicit rapidjson_deserializer(rapidjson::Value&& obj) : m_json(std::move(obj)) {}

    template<typename T>
    void as_bool(std::string_view key, T& val) const
    {
        val = subobject(key).GetBool();
    }

    template<typename T>
    void as_float(std::string_view key, T& val) const
    {
        val = subobject(key).Get<T>();
    }

    template<typename T>
    void as_int(std::string_view key, T& val) const
    {
        // Rapidjson does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
        if constexpr (sizeof(T) < sizeof(int))
        {
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
    void as_string(std::string_view key, T& val) const
    {
        val = subobject(key).GetString();
    }

    template<typename T>
    void as_array(std::string_view key, T& val) const
    {
        const auto& arr = subobject(key).GetArray();

        std::transform(arr.begin(), arr.end(), std::inserter(val, val.end()),
            yield_value<detail::remove_cvref_t<typename T::value_type>>);
    }

    template<typename T, size_t N>
    void as_array(std::string_view key, std::array<T, N>& val) const
    {
        const auto& arr = subobject(key).GetArray();

        if (arr.Size() != N)
        {
            throw std::out_of_range("JSON array out of bounds");
        }

        std::transform(arr.begin(), arr.end(), val.begin(), yield_value<detail::remove_cvref_t<T>>);
    }

    template<typename T, typename Alloc>
    void as_array(std::string_view key, std::forward_list<T, Alloc>& val) const
    {
        using rev_iter_t =
            std::reverse_iterator<rapidjson::GenericArray<true, rapidjson::Value>::ValueIterator>;

        const auto& arr = subobject(key).GetArray();

        const rev_iter_t arr_rbegin{ arr.end() };
        const rev_iter_t arr_rend{ arr.begin() };

        for (auto it = arr_rbegin; it != arr_rend; ++it)
        {
            val.push_front(yield_value<detail::remove_cvref_t<T>>(*it));
        }
    }

    template<typename T>
    void as_map(std::string_view key, T& val) const
    {
        const auto& obj = subobject(key).GetObject();
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
    void as_multimap(std::string_view key, T& val) const
    {
        const auto& obj = subobject(key).GetObject();
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

private:
    [[nodiscard]] const rapidjson::Value& subobject(std::string_view key) const
    {
        if (key.empty())
        {
            return m_json;
        }

        return m_json[std::string{ key }.c_str()];
    }

    template<typename T>
    static T yield_value(const rapidjson::Value& val)
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
            rapidjson_deserializer ser{ val };
            T tmp_val;
            ser.deserialize_object(tmp_val);
            return tmp_val;
        }
    }

    const rapidjson::Value& m_json;
};

class rapidjson_adapter : public serial_adapter_base<rapidjson_adapter>
{
public:
    [[nodiscard]] static rapidjson::Document from_bytes(std::string&& bytes)
    {
        rapidjson::Document doc{};
        doc.SetObject();
        doc.Parse(std::move(bytes).c_str());

        if (doc.HasParseError())
        {
            throw deserialization_error("rapidjson: parsing error occurred");
        }

        if (const auto fname_it = doc.FindMember("func_name");
            (fname_it == doc.MemberEnd()) || (!fname_it->value.IsString()))
        {
            throw deserialization_error("rapidjson: field \"func_name\" not found");
        }

        return doc;
    }

    [[nodiscard]] static std::string to_bytes(const rapidjson::Document& serial_obj)
    {
        rapidjson::StringBuffer buffer{};
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        serial_obj.Accept(writer);
        return buffer.GetString();
    }

    [[nodiscard]] static std::string to_bytes(rapidjson::Document&& serial_obj)
    {
        rapidjson::StringBuffer buffer{};
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        std::move(serial_obj).Accept(writer);
        return buffer.GetString();
    }

    [[nodiscard]] static std::string get_func_name(const rapidjson::Document& serial_obj)
    {
        return serial_obj["func_name"].GetString();
    }

    [[nodiscard]] static rpc_type get_type(const rapidjson::Document& serial_obj)
    {
        return static_cast<rpc_type>(serial_obj["type"].GetInt());
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static detail::rpc_result<IsCallback, R> get_result(
        const rapidjson::Document& serial_obj)
    {
        RPC_HPP_PRECONDITION(
            (IsCallback
                && static_cast<rpc_type>(serial_obj["type"].GetInt()) == rpc_type::callback_result)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj["type"].GetInt()) == rpc_type::func_result));

        if constexpr (std::is_void_v<R>)
        {
            return { serial_obj["func_name"].GetString() };
        }
        else
        {
            return { serial_obj["func_name"].GetString(), parse_arg<R>(serial_obj["result"]) };
        }
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static rapidjson::Document serialize_result(
        const detail::rpc_result<IsCallback, R>& result)
    {
        rapidjson::Document doc{};
        auto& alloc = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember(
            "func_name", rapidjson::Value{}.SetString(result.func_name.c_str(), alloc), alloc);

        if constexpr (!std::is_void_v<R>)
        {
            rapidjson::Value val{};
            push_arg(result.result, val, alloc);
            doc.AddMember("result", std::move(val), alloc);
        }

        if constexpr (IsCallback)
        {
            doc.AddMember("type", static_cast<int>(rpc_type::callback_result), alloc);
        }
        else
        {
            doc.AddMember("type", static_cast<int>(rpc_type::func_result), alloc);
        }

        return doc;
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static detail::rpc_result_w_bind<IsCallback, R, Args...> get_result_w_bind(
        const rapidjson::Document& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj["type"].GetInt())
                                     == rpc_type::callback_result_w_bind)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj["type"].GetInt())
                    == rpc_type::func_result_w_bind));

        const auto& args_val = serial_obj["args"];
        RPC_HPP_UNUSED unsigned arg_counter = 0;

        if constexpr (std::is_void_v<R>)
        {
            return { serial_obj["func_name"].GetString(),
                parse_args<Args>(args_val, arg_counter)... };
        }
        else
        {
            return { serial_obj["func_name"].GetString(),
                parse_arg<R>(serial_obj["result"], parse_args<Args>(args_val, arg_counter)...) };
        }
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static rapidjson::Document serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result)
    {
        rapidjson::Document doc{};
        auto& alloc = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember(
            "func_name", rapidjson::Value{}.SetString(result.func_name.c_str(), alloc), alloc);

        if constexpr (!std::is_void_v<R>)
        {
            rapidjson::Value val{};
            push_arg(result.result, val, alloc);
            doc.AddMember("result", std::move(val), alloc);
        }

        rapidjson::Value arg_arr{};
        arg_arr.SetArray();
        arg_arr.Reserve(static_cast<rapidjson::SizeType>(sizeof...(Args)), alloc);
        doc.AddMember("bind_args", true, alloc);

        detail::for_each_tuple(result.args,
            [&arg_arr, &alloc](auto&& elem)
            { push_args(std::forward<decltype(elem)>(elem), arg_arr, alloc); });

        doc.AddMember("args", std::move(arg_arr), alloc);

        if constexpr (IsCallback)
        {
            doc.AddMember("type", static_cast<int>(rpc_type::callback_result_w_bind), alloc);
        }
        else
        {
            doc.AddMember("type", static_cast<int>(rpc_type::func_result_w_bind), alloc);
        }

        return doc;
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static detail::rpc_request<IsCallback, Args...> get_request(
        const rapidjson::Document& serial_obj)
    {
        RPC_HPP_PRECONDITION(
            (IsCallback
                && (static_cast<rpc_type>(serial_obj["type"].GetInt()) == rpc_type::callback_request
                    || static_cast<rpc_type>(serial_obj["type"].GetInt())
                        == rpc_type::callback_result_w_bind))
            || (!IsCallback
                && (static_cast<rpc_type>(serial_obj["type"].GetInt()) == rpc_type::func_request
                    || static_cast<rpc_type>(serial_obj["type"].GetInt())
                        == rpc_type::func_result_w_bind)));

        const auto& args_val = serial_obj["args"];
        const bool is_bound_args = serial_obj["bind_args"].GetBool();

        if (args_val.GetArray().Size() != sizeof...(Args))
        {
            throw function_mismatch("Argument count mismatch");
        }

        [[maybe_unused]] unsigned arg_counter = 0;
        typename detail::rpc_request<IsCallback, Args...>::args_t args = { parse_args<Args>(
            args_val, arg_counter)... };

        return is_bound_args
            ? detail::rpc_request<IsCallback, Args...>{ detail::bind_args_tag{},
                  serial_obj["func_name"].GetString(), std::move(args) }
            : detail::rpc_request<IsCallback, Args...>{ serial_obj["func_name"].GetString(),
                  std::move(args) };
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static rapidjson::Document serialize_request(
        const detail::rpc_request<IsCallback, Args...>& request)
    {
        rapidjson::Document doc{};
        auto& alloc = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember(
            "func_name", rapidjson::Value{}.SetString(request.func_name.c_str(), alloc), alloc);

        rapidjson::Value arg_arr{};
        arg_arr.SetArray();
        arg_arr.Reserve(static_cast<rapidjson::SizeType>(sizeof...(Args)), alloc);
        doc.AddMember("bind_args", request.bind_args, alloc);

        detail::for_each_tuple(request.args,
            [&arg_arr, &alloc](auto&& elem)
            { push_args(std::forward<decltype(elem)>(elem), arg_arr, alloc); });

        doc.AddMember("args", std::move(arg_arr), alloc);

        if constexpr (IsCallback)
        {
            doc.AddMember("type", static_cast<int>(rpc_type::callback_request), alloc);
        }
        else
        {
            doc.AddMember("type", static_cast<int>(rpc_type::func_request), alloc);
        }

        return doc;
    }

    template<bool IsCallback>
    [[nodiscard]] static detail::rpc_error<IsCallback> get_error(
        const rapidjson::Document& serial_obj)
    {
        RPC_HPP_PRECONDITION(
            (IsCallback
                && static_cast<rpc_type>(serial_obj["type"].GetInt()) == rpc_type::callback_error)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj["type"].GetInt()) == rpc_type::func_error));

        return { serial_obj["func_name"].GetString(),
            static_cast<exception_type>(serial_obj["except_type"].GetInt()),
            serial_obj["err_mesg"].GetString() };
    }

    template<bool IsCallback>
    [[nodiscard]] static rapidjson::Document serialize_error(
        const detail::rpc_error<IsCallback>& error)
    {
        rapidjson::Document doc{};
        auto& alloc = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember(
            "func_name", rapidjson::Value{}.SetString(error.func_name.c_str(), alloc), alloc);
        doc.AddMember(
            "err_mesg", rapidjson::Value{}.SetString(error.err_mesg.c_str(), alloc), alloc);
        doc.AddMember("except_type", static_cast<int>(error.except_type), alloc);

        if constexpr (IsCallback)
        {
            doc.AddMember("type", static_cast<int>(rpc_type::callback_error), alloc);
        }
        else
        {
            doc.AddMember("type", static_cast<int>(rpc_type::func_error), alloc);
        }

        return doc;
    }

    [[nodiscard]] static callback_install_request get_callback_install(
        const rapidjson::Document& serial_obj)
    {
        RPC_HPP_PRECONDITION(static_cast<rpc_type>(serial_obj["type"].GetInt())
            == rpc_type::callback_install_request);

        callback_install_request callback_req{ serial_obj["func_name"].GetString() };
        callback_req.is_uninstall = serial_obj["is_uninstall"].GetBool();
        return callback_req;
    }

    [[nodiscard]] static rapidjson::Document serialize_callback_install(
        const callback_install_request& callback_req)
    {
        rapidjson::Document doc{};
        auto& alloc = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("func_name",
            rapidjson::Value{}.SetString(callback_req.func_name.c_str(), alloc), alloc);
        doc.AddMember("is_uninstall", callback_req.is_uninstall, alloc);
        doc.AddMember("type", static_cast<int>(rpc_type::callback_install_request), alloc);
        return doc;
    }

    [[nodiscard]] static bool has_bound_args(const rapidjson::Document& serial_obj)
    {
        return serial_obj["bind_args"].GetBool();
    }

private:
    template<typename T>
    RPC_HPP_NODISCARD("this function is pointless without checking the bool")
    static constexpr bool validate_arg(const rapidjson::Value& arg) noexcept
    {
        if constexpr (std::is_same_v<T, bool>)
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

    // nodiscard because expect_type is consumed by the function
    [[nodiscard]] static std::string mismatch_message(
        std::string&& expect_type, const rapidjson::Value& obj)
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

        return { "rapidjson expected type: " + std::move(expect_type)
            + ", got type: " + get_type_str() };
    }

    template<typename T>
    static void push_arg(T&& arg, rapidjson::Value& obj, rapidjson::MemoryPoolAllocator<>& alloc)
    {
        rapidjson_serializer ser;
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

    template<typename T>
    RPC_HPP_NODISCARD(
        "parsing can be expensive, and it makes no sense to not use the parsed result")
    static detail::remove_cvref_t<detail::decay_str_t<T>> parse_arg(const rapidjson::Value& arg)
    {
        using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

        if (!validate_arg<no_ref_t>(arg))
        {
            throw function_mismatch{ mismatch_message(typeid(no_ref_t).name(), arg) };
        }

        rapidjson_deserializer ser{ arg };
        no_ref_t out_val;
        ser.deserialize_object(out_val);
        return out_val;
    }

    template<typename T>
    RPC_HPP_NODISCARD(
        "parsing can be expensive, and it makes no sense to not use the parsed result")
    static detail::remove_cvref_t<detail::decay_str_t<T>> parse_args(
        const rapidjson::Value& arg_arr, unsigned& index)
    {
        if (!arg_arr.IsArray())
        {
            return parse_arg<T>(arg_arr);
        }

        const auto& arr = arg_arr.GetArray();

        if (index >= arr.Size())
        {
            throw function_mismatch("Argument count mismatch");
        }

        const auto old_idx = index;
        ++index;
        return parse_arg<T>(arr[old_idx]);
    }
};
} //namespace rpc_hpp::adapters
#endif
