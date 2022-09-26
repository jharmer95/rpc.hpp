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
#include "rpc_adapters/rpc_njson.hpp"

#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>

namespace rpc_hpp::adapters
{
class boost_json_adapter;

template<>
struct serial_traits<boost_json_adapter>
{
    using serial_t = boost::json::object;
    using bytes_t = std::string;
};

class boost_json_serializer : public serializer<boost_json_serializer, false>
{
public:
    boost_json_serializer() noexcept { m_json.emplace_object(); }

    [[nodiscard]] const boost::json::value& object() const& { return m_json; }
    [[nodiscard]] boost::json::value&& object() && { return std::move(m_json); }

    template<typename T>
    void as_bool(std::string_view key, T& val)
    {
        subobject(key) = static_cast<bool>(val);
    }

    template<typename T>
    void as_float(std::string_view key, T& val)
    {
        subobject(key) = val;
    }

    template<typename T>
    void as_int(std::string_view key, T& val)
    {
        subobject(key) = val;
    }

    template<typename T>
    void as_string(std::string_view key, T& val)
    {
        subobject(key) = val;
    }

    template<typename T>
    void as_array(std::string_view key, T& val)
    {
        auto arr = boost::json::array{};

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
    void as_map(std::string_view key, T& val)
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
    void as_multimap(std::string_view key, T& val)
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

private:
    [[nodiscard]] boost::json::value& subobject(std::string_view key)
    {
        return key.empty() ? m_json : m_json.get_object()[key];
    }

    boost::json::value m_json{};
};

class boost_json_deserializer : public serializer<boost_json_deserializer, true>
{
public:
    explicit boost_json_deserializer(const boost::json::value& obj) : m_json(obj) {}
    explicit boost_json_deserializer(boost::json::value&& obj) : m_json(std::move(obj)) {}

    template<typename T>
    void as_bool(std::string_view key, T& val) const
    {
        val = boost::json::value_to<bool>(subobject(key));
    }

    template<typename T>
    void as_float(std::string_view key, T& val) const
    {
        val = boost::json::value_to<T>(subobject(key));
    }

    template<typename T>
    void as_int(std::string_view key, T& val) const
    {
        val = boost::json::value_to<T>(subobject(key));
    }

    template<typename T>
    void as_string(std::string_view key, T& val) const
    {
        val = subobject(key).get_string().c_str();
    }

    template<typename T>
    void as_array(std::string_view key, T& val) const
    {
        const auto& arr = subobject(key).as_array();

        std::transform(arr.begin(), arr.end(), std::inserter(val, val.end()),
            yield_value<detail::remove_cvref_t<typename T::value_type>>);
    }

    template<typename T, size_t N>
    void as_array(std::string_view key, std::array<T, N>& val) const
    {
        const auto& arr = subobject(key).as_array();

        if (arr.size() != N)
        {
            throw std::out_of_range("JSON array out of bounds");
        }

        std::transform(
            arr.cbegin(), arr.cend(), val.begin(), yield_value<detail::remove_cvref_t<T>>);
    }

    template<typename T, typename Alloc>
    void as_array(std::string_view key, std::forward_list<T, Alloc>& val) const
    {
        const auto& arr = subobject(key).as_array();

        const auto arr_rend = arr.crend();

        for (auto it = arr.crbegin(); it != arr_rend; ++it)
        {
            val.push_front(yield_value<detail::remove_cvref_t<T>>(*it));
        }
    }

    template<typename T>
    void as_map(std::string_view key, T& val) const
    {
        const auto& obj = subobject(key).as_object();

        for (const auto& [k, v] : obj)
        {
            boost::json::value key_val = boost::json::parse(k).as_array().front();
            val.insert({ boost::json::value_to<typename T::key_type>(key_val),
                boost::json::value_to<typename T::mapped_type>(v) });
        }
    }

    template<typename T>
    void as_multimap(std::string_view key, T& val) const
    {
        const auto& obj = subobject(key).as_object();

        for (const auto& [k, v] : obj)
        {
            for (const auto& subval : v.as_array())
            {
                boost::json::value key_val = boost::json::parse(k).as_array().front();
                val.insert({ boost::json::value_to<typename T::key_type>(key_val),
                    boost::json::value_to<typename T::mapped_type>(subval) });
            }
        }
    }

private:
    [[nodiscard]] const boost::json::value& subobject(std::string_view key) const
    {
        return key.empty() ? m_json : m_json.at(key);
    }

    template<typename T>
    static T yield_value(const boost::json::value& val)
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
            boost_json_deserializer ser{ val };
            T tmp_val{};
            ser.deserialize_object(tmp_val);
            return tmp_val;
        }
    }

    boost::json::value m_json{};
};

class boost_json_adapter : public serial_adapter_base<boost_json_adapter>
{
public:
    [[nodiscard]] static boost::json::object from_bytes(std::string&& bytes)
    {
        boost::system::error_code err_code;
        boost::json::value val = boost::json::parse(bytes, err_code);

        if (err_code)
        {
            throw deserialization_error(err_code.what());
        }

        if (!val.is_object())
        {
            throw deserialization_error("Boost::JSON: not an object");
        }

        auto& obj = val.get_object();

        if (const auto fname_it = obj.find("func_name");
            (fname_it == obj.end()) || (!fname_it->value().is_string()))
        {
            throw deserialization_error("Boost::JSON: filed \"func_name\" not found or empty");
        }

        return obj;
    }

    [[nodiscard]] static std::string to_bytes(const boost::json::object& serial_obj)
    {
        return boost::json::serialize(serial_obj);
    }

    [[nodiscard]] static std::string to_bytes(boost::json::object&& serial_obj)
    {
        return boost::json::serialize(serial_obj);
    }

    [[nodiscard]] static std::string get_func_name(const boost::json::object& serial_obj)
    {
        return serial_obj.at("func_name").get_string().c_str();
    }

    [[nodiscard]] static rpc_type get_type(const boost::json::object& serial_obj)
    {
        return static_cast<rpc_type>(serial_obj.at("type").get_int64());
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static detail::rpc_result<IsCallback, R> get_result(
        const boost::json::object& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                     == rpc_type::callback_result)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                    == rpc_type::func_result));

        if constexpr (std::is_void_v<R>)
        {
            return { serial_obj.at("func_name").get_string().c_str() };
        }
        else
        {
            return { serial_obj.at("func_name").get_string().c_str(),
                parse_arg<R>(serial_obj.at("result")) };
        }
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static boost::json::object serialize_result(
        const detail::rpc_result<IsCallback, R>& result)
    {
        boost::json::object obj{};
        obj["func_name"] = result.func_name;

        if constexpr (!std::is_void_v<R>)
        {
            obj["result"] = {};
            push_arg(result.result, obj["result"]);
        }

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int64_t>(rpc_type::callback_result);
        }
        else
        {
            obj["type"] = static_cast<int64_t>(rpc_type::func_result);
        }

        return obj;
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static detail::rpc_result_w_bind<IsCallback, R, Args...> get_result_w_bind(
        const boost::json::object& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                     == rpc_type::callback_result_w_bind)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                    == rpc_type::func_result_w_bind));

        const auto& args_val = serial_obj.at("args");
        RPC_HPP_UNUSED unsigned arg_counter = 0;

        if constexpr (std::is_void_v<R>)
        {
            return { serial_obj.at("func_name").get_string().c_str(),
                parse_args<Args>(args_val, arg_counter)... };
        }
        else
        {
            return { serial_obj.at("func_name").get_string().c_str(),
                parse_arg<R>(serial_obj.at("result"), parse_args<Args>(args_val, arg_counter)...) };
        }
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static boost::json::object serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result)
    {
        boost::json::object obj{};
        obj["func_name"] = result.func_name;

        if constexpr (!std::is_void_v<R>)
        {
            obj["result"] = {};
            push_arg(result.result, obj["result"]);
        }

        auto arg_arr = boost::json::array{};
        arg_arr.reserve(sizeof...(Args));
        obj["bind_args"] = true;

        detail::for_each_tuple(result.args,
            [&arg_arr](auto&& elem) { push_args(std::forward<decltype(elem)>(elem), arg_arr); });

        obj["args"] = std::move(arg_arr);

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int64_t>(rpc_type::callback_result_w_bind);
        }
        else
        {
            obj["type"] = static_cast<int64_t>(rpc_type::func_result_w_bind);
        }

        return obj;
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static detail::rpc_request<IsCallback, Args...> get_request(
        const boost::json::object& serial_obj)
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

        const auto& args_val = serial_obj.at("args");
        const bool is_bound_args = serial_obj.at("bind_args").get_bool();

        if (args_val.as_array().size() != sizeof...(Args))
        {
            throw function_mismatch("Argument count mismatch");
        }

        RPC_HPP_UNUSED unsigned arg_counter = 0;
        typename detail::rpc_request<IsCallback, Args...>::args_t args = { parse_args<Args>(
            args_val, arg_counter)... };

        return is_bound_args ? detail::rpc_request<IsCallback, Args...>{ detail::bind_args_tag{},
            serial_obj.at("func_name").get_string().c_str(), std::move(args) }
                             : detail::rpc_request<IsCallback, Args...>{
                                   serial_obj.at("func_name").get_string().c_str(), std::move(args)
                               };
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static boost::json::object serialize_request(
        const detail::rpc_request<IsCallback, Args...>& request)
    {
        boost::json::object obj{};
        obj["func_name"] = request.func_name;
        auto arg_arr = boost::json::array{};
        arg_arr.reserve(sizeof...(Args));
        obj["bind_args"] = request.bind_args;

        detail::for_each_tuple(request.args,
            [&arg_arr](auto&& elem) { push_args(std::forward<decltype(elem)>(elem), arg_arr); });

        obj["args"] = std::move(arg_arr);

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int64_t>(rpc_type::callback_request);
        }
        else
        {
            obj["type"] = static_cast<int64_t>(rpc_type::func_request);
        }

        const auto dbg_str = boost::json::serialize(obj);
        return obj;
    }

    template<bool IsCallback>
    [[nodiscard]] static detail::rpc_error<IsCallback> get_error(
        const boost::json::object& serial_obj)
    {
        RPC_HPP_PRECONDITION((IsCallback
                                 && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                                     == rpc_type::callback_error)
            || (!IsCallback
                && static_cast<rpc_type>(serial_obj.at("type").as_int64())
                    == rpc_type::func_error));

        return { serial_obj.at("func_name").get_string().c_str(),
            static_cast<exception_type>(serial_obj.at("except_type").get_int64()),
            serial_obj.at("err_mesg").get_string().c_str() };
    }

    template<bool IsCallback>
    [[nodiscard]] static boost::json::object serialize_error(
        const detail::rpc_error<IsCallback>& error)
    {
        boost::json::object obj{};
        obj["func_name"] = error.func_name;
        obj["err_mesg"] = error.err_mesg;
        obj["except_type"] = static_cast<int64_t>(error.except_type);

        if constexpr (IsCallback)
        {
            obj["type"] = static_cast<int64_t>(rpc_type::callback_error);
        }
        else
        {
            obj["type"] = static_cast<int64_t>(rpc_type::func_error);
        }

        return obj;
    }

    [[nodiscard]] static callback_install_request get_callback_install(
        const boost::json::object& serial_obj)
    {
        RPC_HPP_PRECONDITION(static_cast<rpc_type>(serial_obj.at("type").as_int64())
            == rpc_type::callback_install_request);

        callback_install_request callback_req{ serial_obj.at("func_name").get_string().c_str() };
        callback_req.is_uninstall = serial_obj.at("is_uninstall").get_bool();
        return callback_req;
    }

    [[nodiscard]] static boost::json::object serialize_callback_install(
        const callback_install_request& callback_req)
    {
        boost::json::object obj{};
        obj["func_name"] = callback_req.func_name;
        obj["is_uninstall"] = callback_req.is_uninstall;
        obj["type"] = static_cast<int64_t>(rpc_type::callback_install_request);
        return obj;
    }

    [[nodiscard]] static bool has_bound_args(const boost::json::object& serial_obj)
    {
        return serial_obj.at("bind_args").as_bool();
    }

private:
    template<typename T>
    [[nodiscard]] static constexpr bool validate_arg(const boost::json::value& arg) noexcept
    {
        if constexpr (std::is_same_v<T, bool>)
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
        else if constexpr (rpc_hpp::detail::is_container_v<T>)
        {
            return arg.is_array();
        }
        else
        {
            return !arg.is_null();
        }
    }

    [[nodiscard]] static std::string mismatch_string(
        std::string&& expect_type, const boost::json::value& obj)
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
    static void push_arg(T&& arg, boost::json::value& obj)
    {
        boost_json_serializer ser;
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

    template<typename T>
    [[nodiscard]] static detail::remove_cvref_t<detail::decay_str_t<T>> parse_arg(
        const boost::json::value& arg)
    {
        using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

        if (!validate_arg<no_ref_t>(arg))
        {
            throw function_mismatch{ mismatch_string(typeid(no_ref_t).name(), arg) };
        }

        boost_json_deserializer ser{ arg };
        no_ref_t out_val;
        ser.deserialize_object(out_val);
        return out_val;
    }

    template<typename T>
    [[nodiscard]] static detail::remove_cvref_t<detail::decay_str_t<T>> parse_args(
        const boost::json::value& arg_arr, unsigned& index)
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
};
} //namespace rpc_hpp::adapters
#endif
