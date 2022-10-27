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

#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#ifndef RPC_HPP_NO_RTTI
#  include <typeinfo>
#endif

namespace rpc_hpp::adapters
{
namespace detail_njson
{
    class serializer;
    class deserializer;

    struct adapter_impl
    {
        using bytes_t = std::string;
        using serial_t = nlohmann::json;
        using serializer_t = serializer;
        using deserializer_t = deserializer;
        using config = void;
    };

    class serial_adapter : public serial_adapter_base<adapter_impl>
    {
    public:
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
    };

    class serializer : public serializer_base<serial_adapter, false>
    {
    public:
        serializer() noexcept = default;

        [[nodiscard]] auto object() const& noexcept -> const nlohmann::json& { return m_json; }
        [[nodiscard]] auto object() && noexcept -> nlohmann::json&& { return std::move(m_json); }

        template<typename T>
        void as_bool(const std::string_view key, const T& val)
        {
            subobject(key) = static_cast<bool>(val);
        }

        template<typename T>
        void as_float(const std::string_view key, const T& val)
        {
            subobject(key) = val;
        }

        template<typename T>
        void as_int(const std::string_view key, const T& val)
        {
            subobject(key) = val;
        }

        template<typename T>
        void as_string(const std::string_view key, const T& val)
        {
            subobject(key) = val;
        }

        template<typename T>
        void as_array(const std::string_view key, const T& val)
        {
            auto arr = nlohmann::json::array();

            for (const auto& subval : val)
            {
                arr.push_back(subval);
            }

            subobject(key) = std::move(arr);
        }

        template<typename T>
        void as_map(const std::string_view key, const T& val)
        {
            auto obj = nlohmann::json::object();

            for (const auto& [k, v] : val)
            {
                const auto key_str = nlohmann::json{ k }.dump();
                obj[key_str] = v;
            }

            subobject(key) = std::move(obj);
        }

        template<typename T>
        void as_multimap(const std::string_view key, const T& val)
        {
            auto obj = nlohmann::json::object();

            for (const auto& [k, v] : val)
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

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, const std::pair<T1, T2>& val)
        {
            auto obj = nlohmann::json::object();
            obj["first"] = val.first;
            obj["second"] = val.second;
            subobject(key) = std::move(obj);
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, const std::tuple<Args...>& val)
        {
            // Need to create the subobject in case args is empty
            auto& arg_arr = subobject(key);

            detail::for_each_tuple(val,
                [&arg_arr](auto&& elem)
                { push_args(std::forward<decltype(elem)>(elem), arg_arr); });
        }

        template<typename T>
        void as_optional(const std::string_view key, const std::optional<T>& val)
        {
            if (val.has_value())
            {
                subobject(key) = val.value();
            }
            else
            {
                subobject(key) = nullptr;
            }
        }

        template<typename T>
        void as_object(const std::string_view key, const T& val)
        {
            push_arg(val, subobject(key));
        }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) -> nlohmann::json&
        {
            return key.empty() ? m_json : m_json[key];
        }

        template<typename T>
        static void push_arg(T&& arg, nlohmann::json& obj)
        {
            serializer ser{};
            ser.serialize_object(std::forward<T>(arg));
            obj = std::move(ser).object();
        }

        template<typename T>
        static void push_args(T&& arg, nlohmann::json& obj_arr)
        {
            nlohmann::json tmp{};
            push_arg(std::forward<T>(arg), tmp);
            obj_arr.push_back(std::move(tmp));
        }

        nlohmann::json m_json{};
    };

    class deserializer : public serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const nlohmann::json& obj) noexcept : m_json(obj) {}

        template<typename T>
        void as_bool(const std::string_view key, T& val) const
        {
            val = subobject(key).get<bool>();
        }

        template<typename T>
        void as_float(const std::string_view key, T& val) const
        {
            val = subobject(key).get<T>();
        }

        template<typename T>
        void as_int(const std::string_view key, T& val) const
        {
            val = subobject(key).get<T>();
        }

        template<typename T>
        void as_string(const std::string_view key, T& val) const
        {
            val = subobject(key).get<std::string>();
        }

        template<typename T>
        void as_array(const std::string_view key, T& val) const
        {
            const auto& arr = subobject(key);
            val = T{ cbegin(arr), cend(arr) };
        }

        template<typename T, size_t N>
        void as_array(const std::string_view key, std::array<T, N>& val) const
        {
            const auto& arr = subobject(key);

            if (arr.size() != N)
            {
                throw std::out_of_range{ "nlohmann::json error: array out of bounds" };
            }

            std::copy(cbegin(arr), cend(arr), begin(val));
        }

        template<typename T>
        void as_map(const std::string_view key, T& val) const
        {
            const auto& obj = subobject(key);

            for (const auto& [k, v] : obj.items())
            {
                val.insert(
                    { nlohmann::json::parse(k).front().template get<typename T::key_type>(), v });
            }
        }

        template<typename T>
        void as_multimap(const std::string_view key, T& val) const
        {
            const auto& obj = subobject(key);

            for (const auto& [k, v] : obj.items())
            {
                for (const auto& subval : v)
                {
                    val.insert(
                        { nlohmann::json::parse(k).front().template get<typename T::key_type>(),
                            subval });
                }
            }
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, std::pair<T1, T2>& val) const
        {
            const auto& obj = subobject(key);
            val = { obj["first"], obj["second"] };
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, std::tuple<Args...>& val) const
        {
            if (subobject(key).size() != sizeof...(Args))
            {
                throw function_mismatch_error{ "nlohmann::json error: invalid number of args" };
            }

            [[maybe_unused]] size_t arg_counter = 0;
            val = { parse_args<Args>(subobject(key), arg_counter)... };
        }

        template<typename T>
        void as_optional(const std::string_view key, std::optional<T>& val) const
        {
            const auto& obj = subobject(key);
            val = obj.is_null() ? std::optional<T>{ std::nullopt }
                                : std::optional<T>{ std::in_place, obj.template get<T>() };
        }

        template<typename T>
        void as_object(const std::string_view key, T& val) const
        {
            val = parse_arg<T>(subobject(key));
        }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) const -> const nlohmann::json&
        {
            return key.empty() ? m_json : m_json[key];
        }

        template<typename T>
        RPC_HPP_NODISCARD("function is pointless without checking the bool")
        static constexpr auto validate_arg(const nlohmann::json& arg) noexcept -> bool
        {
            if constexpr (detail::is_optional_v<T>)
            {
                return arg.is_null() || validate_arg<typename T::value_type>(arg);
            }
            else if constexpr (std::is_same_v<T, bool>)
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

        template<typename T>
        RPC_HPP_NODISCARD(
            "parsing can be expensive and it makes no sense to not use the parsed result")
        static auto parse_arg(const nlohmann::json& arg)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

            if (!validate_arg<T>(arg))
            {
#ifdef RPC_HPP_NO_RTTI
                throw function_mismatch_error{ std::string{
                    "nlohmann::json error: expected type: {NO-RTTI}, got type: " }
                                                   .append(arg.type_name()) };
#else
                throw function_mismatch_error{ std::string{
                    "nlohmann::json error: expected type: " }
                                                   .append(typeid(T).name())
                                                   .append(", got type: ")
                                                   .append(arg.type_name()) };
#endif
            }

            no_ref_t out_val;
            deserializer ser{ arg };
            ser.deserialize_object(out_val);
            return out_val;
        }

        template<typename T>
        RPC_HPP_NODISCARD(
            "parsing can be expensive and it makes no sense to not use the parsed result")
        static auto parse_args(const nlohmann::json& arg_arr, size_t& index)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            if (index >= arg_arr.size())
            {
                throw function_mismatch_error{ "nlohmann::json error: argument count mismatch" };
            }

            if (arg_arr.is_array())
            {
                const auto old_idx = index;
                ++index;
                return parse_arg<T>(arg_arr[old_idx]);
            }

            return parse_arg<T>(arg_arr);
        }

        const nlohmann::json& m_json;
    };

    [[nodiscard]] inline auto serial_adapter::from_bytes(std::string&& bytes) -> nlohmann::json
    {
        nlohmann::json obj = nlohmann::json::parse(std::move(bytes));

        if (!obj.is_object())
        {
            throw deserialization_error{ "nlohmann::json error: not an object" };
        }

        if (const auto fname_it = obj.find("func_name");
            (fname_it == obj.end()) || (!fname_it->is_string()) || (fname_it->empty()))
        {
            throw deserialization_error{ R"(nlohmann::json error: field "func_name" not found)" };
        }

        return obj;
    }

    [[nodiscard]] inline auto serial_adapter::to_bytes(const nlohmann::json& serial_obj)
        -> std::string
    {
        return serial_obj.dump();
    }

    [[nodiscard]] inline auto serial_adapter::to_bytes(nlohmann::json&& serial_obj) -> std::string
    {
        return std::move(serial_obj).dump();
    }

    [[nodiscard]] inline auto serial_adapter::get_func_name(const nlohmann::json& serial_obj)
        -> std::string
    {
        return serial_obj["func_name"];
    }

    [[nodiscard]] inline rpc_type serial_adapter::get_type(const nlohmann::json& serial_obj)
    {
        return static_cast<rpc_type>(serial_obj["type"].get<int>());
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] auto serial_adapter::get_result(const nlohmann::json& serial_obj)
        -> detail::rpc_result<IsCallback, R>
    {
        RPC_HPP_PRECONDITION((IsCallback && serial_obj["type"] == rpc_type::callback_result)
            || (!IsCallback && serial_obj["type"] == rpc_type::func_result));

        detail::rpc_result<IsCallback, R> result;
        deserializer ser{ serial_obj };
        ser.deserialize_object(result);
        return result;
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] auto serial_adapter::serialize_result(
        const detail::rpc_result<IsCallback, R>& result) -> nlohmann::json
    {
        serializer ser{};
        ser.serialize_object(result);
        return std::move(ser).object();
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] auto serial_adapter::get_result_w_bind(const nlohmann::json& serial_obj)
        -> detail::rpc_result_w_bind<IsCallback, R, Args...>
    {
        RPC_HPP_PRECONDITION((IsCallback && serial_obj["type"] == rpc_type::callback_result_w_bind)
            || (!IsCallback && serial_obj["type"] == rpc_type::func_result_w_bind));

        detail::rpc_result_w_bind<IsCallback, R, Args...> result;
        deserializer ser{ serial_obj };
        ser.deserialize_object(result);
        return result;
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] auto serial_adapter::serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) -> nlohmann::json
    {
        serializer ser{};
        ser.serialize_object(result);
        return std::move(ser).object();
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] auto serial_adapter::get_request(const nlohmann::json& serial_obj)
        -> detail::rpc_request<IsCallback, Args...>
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && (serial_obj["type"] == rpc_type::callback_request
                                     || serial_obj["type"] == rpc_type::callback_result_w_bind))
            || (!IsCallback
                && (serial_obj["type"] == rpc_type::func_request
                    || serial_obj["type"] == rpc_type::func_result_w_bind)));

        detail::rpc_request<IsCallback, Args...> request;
        deserializer ser{ serial_obj };
        ser.deserialize_object(request);
        return request;
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] auto serial_adapter::serialize_request(
        const detail::rpc_request<IsCallback, Args...>& request) -> nlohmann::json
    {
        serializer ser{};
        ser.serialize_object(request);
        return std::move(ser).object();
    }

    template<bool IsCallback>
    [[nodiscard]] auto serial_adapter::get_error(const nlohmann::json& serial_obj)
        -> detail::rpc_error<IsCallback>
    {
        RPC_HPP_PRECONDITION((IsCallback && serial_obj["type"] == rpc_type::callback_error)
            || (!IsCallback && serial_obj["type"] == rpc_type::func_error));

        detail::rpc_error<IsCallback> error;
        deserializer ser{ serial_obj };
        ser.deserialize_object(error);
        return error;
    }

    template<bool IsCallback>
    [[nodiscard]] auto serial_adapter::serialize_error(const detail::rpc_error<IsCallback>& error)
        -> nlohmann::json
    {
        serializer ser{};
        ser.serialize_object(error);
        return std::move(ser).object();
    }

    [[nodiscard]] inline auto serial_adapter::get_callback_install(const nlohmann::json& serial_obj)
        -> callback_install_request
    {
        RPC_HPP_PRECONDITION(serial_obj["type"] == rpc_type::callback_install_request);

        callback_install_request cbk_req;
        deserializer ser{ serial_obj };
        ser.deserialize_object(cbk_req);
        return cbk_req;
    }

    [[nodiscard]] inline auto serial_adapter::serialize_callback_install(
        const callback_install_request& callback_req) -> nlohmann::json
    {
        serializer ser{};
        ser.serialize_object(callback_req);
        return std::move(ser).object();
    }

    [[nodiscard]] inline auto serial_adapter::has_bound_args(const nlohmann::json& serial_obj)
        -> bool
    {
        return serial_obj["bind_args"];
    }
} //namespace detail_njson

using njson_adapter = detail_njson::serial_adapter;
} //namespace rpc_hpp::adapters
#endif
