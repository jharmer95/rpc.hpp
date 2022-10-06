#ifndef RPC_HPP_DETAIL_RPC_TYPES_HPP
#define RPC_HPP_DETAIL_RPC_TYPES_HPP

#include "../adapters/serializer.hpp"
#include "../rpc_exceptions.hpp"
#include "traits.hpp"

#include <string>
#include <tuple>
#include <utility>

namespace rpc_hpp::detail
{
template<bool IsCallback>
struct rpc_base
{
    static constexpr bool is_callback = IsCallback;
    std::string func_name{};
};

template<bool IsCallback, typename... Args>
struct rpc_request : rpc_base<IsCallback>
{
    using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;

    rpc_request() noexcept = default;

    rpc_request(std::string t_func_name, args_t t_args, bool t_bind_args = false)
        : rpc_base<IsCallback>{ std::move(t_func_name) },
          bind_args(t_bind_args),
          args(std::move(t_args))
    {
    }

    bool bind_args{ false };
    args_t args{};
};

template<typename Adapter, bool Deserialize, bool IsCallback, typename... Args>
void serialize(
    adapters::serializer_base<Adapter, Deserialize>& ser, rpc_request<IsCallback, Args...>& rpc_obj)
{
    auto type = []
    {
        if constexpr (IsCallback)
        {
            return static_cast<int>(rpc_type::callback_request);
        }
        else
        {
            return static_cast<int>(rpc_type::func_request);
        }
    }();

    ser.as_int("type", type);
    ser.as_string("func_name", rpc_obj.func_name);
    ser.as_bool("bind_args", rpc_obj.bind_args);
    ser.as_tuple("args", rpc_obj.args);
}

template<typename... Args>
using func_request = rpc_request<false, Args...>;

template<typename... Args>
using callback_request = rpc_request<true, Args...>;

template<bool IsCallback, typename R>
struct rpc_result : rpc_base<IsCallback>
{
    R result{};
};

template<bool IsCallback>
struct rpc_result<IsCallback, void> : rpc_base<IsCallback>
{
};

template<typename Adapter, bool Deserialize, bool IsCallback, typename R>
void serialize(
    adapters::serializer_base<Adapter, Deserialize>& ser, rpc_result<IsCallback, R>& rpc_obj)
{
    auto type = []
    {
        if constexpr (IsCallback)
        {
            return static_cast<int>(rpc_type::callback_result);
        }
        else
        {
            return static_cast<int>(rpc_type::func_result);
        }
    }();

    ser.as_int("type", type);
    ser.as_string("func_name", rpc_obj.func_name);

    if constexpr (!std::is_void_v<R>)
    {
        ser.as_object("result", rpc_obj.result);
    }
}

template<typename R>
using func_result = rpc_result<false, R>;

template<typename R>
using callback_result = rpc_result<true, R>;

template<bool IsCallback, typename R, typename... Args>
struct rpc_result_w_bind : rpc_result<IsCallback, R>
{
    using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;

    rpc_result_w_bind() noexcept = default;
    rpc_result_w_bind(std::string t_func_name, R t_result, args_t t_args)
        : rpc_result<IsCallback, R>{ std::move(t_func_name), std::move(t_result) },
          args(std::move(t_args))
    {
    }

    args_t args{};
};

template<bool IsCallback, typename... Args>
struct rpc_result_w_bind<IsCallback, void, Args...> : rpc_request<IsCallback, Args...>
{
    using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;
    using rpc_request<IsCallback, Args...>::rpc_request;
};

template<typename Adapter, bool Deserialize, bool IsCallback, typename R, typename... Args>
void serialize(adapters::serializer_base<Adapter, Deserialize>& ser,
    rpc_result_w_bind<IsCallback, R, Args...>& rpc_obj)
{
    auto type = []
    {
        if constexpr (IsCallback)
        {
            return static_cast<int>(rpc_type::callback_result_w_bind);
        }
        else
        {
            return static_cast<int>(rpc_type::func_result_w_bind);
        }
    }();

    bool bind_args = true;

    ser.as_int("type", type);
    ser.as_string("func_name", rpc_obj.func_name);
    ser.as_bool("bind_args", bind_args);
    ser.as_tuple("args", rpc_obj.args);

    if constexpr (!std::is_void_v<R>)
    {
        ser.as_object("result", rpc_obj.result);
    }
}

template<typename R, typename... Args>
using func_result_w_bind = rpc_result_w_bind<false, R, Args...>;

template<typename R, typename... Args>
using callback_result_w_bind = rpc_result_w_bind<true, R, Args...>;

template<bool IsCallback>
struct rpc_error : rpc_base<IsCallback>
{
    rpc_error() noexcept = default;
    rpc_error(std::string t_func_name, const rpc_exception& except)
        : rpc_base<IsCallback>{ std::move(t_func_name) },
          except_type(except.get_type()),
          err_mesg(except.what())
    {
    }

    rpc_error(std::string t_func_name, const exception_type t_ex_type, std::string t_err_mesg)
        : rpc_base<IsCallback>{ std::move(t_func_name) },
          except_type(t_ex_type),
          err_mesg(std::move(t_err_mesg))
    {
    }

    exception_type except_type{ exception_type::none };
    std::string err_mesg{};
};

template<bool IsCallback>
[[noreturn]] void rpc_throw(const rpc_error<IsCallback>& err) noexcept(false)
{
    switch (err.except_type)
    {
        case exception_type::func_not_found:
            throw function_not_found{ err.err_mesg };

        case exception_type::remote_exec:
            throw remote_exec_error{ err.err_mesg };

        case exception_type::serialization:
            throw serialization_error{ err.err_mesg };

        case exception_type::deserialization:
            throw deserialization_error{ err.err_mesg };

        case exception_type::signature_mismatch:
            throw function_mismatch{ err.err_mesg };

        case exception_type::client_send:
            throw client_send_error{ err.err_mesg };

        case exception_type::client_receive:
            throw client_receive_error{ err.err_mesg };

        case exception_type::server_send:
            throw server_send_error{ err.err_mesg };

        case exception_type::server_receive:
            throw server_receive_error{ err.err_mesg };

        case exception_type::rpc_object_mismatch:
            throw rpc_object_mismatch{ err.err_mesg };

        case exception_type::callback_install:
            throw callback_install_error{ err.err_mesg };

        case exception_type::callback_missing:
            throw callback_missing_error{ err.err_mesg };

        case exception_type::none:
        default:
            throw rpc_exception{ err.err_mesg, exception_type::none };
    }
}

template<typename Adapter, bool Deserialize, bool IsCallback>
void serialize(adapters::serializer_base<Adapter, Deserialize>& ser, rpc_error<IsCallback>& rpc_obj)
{
    auto type = []
    {
        if constexpr (IsCallback)
        {
            return static_cast<int>(rpc_type::callback_error);
        }
        else
        {
            return static_cast<int>(rpc_type::func_error);
        }
    }();

    ser.as_int("type", type);
    ser.as_string("func_name", rpc_obj.func_name);
    ser.as_int("except_type", rpc_obj.except_type);
    ser.as_string("err_mesg", rpc_obj.err_mesg);
}

using func_error = rpc_error<false>;
using callback_error = rpc_error<true>;

struct callback_install_request : detail::rpc_base<true>
{
    callback_install_request() noexcept = default;
    explicit callback_install_request(std::string t_func_name) noexcept
        : rpc_base<true>{ std::move(t_func_name) }
    {
    }

    bool is_uninstall{ false };
};

template<typename Adapter, bool Deserialize>
void serialize(
    adapters::serializer_base<Adapter, Deserialize>& ser, callback_install_request& rpc_obj)
{
    auto type = static_cast<int>(rpc_type::callback_install_request);
    ser.as_int("type", type);
    ser.as_string("func_name", rpc_obj.func_name);
    ser.as_bool("is_uninstall", rpc_obj.is_uninstall);
}
} //namespace rpc_hpp::detail

#endif
