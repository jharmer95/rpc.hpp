#ifndef RPC_CLIENT_HPP
#define RPC_CLIENT_HPP

#include "rpc.hpp"

#include <exception>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>

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

protected:
    client_interface(client_interface&&) noexcept = default;

    virtual void send(bytes_t&& bytes) = 0;
    virtual auto receive() -> bytes_t = 0;

    virtual void handle_callback_object(object_t& request)
    {
        throw object_mismatch_error{ "Invalid rpc_object type detected" };
    }

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
                    return handle_callback_object(response);

                case rpc_type::callback_install_request:
                case rpc_type::callback_error:
                case rpc_type::callback_result:
                case rpc_type::callback_result_w_bind:
                default:
                    throw object_mismatch_error{ "Invalid rpc_object type detected" };
            }
        }

        throw client_receive_error{ "Invalid RPC object received" };
    }
};

template<typename Serial>
class callback_client_interface : public client_interface<Serial>
{
public:
    using client_interface<Serial>::bytes_t;
    using client_interface<Serial>::object_t;
    using client_interface<Serial>::client_interface;
    using client_interface<Serial>::receive;
    using client_interface<Serial>::send;
    using client_interface<Serial>::recv_loop;

    virtual void uninstall_callback(callback_install_request&& callback) = 0;

    [[nodiscard]] auto has_callback(std::string_view func_name) const -> bool
    {
        return m_callback_map.find(func_name) != m_callback_map.cend();
    }

    template<typename R, typename... Args, typename S>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    auto install_callback(S&& func_name, std::function<R(Args...)> func) -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        auto result = install_callback_impl(std::forward<S>(func_name));
        bind_callback<R, Args...>(result.func_name, std::move(func));
        return result;
    }

    template<typename R, typename... Args, typename S>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    auto install_callback(S&& func_name, const detail::fptr_t<R, Args...> func_ptr)
        -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        auto result = install_callback_impl(std::forward<S>(func_name));
        bind_callback<R, Args...>(std::forward<S>(func_name), func_ptr);
        return result;
    }

    template<typename R, typename... Args, typename S, typename F>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    auto install_callback(S&& func_name, F&& func) -> callback_install_request
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        auto result = install_callback_impl(std::forward<S>(func_name));
        bind_callback<R, Args...>(std::forward<S>(func_name), std::forward<F>(func));
        return result;
    }

private:
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    virtual auto install_callback_impl(std::string func_name_sink) -> callback_install_request = 0;

    void handle_callback_object(object_t& request) final
    {
        if (const auto type = request.type();
            type == rpc_type::callback_request || type == rpc_type::func_request)
        {
            dispatch_callback(request);

            try
            {
                send(request.to_bytes());
            }
            catch (const std::exception& ex)
            {
                throw client_send_error{ ex.what() };
            }

            recv_loop(request);
        }
        else
        {
            client_interface<Serial>::handle_callback_object(request);
        }
    }

    template<typename R, typename... Args, typename S, typename F>
    void bind_callback(S&& func_name, F&& func)
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");

        auto [_, status] = m_callback_map.try_emplace(std::forward<S>(func_name),
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

        if (!status)
        {
            // NOTE: `try_emplace` does not move func_name unless successful, so the use of it here is safe
            throw callback_install_error{ std::string{
                "RPC error: Client could not install callback: " }
                                              .append(std::forward<S>(func_name))
                                              .append("() successfully") };
        }
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
            function_missing_error{
                std::string{ "RPC error: Called function: " }.append(func_name).append(
                    "() not found") } } };
    }

    std::unordered_map<std::string, std::function<void(object_t&)>> m_callback_map{};
};
} //namespace rpc_hpp
#endif
