#pragma once

#include "rpc.hpp"

namespace rpc_hpp
{
template<typename Serial>
class client_interface
{
public:
    using bytes_t = typename Serial::bytes_t;

    virtual ~client_interface() = default;

    virtual void send(bytes_t&& bytes) = 0;
    virtual bytes_t receive() = 0;

    template<typename... Args>
    rpc_response<Serial> call_func(std::string func_name, Args&&... args)
    {
        func_request<Args...> req{ std::move(func_name), std::forward<Args>(args)... };
        send(Serial::to_bytes(Serial::serialize(req)));

        const auto recv_loop = [this]
        {
            rpc_response<Serial> resp = Serial::from_bytes(receive());

            switch (resp.type())
            {
                case response_type::func_result:
                case response_type::func_error:
                    return resp;

                case response_type::callback_request:
#ifdef RPC_HPP_ENABLE_CALLBACKS
                {
                    rpc_response<Serial> resp2 = dispatch_callback(resp);
                    send(Serial::to_bytes(Serial::serialize(resp2)));
                    return recv_loop();
                }
#else
                    [[fallthrough]];
#endif
                default:
                    throw std::runtime_error("Invalid rpc_response detected");
            }
        };

        return recv_loop();
    }
};
} //namespace rpc_hpp
