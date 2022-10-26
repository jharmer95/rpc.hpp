#ifndef RPC_CLIENT_HPP
#define RPC_CLIENT_HPP

#include "rpc.hpp"

#include <exception>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#if defined(RPC_HPP_ENABLE_CALLBACKS)
#  include <functional>
#  include <unordered_map>
#endif

#if !defined(RPC_HEADER_FUNC)
#  define RPC_HEADER_FUNC(RT, FNAME, ...) inline RT (*FNAME)(__VA_ARGS__) = nullptr
#  define RPC_HEADER_FUNC_EXTC(RT, FNAME, ...) extern "C" inline RT (*FNAME)(__VA_ARGS__) = nullptr
#  define RPC_HEADER_FUNC_NOEXCEPT(RT, FNAME, ...) \
    inline RT (*FNAME)(__VA_ARGS__) noexcept = nullptr
#endif

#define call_header_func(FNAME, ...) call_header_func_impl(FNAME, #FNAME, __VA_ARGS__)

namespace rpc_hpp
{
template<typename Serial>
class client_interface
{
public:
    using bytes_t = typename Serial::bytes_t;
    using object_t = rpc_object<Serial>;

    virtual ~client_interface() noexcept = default;
    client_interface() = default;
    client_interface(const client_interface&) = delete;
    client_interface& operator=(const client_interface&) = delete;
    client_interface& operator=(client_interface&&) = delete;

    template<typename... Args, typename S>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    auto call_func(S&& func_name, Args&&... args) -> object_t
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        auto response = object_t{ detail::func_request<detail::decay_str_t<Args>...>{
            std::forward<S>(func_name), std::forward_as_tuple(args...) } };

        try
        {
            send(response.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error{ ex.what() };
        }

        recv_loop(response);
        return response;
    }

    template<typename... Args, typename S>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    auto call_func_w_bind(S&& func_name, Args&&... args) -> object_t
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        auto response = object_t{ detail::func_request<detail::decay_str_t<Args>...>{
            std::forward<S>(func_name), std::forward_as_tuple(args...), true } };

        try
        {
            send(response.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error{ ex.what() };
        }

        recv_loop(response);
        detail::tuple_bind(response.template get_args<detail::decay_str_t<Args>...>(),
            std::forward<Args>(args)...);

        return response;
    }

    template<typename R, typename... Args, typename... Args2, typename S>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    auto call_header_func_impl(RPC_HPP_UNUSED const detail::fptr_t<R, Args...> func, S&& func_name,
        Args2&&... args) -> object_t
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

#ifdef __cpp_lib_is_nothrow_convertible
        static_assert(std::conjunction_v<std::is_nothrow_convertible<Args2, Args>...>,
            "Static function call parameters must match type exactly");
#else
        static_assert(std::conjunction_v<std::is_convertible<Args2, Args>...>,
            "Static function call parameters must match type exactly");
#endif

        // If any parameters are non-const lvalue references...
        if constexpr (detail::has_ref_args<Args...>())
        {
            return call_func_w_bind<Args2...>(
                std::forward<S>(func_name), std::forward<Args2>(args)...);
        }
        else
        {
            return call_func<Args2...>(std::forward<S>(func_name), std::forward<Args2>(args)...);
        }
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    auto has_callback(std::string_view func_name) -> bool
    {
        return m_callback_map.find(func_name) != m_callback_map.end();
    }

    template<typename R, typename... Args, typename S>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    RPC_HPP_INLINE auto install_callback(S&& func_name, std::function<R(Args...)> func)
        -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        return install_callback_impl<R, Args...>(std::forward<S>(func_name), std::move(func));
    }

    template<typename R, typename... Args, typename S>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    RPC_HPP_INLINE auto install_callback(S&& func_name, const detail::fptr_t<R, Args...> func_ptr)
        -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        return install_callback_impl<R, Args...>(std::forward<S>(func_name), func_ptr);
    }

    template<typename R, typename... Args, typename S, typename F>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    RPC_HPP_INLINE auto install_callback(S&& func_name, F&& func) -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        return install_callback_impl<R, Args...>(
            std::forward<S>(func_name), std::function<R(Args...)>{ std::forward<F>(func) });
    }

    void uninstall_callback(callback_install_request&& callback)
    {
        callback.is_uninstall = true;
        send(object_t{ std::move(callback) }.to_bytes());

        if (const auto response = object_t::parse_bytes(receive());
            !response.has_value() || response.value().type() != rpc_type::callback_install_request)
        {
            throw callback_install_error{
                "server did not respond to callback_install_request (uninstall)"
            };
        }
    }
#endif

protected:
    client_interface(client_interface&&) noexcept = default;

    virtual void send(bytes_t&& bytes) = 0;
    virtual auto receive() -> bytes_t = 0;

private:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    template<typename R, typename... Args, typename S, typename F>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    auto install_callback_impl(S&& func_name, F&& func) -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        callback_install_request cb{ std::forward<S>(func_name) };

        m_callback_map.try_emplace(cb.func_name,
            [func = std::forward<F>(func)](object_t& rpc_obj)
            {
                try
                {
                    detail::exec_func<true, Serial, R, Args...>(func, rpc_obj);
                }
                catch (const rpc_exception& ex)
                {
                    rpc_obj = object_t{ detail::callback_error{ rpc_obj.get_func_name(), ex } };
                }
            });

        object_t request{ cb };

        try
        {
            send(request.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error{ ex.what() };
        }

        if (auto response = object_t::parse_bytes(receive());
            !response.has_value() || response.value().type() != rpc_type::callback_install_request)
        {
            throw callback_install_error{ "server did not respond to callback_install_request" };
        }

        return cb;
    }

    void dispatch_callback(object_t& rpc_obj)
    {
        const auto func_name = rpc_obj.get_func_name();

        if (const auto it = m_callback_map.find(func_name); it != m_callback_map.cend())
        {
            it->second(rpc_obj);
            return;
        }

        rpc_obj = object_t{ detail::callback_error{ func_name,
            function_not_found{
                std::string{ "RPC error: Called function: \"" }.append(func_name).append(
                    "\" not found") } } };
    }
#endif

    void recv_loop(object_t& response)
    {
        bytes_t bytes = [this]
        {
            try
            {
                return receive();
            }
            catch (const std::exception& ex)
            {
                throw client_receive_error{ ex.what() };
            }
        }();

        if (auto response_opt = object_t::parse_bytes(std::move(bytes)); response_opt.has_value())
        {
            switch (response = std::move(response_opt).value(); response.type())
            {
                case rpc_type::func_result:
                case rpc_type::func_result_w_bind:
                case rpc_type::func_error:
                    return;

                case rpc_type::callback_request:
                case rpc_type::func_request:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
                {
                    dispatch_callback(response);

                    try
                    {
                        send(response.to_bytes());
                    }
                    catch (const std::exception& ex)
                    {
                        throw client_send_error{ ex.what() };
                    }

                    return recv_loop(response);
                }
#else
                    [[fallthrough]];
#endif

                case rpc_type::callback_install_request:
                case rpc_type::callback_error:
                case rpc_type::callback_result:
                case rpc_type::callback_result_w_bind:
                default:
                    throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
            }
        }

        throw client_receive_error{ "Invalid RPC object received" };
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    std::unordered_map<std::string, std::function<void(object_t&)>> m_callback_map{};
#endif
};
} //namespace rpc_hpp
#endif
