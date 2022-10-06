#ifndef RPC_HPP_DETAIL_EXEC_FUNC_HPP
#define RPC_HPP_DETAIL_EXEC_FUNC_HPP

#include "rpc_types.hpp"
#include "../rpc_object.hpp"

#include <utility>

namespace rpc_hpp::detail
{
template<bool IsCallback, typename Serial, typename R, typename... Args, typename F>
void exec_func(F&& func, rpc_object<Serial>& rpc_obj)
{
    auto args = rpc_obj.template get_args<Args...>();
    auto func_name = rpc_obj.get_func_name();
    const auto has_bound_args = rpc_obj.has_bound_args();

    try
    {
        if constexpr (std::is_void_v<R>)
        {
            std::apply(std::forward<F>(func), args);

            if (has_bound_args)
            {
                rpc_obj = rpc_object<Serial>{ rpc_result_w_bind<IsCallback, void, Args...>{
                    std::move(func_name), std::move(args) } };
            }
            else
            {
                rpc_obj =
                    rpc_object<Serial>{ rpc_result<IsCallback, void>{ std::move(func_name) } };
            }
        }
        else
        {
            auto ret_val = std::apply(std::forward<F>(func), args);

            if (has_bound_args)
            {
                rpc_obj = rpc_object<Serial>{ rpc_result_w_bind<IsCallback, R, Args...>{
                    std::move(func_name), std::move(ret_val), std::move(args) } };
            }
            else
            {
                rpc_obj = rpc_object<Serial>{ rpc_result<IsCallback, R>{
                    std::move(func_name), std::move(ret_val) } };
            }
        }
    }
    catch (const std::exception& ex)
    {
        throw remote_exec_error{ ex.what() };
    }
}
} //namespace rpc_hpp::detail

#endif
