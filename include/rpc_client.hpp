#ifndef RPC_CLIENT_HPP
#define RPC_CLIENT_HPP

#include "impl/detail/exec_func.hpp"
#include "impl/detail/for_each.hpp"
#include "impl/detail/macros.hpp"
#include "impl/detail/rpc_types.hpp"
#include "impl/detail/traits.hpp"
#include "impl/rpc_exceptions.hpp"
#include "impl/rpc_object.hpp"

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

    template<typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_func(std::string func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<detail::decay_str_t<Args>...>{
            std::move(func_name), std::forward_as_tuple(args...) } };

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

    template<typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_func_w_bind(std::string func_name, Args&&... args)
    {
        auto response = object_t{ detail::func_request<detail::decay_str_t<Args>...>{
            std::move(func_name), std::forward_as_tuple(args...), true } };

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

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the rpc_object should be checked for its type")
    object_t call_header_func_impl(
        RPC_HPP_UNUSED const detail::fptr_t<R, Args...> func, std::string func_name, Args&&... args)
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
    bool has_callback(std::string_view func_name)
    {
        return m_callback_map.find(func_name) != m_callback_map.end();
    }

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    RPC_HPP_INLINE callback_install_request
        install_callback(std::string func_name, std::function<R(Args...)> func)
    {
        return install_callback_impl<R, Args...>(std::move(func_name), std::move(func));
    }

    template<typename R, typename... Args>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    RPC_HPP_INLINE callback_install_request
        install_callback(std::string func_name, const detail::fptr_t<R, Args...> func_ptr)
    {
        return install_callback_impl<R, Args...>(std::move(func_name), func_ptr);
    }

    template<typename R, typename... Args, typename F>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    RPC_HPP_INLINE callback_install_request install_callback(std::string func_name, F&& func)
    {
        return install_callback_impl<R, Args...>(
            std::move(func_name), std::function<R(Args...)>{ std::forward<F>(func) });
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
    virtual bytes_t receive() = 0;

private:
#if defined(RPC_HPP_ENABLE_CALLBACKS)
    template<typename R, typename... Args, typename F>
    RPC_HPP_NODISCARD("the returned callback_install_request is an input to uninstall_callback")
    callback_install_request install_callback_impl(std::string func_name, F&& func)
    {
        callback_install_request cb{ std::move(func_name) };

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
            function_not_found{ "RPC error: Called function: \"" + func_name + "\" not found" } } };
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
