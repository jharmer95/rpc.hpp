#ifndef RPC_SERVER_HPP
#define RPC_SERVER_HPP

#include "rpc.hpp"

#include <exception>
#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#if !defined(RPC_HEADER_FUNC)
#  define RPC_HEADER_FUNC(RT, FNAME, ...) RT FNAME(__VA_ARGS__)
#  define RPC_HEADER_FUNC_EXTC(RT, FNAME, ...) extern "C" RT FNAME(__VA_ARGS__)
#  define RPC_HEADER_FUNC_NOEXCEPT(RT, FNAME, ...) RT FNAME(__VA_ARGS__) noexcept
#endif

namespace rpc_hpp
{
// invariants:
//   1. m_dispatch_table cannot contain an empty key: ""
//   2. m_dispatch_table cannot contain an empty function value
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

    template<typename R, typename... Args, typename S>
    RPC_HPP_INLINE void bind(S&& func_name, std::function<R(Args...)> func)
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");
        RPC_HPP_PRECONDITION(!std::string_view{ func_name }.empty());
        RPC_HPP_PRECONDITION(static_cast<bool>(func));

        bind_impl<R, Args...>(std::forward<S>(func_name), std::move(func));
    }

    template<typename R, typename... Args, typename S>
    RPC_HPP_INLINE void bind(S&& func_name, const detail::fptr_t<R, Args...> func_ptr)
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");
        RPC_HPP_PRECONDITION(!std::string_view{ func_name }.empty());
        RPC_HPP_PRECONDITION(func_ptr != nullptr);

        bind_impl<R, Args...>(std::forward<S>(func_name), func_ptr);
    }

    template<typename R, typename... Args, typename S, typename F>
    RPC_HPP_INLINE void bind(S&& func_name, F&& func)
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");
        static_assert(std::is_invocable_v<F, Args...>, "F must be invocable with Args...");
        static_assert(
            std::is_convertible_v<std::invoke_result_t<F, Args...>, R>, "F must yield an R");

        RPC_HPP_PRECONDITION(!std::string_view{ func_name }.empty());

        bind_impl<R, Args...>(
            std::forward<S>(func_name), std::function<R(Args...)>{ std::forward<F>(func) });
    }

    void handle_bytes(bytes_t& bytes)
    {
        try
        {
            if (auto rpc_opt = object_t::parse_bytes(std::move(bytes)); rpc_opt.has_value())
            {
                auto& rpc_obj = rpc_opt.value();
                const auto func_name = rpc_obj.get_func_name();

                switch (rpc_obj.get_type())
                {
                    case rpc_type::func_request:
                        dispatch(rpc_obj);
                        bytes = rpc_obj.to_bytes();
                        return;

                    case rpc_type::callback_install_request:
                    case rpc_type::callback_result:
                    case rpc_type::callback_result_w_bind:
                    case rpc_type::callback_error:
                        handle_callback_object(rpc_obj);
                        bytes = rpc_obj.to_bytes();
                        return;

                    case rpc_type::callback_request:
                    case rpc_type::func_error:
                    case rpc_type::func_result:
                    case rpc_type::func_result_w_bind:
                    default:
                        bytes = object_t{
                            detail::func_error{ func_name,
                                object_mismatch_error{
                                    "RPC error: Invalid rpc_object type detected" } }
                        }.to_bytes();
                        return;
                }
            }

            bytes = object_t{
                detail::func_error{
                    "", server_receive_error{ "RPC error: Invalid RPC object received" } }
            }.to_bytes();
        }
        catch (const rpc_exception& ex)
        {
            bytes = object_t{ detail::func_error{ "", ex } }.to_bytes();
        }
        catch (const std::exception& ex)
        {
            bytes =
                object_t{ detail::func_error{ "", exception_type::none, ex.what() } }.to_bytes();
        }
    }

protected:
    server_interface(server_interface&&) noexcept = default;

    virtual void handle_callback_object(object_t& rpc_obj)
    {
        rpc_obj = object_t{ detail::func_error{ "",
            object_mismatch_error{
                "RPC error: Invalid rpc_object type detected (NOTE: callbacks are not "
                "enabled on this server)" } } };
    }

private:
    template<typename R, typename... Args, typename S, typename F>
    void bind_impl(S&& func_name, F&& func)
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");
        static_assert(std::is_invocable_v<F, Args...>, "F must be invocable with Args...");
        static_assert(
            std::is_convertible_v<std::invoke_result_t<F, Args...>, R>, "F must yield an R");

        RPC_HPP_PRECONDITION(!std::string_view{ func_name }.empty());

        auto [iter, status] = m_dispatch_table.try_emplace(std::forward<S>(func_name),
            [func = std::forward<F>(func)](object_t& rpc_obj)
            {
                RPC_HPP_PRECONDITION(rpc_obj.get_type() == rpc_type::func_request);

                try
                {
                    detail::exec_func<false, Serial, R, Args...>(func, rpc_obj);
                }
                catch (const rpc_exception& ex)
                {
                    rpc_obj = object_t{ detail::func_error{ rpc_obj.get_func_name(), ex } };
                }

                RPC_HPP_POSTCONDITION(rpc_obj.get_type() == rpc_type::func_result
                    || rpc_obj.get_type() == rpc_type::func_result_w_bind
                    || rpc_obj.get_type() == rpc_type::func_error);
            });

        if (!status)
        {
            // NOTE: `try_emplace` does not move func_name unless successful, so the use of it here is safe
            throw function_bind_error{ std::string{ "RPC error: server could not bind function: " }
                                           .append(std::forward<S>(func_name))
                                           .append("() successfully") };
        }

        RPC_HPP_POSTCONDITION(static_cast<bool>(iter->second));
    }

    void dispatch(object_t& rpc_obj) const
    {
        try
        {
            RPC_HPP_PRECONDITION(rpc_obj.get_type() == rpc_type::func_request);

            const auto func_name = rpc_obj.get_func_name();

            if (const auto find_it = m_dispatch_table.find(func_name);
                find_it != m_dispatch_table.cend())
            {
                find_it->second(rpc_obj);
            }
            else
            {
                rpc_obj = object_t{ detail::func_error{ func_name,
                    function_missing_error{
                        std::string{ "RPC error: Called function: " }.append(func_name).append(
                            "() not found") } } };
            }
        }
        catch (const rpc_exception& ex)
        {
            rpc_obj = object_t{ detail::func_error{ "", ex } };
        }
        catch (const std::exception& ex)
        {
            rpc_obj = object_t{ detail::func_error{ "", exception_type::none, ex.what() } };
        }
    }

    std::unordered_map<std::string, std::function<void(object_t&)>> m_dispatch_table{};
};

// invariants: (same as server_interface)
template<typename Serial>
class callback_server_interface : public server_interface<Serial>
{
public:
    using typename server_interface<Serial>::bytes_t;
    using typename server_interface<Serial>::object_t;
    using server_interface<Serial>::server_interface;

protected:
    template<typename R, typename... Args, typename S>
    [[nodiscard]] auto call_callback(S&& func_name, Args&&... args) -> R
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");
        RPC_HPP_PRECONDITION(!std::string_view{ func_name }.empty());

        const auto response =
            call_callback_impl(object_t{ detail::callback_request<detail::decay_str_t<Args>...>{
                std::forward<S>(func_name), std::forward_as_tuple(args...) } });

        RPC_HPP_POSTCONDITION(response.get_type() == rpc_type::callback_result
            || response.get_type() == rpc_type::callback_error);
        return response.template get_result<R>();
    }

    template<typename R, typename... Args, typename S>
    [[nodiscard]] auto call_callback_w_bind(S&& func_name, Args&&... args) -> R
    {
        static_assert(detail::is_stringlike_v<S>, "func_name must be a string-like type");
        RPC_HPP_PRECONDITION(!std::string_view{ func_name }.empty());

        const auto response =
            call_callback_impl(object_t{ detail::callback_request<detail::decay_str_t<Args>...>{
                std::forward<S>(func_name), std::forward_as_tuple(args...) } });

        detail::tuple_bind(response.template get_args<true, detail::decay_str_t<Args>...>(),
            std::forward<Args>(args)...);

        RPC_HPP_POSTCONDITION(response.get_type() == rpc_type::callback_result_w_bind
            || response.get_type() == rpc_type::callback_error);
        return response.template get_result<R>();
    }

private:
    virtual auto call_callback_impl(object_t&& request) -> object_t = 0;
    virtual void install_callback(object_t& rpc_obj) = 0;
    virtual void uninstall_callback(const object_t& rpc_obj) = 0;

    void handle_callback_object(object_t& rpc_obj) final
    try
    {
        if (rpc_obj.get_type() == rpc_type::callback_install_request)
        {
            rpc_obj.is_callback_uninstall() ? uninstall_callback(rpc_obj)
                                            : install_callback(rpc_obj);
        }
        else
        {
            server_interface<Serial>::handle_callback_object(rpc_obj);
        }
    }
    catch (const rpc_exception& ex)
    {
        rpc_obj = object_t{ detail::callback_error{ "", ex } };
    }
    catch (const std::exception& ex)
    {
        rpc_obj = object_t{ detail::callback_error{ "", exception_type::none, ex.what() } };
    }
};
} //namespace rpc_hpp
#endif
