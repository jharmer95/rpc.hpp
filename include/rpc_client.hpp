#pragma once

#include "rpc.hpp"

#define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) inline RETURN (*FUNCNAME)(__VA_ARGS__) = nullptr
#define call_header_func(FUNCNAME, ...) call_header_func_impl(FUNCNAME, #FUNCNAME, __VA_ARGS__)

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

    template<typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_func(const std::string& func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<false, detail::decay_str_t<Args>...>{
            func_name, std::forward_as_tuple(args...) } };

        try
        {
            send(response.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error(ex.what());
        }

        response = recv_loop();
        return response;
    }

    template<typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_func(std::string&& func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<false, detail::decay_str_t<Args>...>{
            std::move(func_name), std::forward_as_tuple(args...) } };

        try
        {
            send(response.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error(ex.what());
        }

        response = recv_loop();
        return response;
    }

    template<typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_func_w_bind(const std::string& func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<false, detail::decay_str_t<Args>...>{
            detail::bind_args_tag{}, func_name, std::forward_as_tuple(args...) } };

        try
        {
            send(response.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error(ex.what());
        }

        response = recv_loop();
        detail::tuple_bind(response.template get_args<false, detail::decay_str_t<Args>...>(),
            std::forward<Args>(args)...);

        return response;
    }

    template<typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_func_w_bind(std::string&& func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<false, detail::decay_str_t<Args>...>{
            detail::bind_args_tag{}, std::move(func_name), std::forward_as_tuple(args...) } };

        try
        {
            send(response.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error(ex.what());
        }

        response = recv_loop();
        detail::tuple_bind(response.template get_args<false, detail::decay_str_t<Args>...>(),
            std::forward<Args>(args)...);

        return response;
    }

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_header_func_impl(
        RPC_HPP_UNUSED R (*func)(Args...), const std::string& func_name, Args&&... args)
    {
        // If any parameters are non-const lvalue references...
        if constexpr (detail::has_ref_args<Args...>())
        {
            return call_func_w_bind<Args...>(func_name, std::forward<Args>(args)...);
        }
        else
        {
            return call_func<Args...>(func_name, std::forward<Args>(args)...);
        }
    }

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_header_func_impl(
        RPC_HPP_UNUSED R (*func)(Args...), std::string&& func_name, Args&&... args)
    {
        // If any parameters are non-const lvalue references...
        if constexpr (detail::has_ref_args<Args...>())
        {
            return call_func_w_bind<Args...>(std::move(func_name), std::forward<Args>(args)...);
        }
        else
        {
            return call_func<Args...>(std::move(func_name), std::forward<Args>(args)...);
        }
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    callback_install_request
        install_callback(std::string func_name, const std::function<R(Args...)>& func)
    {
        callback_install_request cb{ std::move(func_name) };

        m_callback_map.try_emplace(cb.func_name,
            [&func](object_t& rpc_obj)
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
            throw client_send_error(ex.what());
        }

        auto response = object_t::parse_bytes(receive());

        if (!response.has_value() || response.value().type() != rpc_type::callback_install_request)
        {
            throw callback_install_error("server did not respond to callback_install_request");
        }

        return cb;
    }

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    callback_install_request
        install_callback(std::string func_name, std::function<R(Args...)>&& func)
    {
        callback_install_request cb{ std::move(func_name) };

        m_callback_map.try_emplace(cb.func_name,
            [func = std::move(func)](object_t& rpc_obj)
            {
                try
                {
                    detail::exec_func<true, Serial, R, Args...>(std::move(func), rpc_obj);
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
            throw client_send_error(ex.what());
        }

        auto response = object_t::parse_bytes(receive());

        if (!response.has_value() || response.value().type() != rpc_type::callback_install_request)
        {
            throw callback_install_error("server did not respond to callback_install_request");
        }

        return cb;
    }

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    callback_install_request install_callback(std::string func_name, R (*func_ptr)(Args...))
    {
        callback_install_request cb{ std::move(func_name) };

        m_callback_map.try_emplace(cb.func_name,
            [func_ptr](object_t& rpc_obj)
            {
                try
                {
                    detail::exec_func<true, Serial, R, Args...>(func_ptr, rpc_obj);
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
            throw client_send_error(ex.what());
        }

        auto response = object_t::parse_bytes(receive());

        if (!response.has_value() || response.value().type() != rpc_type::callback_install_request)
        {
            throw callback_install_error("server did not respond to callback_install_request");
        }

        return cb;
    }

    template<typename R, typename... Args, typename F>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    callback_install_request install_callback(std::string func_name, F&& func)
    {
        return install_callback(
            std::move(func_name), std::function<R(Args...)>{ std::forward<F>(func) });
    }

    void uninstall_callback(callback_install_request&& callback)
    {
        callback.is_uninstall = true;
        object_t request{ std::move(callback) };
        send(request.to_bytes());

        auto response = object_t::parse_bytes(receive());

        if (!response.has_value() || response.value().type() != rpc_type::callback_install_request)
        {
            throw callback_install_error(
                "server did not respond to callback_install_request (uninstall)");
        }
    }
#endif

protected:
    client_interface(client_interface&&) noexcept = default;

    virtual void send(bytes_t&& bytes) = 0;
    virtual bytes_t receive() = 0;

private:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    void dispatch_callback(object_t& rpc_obj)
    {
        const auto func_name = rpc_obj.get_func_name();

        if (const auto it = m_callback_map.find(func_name); it != m_callback_map.cend())
        {
            it->second(rpc_obj);
            return;
        }

        rpc_obj = object_t{ detail::callback_error{ func_name,
            function_not_found("RPC error: Called function: \"" + func_name + "\" not found") } };
    }
#endif

    object_t recv_loop()
    {
        bytes_t bytes;

        try
        {
            bytes = receive();
        }
        catch (const std::exception& ex)
        {
            throw client_receive_error(ex.what());
        }

        if (auto o_response = object_t::parse_bytes(std::move(bytes)); o_response.has_value())
        {
            switch (auto& response = o_response.value(); response.type())
            {
                case rpc_type::func_result:
                case rpc_type::func_result_w_bind:
                case rpc_type::func_error:
                    return response;

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
                        throw client_send_error(ex.what());
                    }

                    return recv_loop();
                }
#else
                    [[fallthrough]];
#endif

                case rpc_type::callback_install_request:
                case rpc_type::callback_error:
                case rpc_type::callback_result:
                case rpc_type::callback_result_w_bind:
                default:
                    throw rpc_object_mismatch("Invalid rpc_object type detected");
            }
        }

        throw client_receive_error("Invalid RPC object received");
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    std::unordered_map<std::string, std::function<void(object_t&)>> m_callback_map{};
#endif
};
} //namespace rpc_hpp
