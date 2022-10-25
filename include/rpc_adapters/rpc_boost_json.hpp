///@file rpc_adapters/rpc_boost_json.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting Boost.JSON (https://github.com/boostorg/json)
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

#ifndef RPC_ADAPTERS_BOOST_JSON_HPP
#define RPC_ADAPTERS_BOOST_JSON_HPP

#include "../rpc.hpp"

#include <boost/json.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace rpc_hpp::adapters
{
namespace detail_boost_json
{
    class serializer;
    class deserializer;

    struct adapter_impl
    {
        using bytes_t = std::string;
        using serial_t = boost::json::object;
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
        serializer() noexcept { m_json.emplace_object(); }

        [[nodiscard]] auto object() const& noexcept -> const boost::json::value& { return m_json; }
        [[nodiscard]] auto object() && noexcept -> boost::json::value&&
        {
            return std::move(m_json);
        }

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
            if constexpr (std::is_enum_v<T>)
            {
                subobject(key) = static_cast<int64_t>(val);
            }
            else
            {
                subobject(key) = val;
            }
        }

        template<typename T>
        void as_string(const std::string_view key, const T& val)
        {
            subobject(key) = val;
        }

        template<typename T>
        void as_array(const std::string_view key, const T& val)
        {
            auto arr = boost::json::array{};

            if constexpr (detail::has_size<T>::value)
            {
                arr.reserve(val.size());
            }

            for (const auto& subval : val)
            {
                if constexpr (detail::is_stringlike_v<detail::remove_cvref_t<decltype(subval)>>)
                {
                    boost::json::value v_subval{};
                    auto& subval_str = v_subval.emplace_string();
                    subval_str = subval;
                    arr.push_back(std::move(v_subval));
                }
                else
                {
                    arr.push_back(subval);
                }
            }

            subobject(key) = std::move(arr);
        }

        template<typename T>
        void as_map(const std::string_view key, const T& val)
        {
            auto obj = boost::json::object{};

            for (const auto& [k, v] : val)
            {
                const auto key_str = boost::json::serialize(boost::json::value{ k });
                obj[key_str] = v;
            }

            subobject(key) = std::move(obj);
        }

        template<typename T>
        void as_multimap(const std::string_view key, const T& val)
        {
            auto obj = boost::json::object{};

            for (const auto& [k, v] : val)
            {
                const auto key_str = boost::json::serialize(boost::json::value{ k });

                if (obj.find(key_str) == obj.end())
                {
                    obj[key_str] = boost::json::array{};
                }

                if constexpr (detail::is_stringlike_v<detail::remove_cvref_t<decltype(v)>>)
                {
                    obj[key_str].get_array().push_back(boost::json::string{ v });
                }
                else
                {
                    obj[key_str].get_array().push_back(v);
                }
            }

            subobject(key) = std::move(obj);
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, const std::pair<T1, T2>& val)
        {
            auto obj = boost::json::object{};
            obj["first"] = val.first;
            obj["second"] = val.second;
            subobject(key) = std::move(obj);
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, const std::tuple<Args...>& val)
        {
            auto arg_arr = boost::json::array{};
            arg_arr.reserve(sizeof...(Args));
            detail::for_each_tuple(val,
                [&arg_arr](auto&& elem)
                { push_args(std::forward<decltype(elem)>(elem), arg_arr); });

            subobject(key) = std::move(arg_arr);
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
                subobject(key).emplace_null();
            }
        }

        template<typename T>
        void as_object(const std::string_view key, const T& val)
        {
            push_arg(val, subobject(key));
        }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) -> boost::json::value&
        {
            return key.empty() ? m_json : m_json.get_object()[key];
        }

        template<typename T>
        static void push_arg(T&& arg, boost::json::value& obj)
        {
            serializer ser{};
            ser.serialize_object(std::forward<T>(arg));
            obj = std::move(ser).object();
        }

        template<typename T>
        static void push_args(T&& arg, boost::json::array& obj_arr)
        {
            boost::json::value tmp{};
            push_arg(std::forward<T>(arg), tmp);
            obj_arr.push_back(tmp);
        }

        boost::json::value m_json{};
    };

    class deserializer : public serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const boost::json::value& obj) : m_json(obj) {}
        explicit deserializer(boost::json::value&& obj) noexcept : m_json(std::move(obj)) {}

        template<typename T>
        void as_bool(const std::string_view key, T& val) const
        {
            val = boost::json::value_to<bool>(subobject(key));
        }

        template<typename T>
        void as_float(const std::string_view key, T& val) const
        {
            val = boost::json::value_to<T>(subobject(key));
        }

        template<typename T>
        void as_int(const std::string_view key, T& val) const
        {
            if constexpr (std::is_enum_v<T>)
            {
                val = static_cast<T>(subobject(key).get_int64());
            }
            else
            {
                val = boost::json::value_to<T>(subobject(key));
            }
        }

        template<typename T>
        void as_string(const std::string_view key, T& val) const
        {
            val = subobject(key).get_string().c_str();
        }

        template<typename T>
        void as_array(const std::string_view key, T& val) const
        {
            const auto& arr = subobject(key).as_array();
            std::transform(arr.cbegin(), arr.cend(), std::inserter(val, val.end()),
                yield_value<detail::remove_cvref_t<typename T::value_type>>);
        }

        template<typename T, size_t N>
        void as_array(const std::string_view key, std::array<T, N>& val) const
        {
            const auto& arr = subobject(key).as_array();

            if (arr.size() != N)
            {
                throw std::out_of_range{ "JSON array out of bounds" };
            }

            std::transform(
                arr.cbegin(), arr.cend(), val.begin(), yield_value<detail::remove_cvref_t<T>>);
        }

        template<typename T, typename Alloc>
        void as_array(const std::string_view key, std::forward_list<T, Alloc>& val) const
        {
            const auto& arr = subobject(key).as_array();
            const auto arr_rend = arr.crend();

            for (auto it = arr.crbegin(); it != arr_rend; ++it)
            {
                val.push_front(yield_value<detail::remove_cvref_t<T>>(*it));
            }
        }

        template<typename T>
        void as_map(const std::string_view key, T& val) const
        {
            const auto& obj = subobject(key).as_object();

            for (const auto& [k, v] : obj)
            {
                const boost::json::value key_val = boost::json::parse(k).as_array().front();
                val.insert({ boost::json::value_to<typename T::key_type>(key_val),
                    boost::json::value_to<typename T::mapped_type>(v) });
            }
        }

        template<typename T>
        void as_multimap(const std::string_view key, T& val) const
        {
            const auto& obj = subobject(key).as_object();

            for (const auto& [k, v] : obj)
            {
                for (const auto& subval : v.as_array())
                {
                    const boost::json::value key_val = boost::json::parse(k).as_array().front();
                    val.insert({ boost::json::value_to<typename T::key_type>(key_val),
                        boost::json::value_to<typename T::mapped_type>(subval) });
                }
            }
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, std::pair<T1, T2>& val) const
        {
            const auto& obj = subobject(key).as_object();
            val = { boost::json::value_to<T1>(obj.at("first")),
                boost::json::value_to<T2>(obj.at("second")) };
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, std::tuple<Args...>& val) const
        {
            const auto& arg_arr = subobject(key);

            if (arg_arr.as_array().size() != sizeof...(Args))
            {
                throw function_mismatch{ "Boost.JSON: invalid number of args" };
            }

            [[maybe_unused]] size_t arg_counter = 0;
            val = { parse_args<Args>(arg_arr, arg_counter)... };
        }

        template<typename T>
        void as_optional(const std::string_view key, std::optional<T>& val) const
        {
            const auto& obj = subobject(key);
            val = obj.is_null() ? std::optional<T>{ std::nullopt }
                                : std::optional<T>{ std::in_place, boost::json::value_to<T>(obj) };
        }

        template<typename T>
        void as_object(const std::string_view key, T& val) const
        {
            val = parse_arg<T>(subobject(key));
        }

    private:
        [[nodiscard]] auto subobject(std::string_view key) const -> const boost::json::value&
        {
            return key.empty() ? m_json : m_json.at(key);
        }

        template<typename T>
        [[nodiscard]] static constexpr auto validate_arg(const boost::json::value& arg) noexcept
            -> bool
        {
            if constexpr (detail::is_optional_v<T>)
            {
                return arg.is_null() || validate_arg<typename T::value_type>(arg);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return arg.is_bool();
            }
            else if constexpr (std::is_integral_v<T>)
            {
                return arg.is_int64() || arg.is_uint64();
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                return arg.is_double();
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

        [[nodiscard]] static auto mismatch_string(
            std::string&& expect_type, const boost::json::value& obj) -> std::string
        {
            const auto get_type_str = [&obj]() noexcept
            {
                switch (obj.kind())
                {
                    case boost::json::kind::bool_:
                        return "bool";

                    case boost::json::kind::int64:
                        return "int64";

                    case boost::json::kind::uint64:
                        return "uint64";

                    case boost::json::kind::double_:
                        return "double";

                    case boost::json::kind::string:
                        return "string";

                    case boost::json::kind::array:
                        return "array";

                    case boost::json::kind::object:
                        return "object";

                    case boost::json::kind::null:
                    default:
                        return "null";
                }
            };

            return { "Boost.JSON expected type: " + std::move(expect_type)
                + ", got type: " + get_type_str() };
        }

        template<typename T>
        [[nodiscard]] static auto parse_arg(const boost::json::value& arg)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

            if (!validate_arg<no_ref_t>(arg))
            {
                throw function_mismatch{ mismatch_string(typeid(no_ref_t).name(), arg) };
            }

            no_ref_t out_val;
            deserializer ser{ arg };
            ser.deserialize_object(out_val);
            return out_val;
        }

        template<typename T>
        [[nodiscard]] static auto parse_args(const boost::json::value& arg_arr, size_t& index)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            if (!arg_arr.is_array())
            {
                return parse_arg<T>(arg_arr);
            }

            const auto& arr = arg_arr.get_array();

            if (index >= arr.size())
            {
                throw function_mismatch{ "Argument count mismatch" };
            }

            const auto old_idx = index;
            ++index;
            return parse_arg<T>(arr[old_idx]);
        }

        template<typename T>
        static auto yield_value(const boost::json::value& val) -> T
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                return boost::json::value_to<T>(val);
            }
            else if constexpr (detail::is_stringlike_v<T>)
            {
                return T{ val.get_string().c_str() };
            }
            else
            {
                T tmp_val;
                deserializer ser{ val };
                ser.deserialize_object(tmp_val);
                return tmp_val;
            }
        }

        boost::json::value m_json;
    };

    inline auto serial_adapter::from_bytes(std::string&& bytes) -> boost::json::object
    {
        boost::system::error_code err_code{};
        boost::json::value val = boost::json::parse(bytes, err_code);

        if (err_code)
        {
            throw deserialization_error{ err_code.what() };
        }

        if (!val.is_object())
        {
            throw deserialization_error{ "Boost::JSON: not an object" };
        }

        const auto& obj = val.get_object();

        if (const auto fname_it = obj.find("func_name");
            (fname_it == obj.cend()) || (!fname_it->value().is_string()))
        {
            throw deserialization_error{ "Boost::JSON: field \"func_name\" not found" };
        }

        return obj;
    }

    inline auto serial_adapter::to_bytes(const boost::json::object& serial_obj) -> std::string
    {
        return boost::json::serialize(serial_obj);
    }

    inline auto serial_adapter::to_bytes(boost::json::object&& serial_obj) -> std::string
    {
        return boost::json::serialize(serial_obj);
    }

    inline auto serial_adapter::get_func_name(const boost::json::object& serial_obj) -> std::string
    {
        return serial_obj.at("func_name").get_string().c_str();
    }

    inline auto serial_adapter::get_type(const boost::json::object& serial_obj) -> rpc_type
    {
        return static_cast<rpc_type>(serial_obj.at("type").get_int64());
    }

    template<bool IsCallback, typename R>
    auto serial_adapter::get_result(const boost::json::object& serial_obj)
        -> detail::rpc_result<IsCallback, R>
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                     == rpc_type::callback_result)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                    == rpc_type::func_result));

        detail::rpc_result<IsCallback, R> result;
        deserializer ser{ serial_obj };
        ser.deserialize_object(result);
        return result;
    }

    template<bool IsCallback, typename R>
    auto serial_adapter::serialize_result(const detail::rpc_result<IsCallback, R>& result)
        -> boost::json::object
    {
        serializer ser{};
        ser.serialize_object(result);
        return std::move(ser).object().get_object();
    }

    template<bool IsCallback, typename R, typename... Args>
    auto serial_adapter::get_result_w_bind(const boost::json::object& serial_obj)
        -> detail::rpc_result_w_bind<IsCallback, R, Args...>
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                     == rpc_type::callback_result_w_bind)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                    == rpc_type::func_result_w_bind));

        detail::rpc_result_w_bind<IsCallback, R> result;
        deserializer ser{ serial_obj };
        ser.deserialize_object(result);
        return result;
    }

    template<bool IsCallback, typename R, typename... Args>
    auto serial_adapter::serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) -> boost::json::object
    {
        serializer ser{};
        ser.serialize_object(result);
        return std::move(ser).object().get_object();
    }

    template<bool IsCallback, typename... Args>
    auto serial_adapter::get_request(const boost::json::object& serial_obj)
        -> detail::rpc_request<IsCallback, Args...>
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && (static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                         == rpc_type::callback_request
                                     || static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                         == rpc_type::callback_result_w_bind))
            || (!IsCallback
                && (static_cast<rpc_type>(serial_obj.at("type").as_int64())
                        == rpc_type::func_request
                    || static_cast<rpc_type>(serial_obj.at("type").as_int64())
                        == rpc_type::func_result_w_bind)));

        detail::rpc_request<IsCallback, Args...> request;
        deserializer ser{ serial_obj };
        ser.deserialize_object(request);
        return request;
    }

    template<bool IsCallback, typename... Args>
    auto serial_adapter::serialize_request(const detail::rpc_request<IsCallback, Args...>& request)
        -> boost::json::object
    {
        serializer ser{};
        ser.serialize_object(request);
        return std::move(ser).object().get_object();
    }

    template<bool IsCallback>
    auto serial_adapter::get_error(const boost::json::object& serial_obj)
        -> detail::rpc_error<IsCallback>
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                     == rpc_type::callback_error)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                    == rpc_type::func_error));

        detail::rpc_error<IsCallback> error;
        deserializer ser{ serial_obj };
        ser.deserialize_object(error);
        return error;
    }

    template<bool IsCallback>
    auto serial_adapter::serialize_error(const detail::rpc_error<IsCallback>& error)
        -> boost::json::object
    {
        serializer ser{};
        ser.serialize_object(error);
        return std::move(ser).object().get_object();
    }

    inline auto serial_adapter::get_callback_install(const boost::json::object& serial_obj)
        -> callback_install_request
    {
        RPC_HPP_PRECONDITION(static_cast<rpc_type>(serial_obj.at("type").as_int64())
            == rpc_type::callback_install_request);

        callback_install_request cbk_req;
        deserializer ser{ serial_obj };
        ser.deserialize_object(cbk_req);
        return cbk_req;
    }

    inline auto serial_adapter::serialize_callback_install(
        const callback_install_request& callback_req) -> boost::json::object
    {
        serializer ser{};
        ser.serialize_object(callback_req);
        return std::move(ser).object().get_object();
    }

    inline auto serial_adapter::has_bound_args(const boost::json::object& serial_obj) -> bool
    {
        return serial_obj.at("bind_args").as_bool();
    }
} //namespace detail_boost_json

using boost_json_adapter = detail_boost_json::serial_adapter;
} //namespace rpc_hpp::adapters
#endif
