#pragma once

#include "rpc.hpp"

#define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) inline RETURN (*FUNCNAME)(__VA_ARGS__) = nullptr
#define call_header_func(FUNCNAME, ...) call_header_func_impl(FUNCNAME, #FUNCNAME, __VA_ARGS__)

namespace rpc_hpp
{
namespace detail
{
    template<typename T>
    struct decay_str
    {
        static_assert(!std::is_pointer_v<remove_cvref_t<T>>, "Pointer parameters are not allowed");

        static_assert(
            !std::is_array_v<remove_cvref_t<T>>, "C-style array parameters are not allowed");

        using type = T;
    };

    template<>
    struct decay_str<const char*>
    {
        using type = const std::string&;
    };

    template<>
    struct decay_str<const char*&>
    {
        using type = const std::string&;
    };

    template<>
    struct decay_str<const char* const&>
    {
        using type = const std::string&;
    };

    template<size_t N>
    struct decay_str<const char (&)[N]>
    {
        using type = const std::string&;
    };

    template<typename T>
    using decay_str_t = typename decay_str<T>::type;

    template<typename... Args, size_t... Is>
    constexpr void tuple_bind(const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src,
        std::index_sequence<Is...>, Args&&... dest)
    {
        using expander = int[];
        std::ignore = expander{ 0,
            (
                (void)[](auto&& x, auto&& y) {
                    if constexpr (
                        std::is_reference_v<
                            decltype(x)> && !std::is_const_v<std::remove_reference_t<decltype(x)>> && !std::is_pointer_v<std::remove_reference_t<decltype(x)>>)
                    {
                        x = std::forward<decltype(y)>(y);
                    }
                }(dest, std::get<Is>(src)),
                0)... };
    }

    template<typename... Args>
    constexpr void tuple_bind(
        const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src, Args&&... dest)
    {
        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }
} //namespace detail

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

    // nodiscard because the rpc_object should be checked for its type
    template<typename... Args>
    [[nodiscard]] rpc_object<Serial> call_func(std::string func_name, Args&&... args)
    {
        auto response = rpc_object<Serial>{ func_request<detail::decay_str_t<Args>...>{
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
    [[nodiscard]] rpc_object<Serial> call_func_w_bind(std::string func_name, Args&&... args)
    {
        auto response = rpc_object<Serial>{ func_request<detail::decay_str_t<Args>...>{
            bind_args_tag{}, std::move(func_name), std::forward_as_tuple(args...) } };

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
    [[nodiscard]] rpc_object<Serial> call_header_func_impl(
        [[maybe_unused]] R (*func)(Args...), std::string func_name, Args&&... args)
    {
        return call_func_w_bind<Args...>(std::move(func_name), std::forward<Args>(args)...);
    }

    template<typename R, typename... Args>
    callback_install_request install_callback(std::string func_name, R (*func)(Args...))
    {
        callback_install_request cb{ std::move(func_name), reinterpret_cast<size_t>(func) };
        rpc_object<Serial> request{ cb };

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
        bytes_t bytes;

        try
        {
            bytes = receive();
        }
        catch (const std::exception& ex)
        {
            throw client_receive_error(ex.what());
        }

        if (auto o_response = rpc_object<Serial>::parse_bytes(std::move(bytes));
            o_response.has_value())
        {
            auto& response = o_response.value();

            switch (response.type())
            {
                case rpc_object_type::func_result:
                case rpc_object_type::func_result_w_bind:
                case rpc_object_type::func_error:
                    return response;

                case rpc_object_type::callback_request:
#ifdef RPC_HPP_ENABLE_CALLBACKS
                {
                    rpc_object<Serial> resp2 = dispatch_callback(resp);

                    try
                    {
                        send(resp2.to_bytes());
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

                default:
                    throw rpc_object_type_mismatch("Invalid rpc_object type detected");
            }
        }

        throw client_receive_error("Invalid RPC object received");
    }
};
} //namespace rpc_hpp
