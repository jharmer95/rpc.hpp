#pragma once

#include "rpc.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) extern RETURN FUNCNAME(__VA_ARGS__)

namespace rpc_hpp
{
template<typename Serial>
class server_interface
{
public:
    using bytes_t = typename Serial::bytes_t;
    using object_t = rpc_object<Serial>;

    virtual ~server_interface() noexcept = default;
    server_interface() noexcept = default;

    server_interface(const server_interface&) = delete;
    server_interface& operator=(const server_interface&) = delete;
    server_interface& operator=(server_interface&&) noexcept = delete;

    virtual void send(bytes_t&& bytes) = 0;
    virtual bytes_t receive() = 0;

    template<typename R, typename... Args>
    void bind(std::string func_name, const std::function<R(Args...)>& func)
    {
        m_dispatch_table.try_emplace(std::move(func_name),
            [&func](object_t& rpc_obj)
            {
                try
                {
                    detail::exec_func<false, Serial, R, Args...>(func, rpc_obj);
                }
                catch (const rpc_exception& ex)
                {
                    rpc_obj = object_t{ detail::func_error<false>{ rpc_obj.get_func_name(), ex } };
                }
            });
    }

    template<typename R, typename... Args>
    void bind(std::string func_name, std::function<R(Args...)>&& func)
    {
        m_dispatch_table.try_emplace(std::move(func_name),
            [func = std::move(func)](object_t& rpc_obj)
            {
                try
                {
                    detail::exec_func<false, Serial, R, Args...>(std::move(func), rpc_obj);
                }
                catch (const rpc_exception& ex)
                {
                    rpc_obj = object_t{ detail::func_error<false>{ rpc_obj.get_func_name(), ex } };
                }
            });
    }

    template<typename R, typename... Args>
    void bind(std::string func_name, R (*func_ptr)(Args...))
    {
        m_dispatch_table.try_emplace(std::move(func_name),
            [func_ptr](object_t& rpc_obj)
            {
                try
                {
                    detail::exec_func<false, Serial, R, Args...>(func_ptr, rpc_obj);
                }
                catch (const rpc_exception& ex)
                {
                    rpc_obj = object_t{ detail::func_error<false>{ rpc_obj.get_func_name(), ex } };
                }
            });
    }

    template<typename R, typename... Args, typename F>
    void bind(std::string func_name, F&& func)
    {
        bind<R, Args...>(std::move(func_name), std::function<R(Args...)>{ std::forward<F>(func) });
    }

    RPC_HPP_NODISCARD("the bytes are consumed by this function")
    object_t handle_bytes(bytes_t&& bytes)
    {
        if (auto rpc_opt = object_t::parse_bytes(std::move(bytes)); rpc_opt.has_value())
        {
            auto& rpc_obj = rpc_opt.value();
            const auto func_name = rpc_obj.get_func_name();

            switch (rpc_obj.type())
            {
                case rpc_type::func_request:
                    dispatch(rpc_obj);
                    return rpc_obj;

                case rpc_type::callback_install_request:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
                    rpc_obj.is_callback_uninstall() ? uninstall_callback(rpc_obj)
                                                    : install_callback(rpc_obj);

                    return rpc_obj;

#else
                    [[fallthrough]];
#endif

                case rpc_type::callback_result:
                case rpc_type::callback_result_w_bind:
                case rpc_type::callback_error:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
                    [[fallthrough]];
#else
                    return object_t{ detail::func_error<false>{ "",
                        rpc_object_mismatch(
                            "Invalid rpc_object type detected (NOTE: callbacks are not "
                            "enabled on this server)") } };
#endif

                case rpc_type::callback_request:
                case rpc_type::func_error:
                case rpc_type::func_result:
                case rpc_type::func_result_w_bind:
                default:
                    return object_t{ detail::func_error<false>{
                        func_name, rpc_object_mismatch("Invalid rpc_object type detected") } };
            }
        }

        return object_t{ detail::func_error<false>{
            "", server_receive_error("Invalid RPC object received") } };
    }

    void dispatch(object_t& rpc_obj) const
    {
        const auto func_name = rpc_obj.get_func_name();

        if (const auto it = m_dispatch_table.find(func_name); it != m_dispatch_table.cend())
        {
            it->second(rpc_obj);
            return;
        }

        rpc_obj = object_t{ detail::func_error<false>{ func_name,
            function_not_found("RPC error: Called function: \"" + func_name + "\" not found") } };
    }

protected:
    server_interface(server_interface&&) noexcept = default;

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    template<typename R, typename... Args>
    [[nodiscard]] R call_callback(const std::string& func_name, Args&&... args)
    {
        if (const auto it = m_installed_callbacks.find(func_name);
            it != m_installed_callbacks.cend())
        {
            object_t response = object_t{ detail::callback_request<detail::decay_str_t<Args>...>{
                func_name, std::forward_as_tuple(args...) } };

            try
            {
                send(std::move(response).to_bytes());
            }
            catch (const std::exception& ex)
            {
                throw server_send_error(ex.what());
            }

            response = recv_impl();
            return response.template get_result<R, true>();
        }

        throw callback_missing_error("Callback" + func_name + "was called but not installed");
    }

    template<typename R, typename... Args>
    [[nodiscard]] R call_callback_w_bind(const std::string& func_name, Args&&... args)
    {
        if (const auto it = m_installed_callbacks.find(func_name);
            it != m_installed_callbacks.cend())
        {
            object_t response = object_t{ detail::callback_request<detail::decay_str_t<Args>...>{
                detail::bind_args_tag{}, func_name, std::forward_as_tuple(args...) } };

            try
            {
                send(response.to_bytes());
            }
            catch (const std::exception& ex)
            {
                throw server_send_error(ex.what());
            }

            response = recv_impl();
            detail::tuple_bind(response.template get_args<true, detail::decay_str_t<Args>...>(),
                std::forward<Args>(args)...);

            return response.template get_result<R, true>();
        }

        throw callback_missing_error("Callback" + func_name + "was called but not installed");
    }
#endif

private:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    [[nodiscard]] object_t recv_impl()
    {
        bytes_t bytes = [this]
        {
            try
            {
                return receive();
            }
            catch (const std::exception& ex)
            {
                throw server_receive_error(ex.what());
            }
        }();

        auto o_response = object_t::parse_bytes(std::move(bytes));

        if (!o_response.has_value())
        {
            throw server_receive_error("Invalid RPC object received");
        }

        switch (auto& response = o_response.value(); response.type())
        {
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
            case rpc_type::callback_error:
                return response;

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::func_error:
            case rpc_type::func_request:
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
            default:
                throw rpc_object_mismatch("Invalid rpc_object type detected");
        }
    }

    void install_callback(object_t& rpc_obj)
    {
        auto func_name = rpc_obj.get_func_name();

        auto [_, inserted] = m_installed_callbacks.insert(std::move(func_name));

        if (!inserted)
        {
            // NOTE: since insertion did not occur, func_name was not moved so it is safe to use here
            rpc_obj = object_t{ detail::callback_error{ func_name,
                callback_install_error("Callback: \"" + func_name + "\" is already installed") } };
        }
    }

    void uninstall_callback(const object_t& rpc_obj)
    {
        m_installed_callbacks.erase(rpc_obj.get_func_name());
    }
#endif

    std::unordered_map<std::string, std::function<void(object_t&)>> m_dispatch_table{};

#if defined(RPC_HPP_ENABLE_CALLBACKS)
    std::unordered_set<std::string> m_installed_callbacks{};
#endif
};
} //namespace rpc_hpp
