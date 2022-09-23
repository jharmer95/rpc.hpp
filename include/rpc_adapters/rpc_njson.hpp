///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann's JSON library (https://github.com/nlohmann/json)
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

#ifndef RPC_ADAPTERS_NJSON_HPP
#define RPC_ADAPTERS_NJSON_HPP

#include "../rpc.hpp"

#include <nlohmann/json.hpp>

#ifndef RPC_HPP_NO_RTTI
#  include <typeinfo>
#endif

namespace rpc_hpp::adapters
{
class njson_adapter;

template<>
struct serial_traits<njson_adapter>
{
    using serial_t = nlohmann::json;
    using bytes_t = std::string;
};

class njson_serializer : public serializer<njson_serializer, false>
{
public:
    const nlohmann::json& object() const& { return m_json; }
    nlohmann::json&& object() && { return std::move(m_json); }

    template<typename T>
    void as_bool(std::string_view key, T& t)
    {
        subobject(key) = static_cast<bool>(t);
    }

    template<typename T>
    void as_float(std::string_view key, T& t)
    {
        subobject(key) = t;
    }

    template<typename T>
    void as_int(std::string_view key, T& t)
    {
        subobject(key) = t;
    }

    template<typename T>
    void as_string(std::string_view key, T& t)
    {
        subobject(key) = t;
    }

    template<typename T>
    void as_array(std::string_view key, T& t)
    {
        auto arr = nlohmann::json::array();

        for (const auto& val : t)
        {
            arr.push_back(val);
        }

        subobject(key) = std::move(arr);
    }

    template<typename T>
    void as_map(std::string_view key, T& t)
    {
        auto obj = nlohmann::json::object();

        for (const auto& [k, v] : t)
        {
            obj[nlohmann::json{ k }.dump()] = v;
        }

        subobject(key) = std::move(obj);
    }

    template<typename T>
    void as_multimap(std::string_view key, T& t)
    {
        auto obj = nlohmann::json::object();

        for (const auto& [k, v] : t)
        {
            const auto key_str = nlohmann::json{ k }.dump();

            if (obj.find(key_str) == end(obj))
            {
                obj[key_str] = nlohmann::json::array();
            }

            obj[key_str].push_back(v);
        }

        subobject(key) = std::move(obj);
    }

private:
    nlohmann::json& subobject(std::string_view key) { return key.empty() ? m_json : m_json[key]; }

    nlohmann::json m_json{};
};

class njson_deserializer : public serializer<njson_deserializer, true>
{
public:
    explicit njson_deserializer(const nlohmann::json& obj) : m_json(obj) {}
    explicit njson_deserializer(nlohmann::json&& obj) : m_json(std::move(obj)) {}

    template<typename T>
    void as_bool(std::string_view key, T& t) const
    {
        t = subobject(key).get<bool>();
    }

    template<typename T>
    void as_float(std::string_view key, T& t) const
    {
        t = subobject(key).get<T>();
    }

    template<typename T>
    void as_int(std::string_view key, T& t) const
    {
        t = subobject(key).get<T>();
    }

    template<typename T>
    void as_string(std::string_view key, T& t) const
    {
        t = subobject(key).get<std::string>();
    }

    template<typename T>
    void as_array(std::string_view key, T& t) const
    {
        const auto& arr = subobject(key);
        t = T{ cbegin(arr), cend(arr) };
    }

    template<typename T, size_t N>
    void as_array(std::string_view key, std::array<T, N>& t) const
    {
        const auto& arr = subobject(key);

        if (arr.size() != N)
        {
            throw std::out_of_range("JSON array out of bounds");
        }

        std::copy(cbegin(arr), cend(arr), begin(t));
    }

    template<typename T>
    void as_map(std::string_view key, T& t) const
    {
        const auto& obj = subobject(key);

        for (const auto& [k, v] : obj.items())
        {
            t.insert({ nlohmann::json::parse(k).front().template get<typename T::key_type>(), v });
        }
    }

    template<typename T>
    void as_multimap(std::string_view key, T& t) const
    {
        const auto& obj = subobject(key);

        for (const auto& [k, v] : obj.items())
        {
            for (const auto& subval : v)
            {
                t.insert({ nlohmann::json::parse(k).front().template get<typename T::key_type>(),
                    subval });
            }
        }
    }

private:
    const nlohmann::json& subobject(std::string_view key) const
    {
        return key.empty() ? m_json : m_json[key];
    }

    nlohmann::json m_json{};
};

class njson_adapter : public serial_adapter_base<njson_adapter>
{
public:
    [[nodiscard]] static nlohmann::json from_bytes(std::string&& bytes)
    {
        nlohmann::json obj = nlohmann::json::parse(std::move(bytes));

        if (!obj.is_object())
        {
            throw deserialization_error("NJSON: not an object");
        }

        if (const auto fname_it = obj.find("func_name");
            (fname_it == obj.end()) || (!fname_it->is_string()) || (fname_it->empty()))
        {
            throw deserialization_error("NJSON: field \"func_name\" not found");
        }

        return obj;
    }

    [[nodiscard]] static std::string to_bytes(const nlohmann::json& serial_obj)
    {
        return serial_obj.dump();
    }

    [[nodiscard]] static std::string to_bytes(nlohmann::json&& serial_obj)
    {
        return std::move(serial_obj).dump();
    }

    [[nodiscard]] static std::string get_func_name(const nlohmann::json& serial_obj)
    {
        return serial_obj["func_name"];
    }

    [[nodiscard]] static rpc_type get_type(const nlohmann::json& serial_obj)
    {
        return static_cast<rpc_type>(serial_obj["type"].get<int>());
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static detail::rpc_result<IsCallback, R> get_result(
        const nlohmann::json& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback && serial_obj["type"] == rpc_type::callback_result)
            || (!IsCallback && serial_obj["type"] == rpc_type::func_result));

        if constexpr (std::is_void_v<R>)
        {
            return { serial_obj["func_name"].get<std::string>() };
        }
        else
        {
            return { serial_obj["func_name"].get<std::string>(),
                parse_arg<R>(serial_obj["result"]) };
        }
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static nlohmann::json serialize_result(
        const detail::rpc_result<IsCallback, R>& result)
    {
        nlohmann::json obj{};
        obj["func_name"] = result.func_name;

        if constexpr (!std::is_void_v<R>)
        {
            obj["result"] = {};
            push_arg(result.result, obj["result"]);
        }

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int>(rpc_type::callback_result);
        }
        else
        {
            obj["type"] = static_cast<int>(rpc_type::func_result);
        }

        return obj;
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static detail::rpc_result_w_bind<IsCallback, R, Args...> get_result_w_bind(
        const nlohmann::json& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback && serial_obj["type"] == rpc_type::callback_result_w_bind)
            || (!IsCallback && serial_obj["type"] == rpc_type::func_result_w_bind));

        const auto& args_val = serial_obj["args"];
        RPC_HPP_UNUSED unsigned arg_counter = 0;

        if constexpr (std::is_void_v<R>)
        {
            return { serial_obj["func_name"].get<std::string>(),
                parse_args<Args>(args_val, arg_counter)... };
        }
        else
        {
            return { serial_obj["func_name"].get<std::string>(),
                parse_arg<R>(serial_obj["result"], parse_args<Args>(args_val, arg_counter)...) };
        }
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static nlohmann::json serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result)
    {
        nlohmann::json obj{};

        obj["func_name"] = result.func_name;
        obj["args"] = nlohmann::json::array();

        if constexpr (!std::is_void_v<R>)
        {
            obj["result"] = {};
            push_arg(result.result, obj["result"]);
        }

        auto& arg_arr = obj["args"];
        obj["bind_args"] = true;
        arg_arr.get_ref<nlohmann::json::array_t&>().reserve(sizeof...(Args));

        detail::for_each_tuple(result.args,
            [&arg_arr](auto&& elem) { push_args(std::forward<decltype(elem)>(elem), arg_arr); });

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int>(rpc_type::callback_result_w_bind);
        }
        else
        {
            obj["type"] = static_cast<int>(rpc_type::func_result_w_bind);
        }

        return obj;
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static detail::rpc_request<IsCallback, Args...> get_request(
        const nlohmann::json& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && (serial_obj["type"] == rpc_type::callback_request
                                     || serial_obj["type"] == rpc_type::callback_result_w_bind))
            || (!IsCallback
                && (serial_obj["type"] == rpc_type::func_request
                    || serial_obj["type"] == rpc_type::func_result_w_bind)));

        const auto& args_val = serial_obj["args"];
        const bool is_bound_args = serial_obj["bind_args"];

        if (args_val.size() != sizeof...(Args))
        {
            throw function_mismatch("Argument count mismatch");
        }

        RPC_HPP_UNUSED unsigned arg_counter = 0;
        typename detail::rpc_request<IsCallback, Args...>::args_t args = { parse_args<Args>(
            args_val, arg_counter)... };

        return is_bound_args
            ? detail::rpc_request<IsCallback, Args...>{ detail::bind_args_tag{},
                  serial_obj["func_name"].get<std::string>(), std::move(args) }
            : detail::rpc_request<IsCallback, Args...>{ serial_obj["func_name"].get<std::string>(),
                  std::move(args) };
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static nlohmann::json serialize_request(
        const detail::rpc_request<IsCallback, Args...>& request)
    {
        nlohmann::json obj{};
        obj["func_name"] = request.func_name;
        obj["args"] = nlohmann::json::array();
        auto& arg_arr = obj["args"];
        obj["bind_args"] = request.bind_args;
        arg_arr.get_ref<nlohmann::json::array_t&>().reserve(sizeof...(Args));

        detail::for_each_tuple(request.args,
            [&arg_arr](auto&& elem) { push_args(std::forward<decltype(elem)>(elem), arg_arr); });

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int>(rpc_type::callback_request);
        }
        else
        {
            obj["type"] = static_cast<int>(rpc_type::func_request);
        }

        return obj;
    }

    template<bool IsCallback>
    [[nodiscard]] static detail::rpc_error<IsCallback> get_error(const nlohmann::json& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback && serial_obj["type"] == rpc_type::callback_error)
            || (!IsCallback && serial_obj["type"] == rpc_type::func_error));

        return { serial_obj["func_name"].get<std::string>(),
            static_cast<exception_type>(serial_obj["except_type"]),
            serial_obj["err_mesg"].get<std::string>() };
    }

    template<bool IsCallback>
    [[nodiscard]] static nlohmann::json serialize_error(const detail::rpc_error<IsCallback>& error)
    {
        nlohmann::json obj{};
        obj["func_name"] = error.func_name;
        obj["err_mesg"] = error.err_mesg;
        obj["except_type"] = static_cast<int>(error.except_type);

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int>(rpc_type::callback_error);
        }
        else
        {
            obj["type"] = static_cast<int>(rpc_type::func_error);
        }

        return obj;
    }

    [[nodiscard]] static callback_install_request get_callback_install(
        const nlohmann::json& serial_obj)
    {
        RPC_HPP_PRECONDITION(serial_obj["type"] == rpc_type::callback_install_request);

        callback_install_request callback_req{ serial_obj["func_name"].get<std::string>() };
        callback_req.is_uninstall = serial_obj["is_uninstall"];
        return callback_req;
    }

    [[nodiscard]] static nlohmann::json serialize_callback_install(
        const callback_install_request& callback_req)
    {
        nlohmann::json obj{};
        obj["func_name"] = callback_req.func_name;
        obj["is_uninstall"] = callback_req.is_uninstall;
        obj["type"] = static_cast<int>(rpc_type::callback_install_request);
        return obj;
    }

    [[nodiscard]] static bool has_bound_args(const nlohmann::json& serial_obj)
    {
        return serial_obj["bind_args"];
    }

private:
    template<typename T>
    RPC_HPP_NODISCARD("function is pointless without checking the bool")
    static constexpr bool validate_arg(const nlohmann::json& arg) noexcept
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            return arg.is_boolean();
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return arg.is_number() && (!arg.is_number_float());
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return arg.is_number_float();
        }
        else if constexpr (detail::is_stringlike_v<T>)
        {
            return arg.is_string();
        }
        else if constexpr (detail::is_map_v<T>)
        {
            return arg.is_object();
        }
        else if constexpr (detail::is_container_v<T>)
        {
            return arg.is_array();
        }
        else
        {
            return !arg.is_null();
        }
    }

    RPC_HPP_NODISCARD("expect_type is consumed by the function")
    static std::string mismatch_string(std::string&& expect_type, const nlohmann::json& arg)
    {
        return { "njson expected type: " + std::move(expect_type)
            + ", got type: " + arg.type_name() };
    }

    template<typename T>
    static void push_arg(T&& arg, nlohmann::json& obj)
    {
        njson_serializer s;
        s.serialize_object(std::forward<T>(arg));
        obj = std::move(s).object();
    }

    template<typename T>
    static void push_args(T&& arg, nlohmann::json& obj_arr)
    {
        nlohmann::json tmp{};
        push_arg(std::forward<T>(arg), tmp);
        obj_arr.push_back(std::move(tmp));
    }

    template<typename T>
    RPC_HPP_NODISCARD("parsing can be expensive and it makes no sense to not use the parsed result")
    static detail::remove_cvref_t<detail::decay_str_t<T>> parse_arg(const nlohmann::json& arg)
    {
        using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

        if (!validate_arg<T>(arg))
        {
#ifdef RPC_HPP_NO_RTTI
            throw function_mismatch{ mismatch_string("{NO-RTTI}", arg) };
#else
            throw function_mismatch{ mismatch_string(typeid(T).name(), arg) };
#endif
        }

        njson_deserializer s{ arg };
        no_ref_t x;
        s.deserialize_object(x);

        return x;
    }

    template<typename T>
    RPC_HPP_NODISCARD("parsing can be expensive and it makes no sense to not use the parsed result")
    static detail::remove_cvref_t<detail::decay_str_t<T>> parse_args(const nlohmann::json& arg_arr, unsigned& index)
    {
        if (index >= arg_arr.size())
        {
            throw function_mismatch("Argument count mismatch");
        }

        if (arg_arr.is_array())
        {
            const auto old_idx = index;
            ++index;
            return parse_arg<T>(arg_arr[old_idx]);
        }
        else
        {
            return parse_arg<T>(arg_arr);
        }
    }
};
} //namespace rpc_hpp::adapters
#endif
