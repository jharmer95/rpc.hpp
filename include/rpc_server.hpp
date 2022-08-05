#pragma once

#include "rpc.hpp"

#include <functional>
#include <string>
#include <unordered_map>

namespace rpc_hpp
{
template<typename Serial>
class server_base
{
public:
    using bytes_t = typename Serial::bytes_t;
    using response_t = rpc_object<Serial>;

    server_base() noexcept = default;

    server_base(const server_base&) = delete;
    server_base& operator=(const server_base&) = delete;

    server_base(server_base&&) noexcept = default;
    server_base& operator=(server_base&&) noexcept = default;

    template<typename R, typename... Args>
    void bind(std::string func_name, R (*func_ptr)(Args...))
    {
        m_dispatch_table.emplace(std::move(func_name),
            [func_ptr](response_t& response)
            {
                try
                {
                    exec_func(func_ptr, response);
                }
                catch (const rpc_exception& ex)
                {
                    response = func_error{ response.get_func_name(), ex };
                }
            });
    }

    template<typename R, typename... Args, typename F>
    void bind(std::string func_name, F&& func)
    {
        using fptr_t = R (*)(Args...);

        bind(std::move(func_name), fptr_t{ std::forward<F>(func) });
    }

    response_t handle_bytes(bytes_t&& bytes) const
    {
        if (auto o_request = response_t::parse_bytes(std::move(bytes)); o_request.has_value())
        {
            auto& request = o_request.value();
            const auto func_name = request.get_func_name();

            switch (request.get_type())
            {
                case rpc_object_type::func_request:
                    dispatch(request);
                    return request;

                case rpc_object_type::callback_install_request:
#ifdef RPC_HPP_ENABLE_CALLBACKS
                    request.is_callback_uninstall() ? uninstall_callback(request)
                                                    : install_callback(request);
                    return request;
#else
                    [[fallthrough]];
#endif

                case rpc_object_type::callback_result:
                case rpc_object_type::callback_result_w_bind:
                case rpc_object_type::callback_error:
#ifdef RPC_HPP_ENABLE_CALLBACKS
                    [[fallthrough]];
#else
                    return func_error{ "",
                        rpc_object_type_mismatch(
                            "Invalid rpc_object type detected (NOTE: callbacks are not "
                            "enabled on this server)") };
#endif

                default:
                    return func_error{ func_name,
                        rpc_object_type_mismatch("Invalid rpc_object type detected") };
            }
        }

        return func_error{ "", server_receive_error("Invalid RPC object received") };
    }

    void dispatch(response_t& rpc_obj) const
    {
        const auto func_name = rpc_obj.get_func_name();

        if (const auto it = m_dispatch_table.find(func_name); it != m_dispatch_table.cend())
        {
            it->second(rpc_obj);
        }

        rpc_obj = func_error{ func_name,
            function_not_found("RPC error: Called function: \"" + func_name + "\" not found") };
    }

protected:
    ~server_base() noexcept = default;

    template<typename R, typename... Args>
    static void exec_func(R (*func)(Args...), response_t& response)
    {
        auto args = response.template get_request<Args...>();
        auto func_name = response.get_func_name();

        if constexpr (std::is_void_v<R>)
        {
            try
            {
                std::apply(func, std::move(args));
                response = func_result<void>{ std::move(func_name) };
            }
            catch (const std::exception& ex)
            {
                throw remote_exec_error(ex.what());
            }
        }
        else
        {
            try
            {
                auto ret_val = std::apply(func, std::move(args));
                response = func_result<R>{ std::move(func_name), std::move(ret_val) };
            }
            catch (const std::exception& ex)
            {
                throw remote_exec_error(ex.what());
            }
        }
    }

private:
#ifdef RPC_HPP_ENABLE_CALLBACKS
    void install_callback(response_t& rpc_obj)
    {
        auto func_name = rpc_obj.get_func_name();

        auto [_, inserted] =
            m_installed_callbacks.try_emplace(std::move(func_name), rpc_obj.get_callback_id());

        if (!inserted)
        {
            // NOTE: since insertion did not occur, func_name was not moved so it is safe to use here
            rpc_obj = func_error{ func_name,
                callback_install_error("Callback: \"" + func_name + "\" is already installed") };
        }
    }

    void uninstall_callback(response_t& rpc_obj)
    {
        m_installed_callbacks.erase(rpc_obj.get_func_name());
    }
#endif

    std::unordered_map<std::string, std::function<void(response_t&)>> m_dispatch_table{};
#ifdef RPC_HPP_ENABLE_CALLBACKS
    std::unordered_map<std::string, uint64_t> m_installed_callbacks;
#endif
};
} //namespace rpc_hpp
