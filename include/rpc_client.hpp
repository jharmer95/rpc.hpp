#pragma once

#include "rpc.hpp"

namespace rpc_hpp
{
template<typename Serial>
class client_interface
{
public:
    using bytes_t = typename Serial::bytes_t;

    virtual ~client_interface() noexcept = default;
    client_interface() = default;
    client_interface(const client_interface&) = delete;
    client_interface& operator=(const client_interface&) = delete;
    client_interface& operator=(client_interface&&) = delete;

    template<typename... Args>
    rpc_object<Serial> call_func(std::string func_name, Args&&... args)
    {
        rpc_object<Serial> response =
            func_request<Args...>{ std::move(func_name), std::forward<Args>(args)... };

        send(response.to_bytes());

        response = recv_loop();
        assert(response.type() == rpc_object_type::func_result);
        return response;
    }

    template<typename R, typename... Args>
    callback_install_request install_callback(std::string func_name, R (*func)(Args...))
    {
        callback_install_request cb{ std::move(func_name), reinterpret_cast<size_t>(func) };
        rpc_object<Serial> request{ cb };
        send(request.to_bytes());
        return cb;
    }

    void uninstall_callback(callback_install_request&& callback)
    {
        callback.set_uninstall(true);
        rpc_object<Serial> request{ std::move(callback) };
        send(request.to_bytes());
    }

#ifdef RPC_HPP_ENABLE_CALLBACKS
    template<typename... Args>
    rpc_object<Serial> call_func_w_bind(std::string func_name, Args&&... args)
    {
        func_request<Args...> req{ bind_args_tag{}, std::move(func_name),
            std::forward<Args>(args)... };

        send(Serial::to_bytes(Serial::serialize(req)));

        auto response = recv_loop();
        assert(response.type() == rpc_object_type::func_result_w_bind);
        tuple_bind(response.template get_request<Args...>(), std::forward<Args>(args)...);
        return response;
    }
#endif

protected:
    client_interface(client_interface&&) noexcept = default;

    virtual void send(bytes_t&& bytes) = 0;
    virtual bytes_t receive() = 0;

private:
    rpc_object<Serial> recv_loop()
    {
        if (auto o_response = rpc_object<Serial>::parse_bytes(receive()); o_response.has_value())
        {
            auto& response = o_response.value();

            switch (response.type())
            {
                case rpc_object_type::func_result:
                case rpc_object_type::func_error:
                    return response;

                case rpc_object_type::callback_request:
#ifdef RPC_HPP_ENABLE_CALLBACKS
                {
                    rpc_object<Serial> resp2 = dispatch_callback(resp);
                    send(resp2.to_bytes());
                    return recv_loop();
                }
#else
                    [[fallthrough]];
#endif
                default:
                    throw rpc_object_type_mismatch("Invalid rpc_object type detected");
            }
        }

        throw client_receive_error("Invalid RPC object received");
    }
};
} //namespace rpc_hpp
