#pragma once

#include "rpc_common.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#define call_header_func(FUNCNAME, ...) call_header_func_impl(FUNCNAME, #FUNCNAME, __VA_ARGS__)
#define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) inline RETURN (*FUNCNAME)(__VA_ARGS__) = nullptr

namespace rpc_hpp
{
namespace detail
{
    // Allows passing in string literals
    template<typename T>
    struct decay_str
    {
        static_assert(!std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<T>>>,
            "Pointer parameters are not allowed");

        static_assert(!std::is_array_v<std::remove_cv_t<std::remove_reference_t<T>>>,
            "C-style array parameters are not allowed");

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
    constexpr void tuple_bind(
        const std::tuple<std::remove_cv_t<std::remove_reference_t<decay_str_t<Args>>>...>& src,
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
        const std::tuple<std::remove_cv_t<std::remove_reference_t<decay_str_t<Args>>>...>& src,
        Args&&... dest)
    {
        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }
} //namespace detail

///@brief Class defining an interface for calling into an RPC server or module
///
///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
template<typename Serial>
class client_interface
{
public:
    virtual ~client_interface() noexcept = default;
    client_interface() noexcept = default;

    // Prevent copying
    client_interface(const client_interface&) = delete;
    client_interface& operator=(const client_interface&) = delete;

    // Prevent slicing
    client_interface& operator=(client_interface&&) = delete;

    ///@brief Sends an RPC call request to a server, waits for a response, then returns the result
    ///
    ///@tparam R Return type of the remote function to call
    ///@tparam Args Variadic argument type(s) of the remote function to call
    ///@param func_name Name of the remote function to call
    ///@param args Argument(s) for the remote function
    ///@return R Result of the function call, will throw with server's error message if the result does not exist
    ///@throws client_send_error Thrown if error occurs during the @ref send function
    ///@throws client_receive_error Thrown if error occurs during the @ref receive function
    ///@note nodiscard because an expensive remote procedure call is being performed
    template<typename R = void, typename... Args>
    [[nodiscard]] R call_func(std::string func_name, Args&&... args)
    {
        RPC_HPP_PRECONDITION(!func_name.empty());

        auto bytes = serialize_call<R, Args...>(std::move(func_name), std::forward<Args>(args)...);

        try
        {
            send(std::move(bytes));
        }
        catch (const std::exception& ex)
        {
            throw client_send_error(ex.what());
        }

        try
        {
            bytes = receive();
        }
        catch (const std::exception& ex)
        {
            throw client_receive_error(ex.what());
        }

        const auto pack = deserialize_call<R, Args...>(std::move(bytes));

        // Assign values back to any (non-const) reference members
        detail::tuple_bind(pack.get_args(), std::forward<Args>(args)...);
        return pack.get_result();
    }

    ///@brief Sends an RPC call request to a server, waits for a response, then returns the result
    ///
    ///@tparam R Return type of the remote function to call
    ///@tparam Args Variadic argument type(s) of the remote function to call
    ///@param func Pointer to the function definition to deduce signature
    ///@param func_name Name of the remote function to call
    ///@param args Argument(s) for the remote function
    ///@return R Result of the function call, will throw with server's error message if the result does not exist
    ///@note nodiscard because an expensive remote procedure call is being performed
    template<typename R, typename... Args>
    [[nodiscard]] R call_header_func_impl(
        [[maybe_unused]] R (*func)(Args...), std::string func_name, Args&&... args)
    {
        RPC_HPP_PRECONDITION(!func_name.empty());

        return call_func<R, Args...>(std::move(func_name), std::forward<Args>(args)...);
    }

protected:
    client_interface(client_interface&&) noexcept = default;

    ///@brief Sends serialized data to a server or module
    ///
    ///@param bytes Serialized data to be sent
    virtual void send(const typename Serial::bytes_t& bytes) = 0;

    ///@brief Receives serialized data from a server or module
    ///
    ///@return Serial::bytes_t Received serialized data
    virtual typename Serial::bytes_t receive() = 0;

private:
    template<typename R, typename... Args>
    static RPC_HPP_INLINE typename Serial::bytes_t serialize_call(
        std::string func_name, Args&&... args)
    {
        detail::packed_func<R, detail::decay_str_t<Args>...> pack = [&]() noexcept
        {
            if constexpr (std::is_void_v<R>)
            {
                return detail::packed_func<void, detail::decay_str_t<Args>...>{
                    std::move(func_name), std::forward_as_tuple(args...)
                };
            }
            else
            {
                return detail::packed_func<R, detail::decay_str_t<Args>...>{ std::move(func_name),
                    std::nullopt, std::forward_as_tuple(args...) };
            }
        }();

        auto serial_obj = [&pack]
        {
            try
            {
                return Serial::serialize_pack(pack);
            }
            catch (const rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw serialization_error(ex.what());
            }
        }();

        return Serial::to_bytes(std::move(serial_obj));
    }

    template<typename R, typename... Args>
    static RPC_HPP_INLINE auto deserialize_call(typename Serial::bytes_t&& bytes)
    {
        const auto ret_obj = Serial::from_bytes(std::move(bytes));

        if (!ret_obj.has_value())
        {
            throw client_receive_error("Client received invalid RPC object");
        }

        try
        {
            return Serial::template deserialize_pack<R, detail::decay_str_t<Args>...>(
                ret_obj.value());
        }
        catch (const rpc_exception&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw deserialization_error(ex.what());
        }
    }
};
} //namespace rpc_hpp