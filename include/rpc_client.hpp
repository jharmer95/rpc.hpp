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

    // nodiscard because the rpc_object should be checked for its type
    template<typename... Args>
    [[nodiscard]] object_t call_func(std::string func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<detail::decay_str_t<Args>...>{
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

    // nodiscard because the rpc_object should be checked for its type
    template<typename... Args>
    [[nodiscard]] object_t call_func_w_bind(std::string func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<detail::decay_str_t<Args>...>{
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
        detail::tuple_bind(response.template get_args<detail::decay_str_t<Args>...>(),
            std::forward<Args>(args)...);

        return response;
    }

    // nodiscard because the rpc_object should be checked for its type
    template<typename R, typename... Args>
    [[nodiscard]] object_t call_header_func_impl(
        RPC_HPP_UNUSED R (*func)(Args...), std::string func_name, Args&&... args)
    {
        return call_func_w_bind<Args...>(std::move(func_name), std::forward<Args>(args)...);
    }

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    template<typename R, typename... Args>
    callback_install_request install_callback(std::string func_name, R (*func)(Args...))
    {
        callback_install_request cb{ std::move(func_name), reinterpret_cast<size_t>(func) };
        object_t request{ cb };

        try
        {
            send(request.to_bytes());
        }
        catch (const std::exception& ex)
        {
            throw client_send_error(ex.what());
        }

        return cb;
    }

    void uninstall_callback(callback_install_request&& callback)
    {
        callback.set_uninstall(true);
        object_t request{ std::move(callback) };
        send(request.to_bytes());
    }
#endif

protected:
    client_interface(client_interface&&) noexcept = default;

    virtual void send(bytes_t&& bytes) = 0;
    virtual bytes_t receive() = 0;

private:
    void dispatch_callback(object_t& rpc_obj)
    {
        // TODO: Implement this
    }

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

                case rpc_type::callback_error:
                case rpc_type::callback_install_request:
                case rpc_type::callback_result:
                case rpc_type::callback_result_w_bind:
                case rpc_type::func_request:
                default:
                    throw rpc_object_mismatch("Invalid rpc_object type detected");
            }
        }

        throw client_receive_error("Invalid RPC object received");
    }
};
} //namespace rpc_hpp
