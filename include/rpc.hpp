///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.1.0.0
///@date 08-25-2020
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#pragma once

#include <any>
#include <future>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

namespace rpc
{
/// @brief Default implementation for SFINAE struct
template<typename T>
struct function_traits;

/// @brief SFINAE struct to extract information from a function object
///
/// Can be used to get return and parameter types, as well as the count of parameters
/// @tparam R The return type of the function
/// @tparam Args The argument types of the function
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    static constexpr size_t nargs = sizeof...(Args);
    using result_type = R;
    using args_type = std::tuple<Args...>;

    template<size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, args_type>::type;
    };
};

template<typename R, typename... Args>
struct function_traits<R (*)(Args...)>
{
    static constexpr size_t nargs = sizeof...(Args);
    using result_type = R;
    using args_type = std::tuple<Args...>;

    template<size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, args_type>::type;
    };
};

/// @brief Helper variable for getting parameter count from @ref function_traits
///
/// @tparam R The return type of the function
/// @tparam Args The argument types of the function
template<typename R, typename... Args>
inline constexpr size_t function_param_count_v = function_traits<std::function<R(Args...)>>::nargs;

/// @brief Helper variable for getting the return type from @ref function_traits
///
/// @tparam R The return type of the function
/// @tparam Args The argument types of the function
template<typename R, typename... Args>
using function_result_t = typename function_traits<std::function<R(Args...)>>::type;

/// @brief Helper variable for getting the argument types from @ref function_traits
///
/// @tparam i The index of the argument to get the type of
/// @tparam R The return type of the function
/// @tparam Args The argument types of the function
template<size_t i, typename R, typename... Args>
using function_args_t = typename function_traits<std::function<R(Args...)>>::template arg<i>::type;

/// @brief Default implementation of meta-programming function
///
/// @tparam F Function type
/// @tparam Ts Tuple types (generic)
/// @tparam Is Index sequence to iterate over
/// @param tuple Tuple to iterate over
/// @param func Function to apply to each value
template<typename F, typename... Ts, size_t... Is>
void for_each_tuple(
    const std::tuple<Ts...>& tuple, const F& func, std::index_sequence<Is...> /*unused*/)
{
    using expander = int[];
    (void)expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

/// @brief Meta-programming function to apply a function over each member of a tuple
///
/// @tparam F Function type
/// @tparam Ts Tuple types (generic)
/// @param tuple Tuple to iterate over
/// @param func Function to apply to each value
template<typename F, typename... Ts>
void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func)
{
    for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

class packed_func_base
{
public:
    virtual ~packed_func_base() = default;
    packed_func_base() = delete;

    explicit packed_func_base(std::string&& func_name) : m_func_name(std::move(func_name)) {}

    packed_func_base(const packed_func_base&) = default;
    packed_func_base(packed_func_base&&) = default;

    [[nodiscard]] std::string get_func_name() const noexcept { return m_func_name; }
    [[nodiscard]] std::string get_err_mesg() const noexcept { return m_err_mesg; }
    void set_err_mesg(const std::string& mesg) noexcept { m_err_mesg = mesg; }

private:
    const std::string m_func_name;
    std::string m_err_mesg;
};

template<typename R, typename... Args>
class packed_func final : public packed_func_base
{
public:
    using result_type = R;

    packed_func() = delete;

    packed_func(std::string&& func_name, std::optional<result_type> result,
        std::array<std::any, sizeof...(Args)> args)
        : packed_func_base(std::move(func_name)), m_result(result), m_args(std::move(args))
    {
    }

    packed_func(const packed_func&) = default;
    packed_func(packed_func&&) = default;
    packed_func& operator=(const packed_func&) = default;
    packed_func& operator=(packed_func&&) = default;

    explicit operator bool() const noexcept { return m_result.has_value(); }

    [[nodiscard]] std::optional<R> get_result() const noexcept { return m_result; }

    void set_result(R value) noexcept { m_result = value; }

    void clear_result() noexcept { m_result = std::nullopt; }

    template<typename T>
    void set_arg(size_t arg_index, T value)
    {
        if (m_args[arg_index].type() != typeid(T))
        {
            // TODO: Print arg type name
            throw std::runtime_error("Invalid argument type provided!");
        }

        m_args[arg_index] = value;
    }

    template<typename T>
    [[nodiscard]] T get_arg(size_t arg_index) const
    {
        return std::any_cast<T>(m_args[arg_index]);
    }

private:
    std::optional<result_type> m_result{ std::nullopt };
    std::array<std::any, sizeof...(Args)> m_args;
};

template<typename... Args>
class packed_func<void, Args...> final : public packed_func_base
{
public:
    using result_type = void;

    packed_func() = delete;

    packed_func(std::string func_name, std::array<std::any, sizeof...(Args)> args)
        : packed_func_base(std::move(func_name)), m_args(std::move(args))
    {
    }

    packed_func(const packed_func&) = default;
    packed_func(packed_func&&) = default;
    packed_func& operator=(const packed_func&) = default;
    packed_func& operator=(packed_func&&) = default;

    explicit operator bool() const noexcept { return true; }

    template<typename T>
    void set_arg(size_t arg_index, T value)
    {
        if (m_args[arg_index].type() != typeid(T))
        {
            // TODO: Print arg type name
            throw std::runtime_error("Invalid argument type provided!");
        }

        m_args[arg_index] = value;
    }

    template<typename T>
    [[nodiscard]] T get_arg(size_t arg_index) const
    {
        return std::any_cast<T>(m_args[arg_index]);
    }

private:
    std::array<std::any, sizeof...(Args)> m_args;
};

template<typename T>
struct packed_func_traits;

template<typename R, typename... Args>
struct packed_func_traits<R (*)(Args...)>
{
    using packed_func_type = packed_func<R, Args...>;
    using result_type = R;
};

template<typename R, typename... Args>
struct packed_func_traits<std::function<R(Args...)>>
{
    using packed_func_type = packed_func<R, Args...>;
    using result_type = R;
};

template<typename R, typename... Args>
packed_func<R, Args...>& convert_func(
    std::function<R(Args...)> /*unused*/, const packed_func_base& pack)
{
    return dynamic_cast<packed_func<R, Args...>&>(pack);
}

template<typename R, typename... Args>
packed_func<R, Args...>& convert_func(R (*/*unused*/)(Args...), const packed_func_base& pack)
{
    return dynamic_cast<packed_func<R, Args...>&>(pack);
}

// NOTE: Member functions are to be implemented by an adapter
template<typename Serial>
class serial_adapter
{
public:
    template<typename R, typename... Args>
    [[nodiscard]] static packed_func<R, Args...> to_packed_func(const Serial& serial_obj);

    template<typename R, typename... Args>
    [[nodiscard]] static Serial from_packed_func(const packed_func<R, Args...>& pack);

    template<typename T>
    [[nodiscard]] static T pack_arg(const Serial& obj, unsigned& i);

    [[nodiscard]] static std::string to_string(const Serial& serial_obj);

    [[nodiscard]] static Serial from_string(const std::string& str);

    [[nodiscard]] static std::string extract_func_name(const Serial& obj);
};

namespace details
{
    template<typename Value, typename R, typename... Args>
    Value decode_argument(const packed_func<R, Args...>& pack, unsigned& arg_index)
    {
        // TODO: Address cases where pointer, container, or custom type is used
        return pack.template get_arg<Value>(arg_index++);
    }

    template<typename R, typename... Args>
    packed_func<R, Args...> pack_call(const std::string& func_name, Args&&... args)
    {
        // TODO: Address cases where pointer, container, or custom type is used?
        std::array<std::any, sizeof...(Args)> argArray{ std::forward<Args>(args)... };

        if constexpr (std::is_void_v<R>)
        {
            return packed_func<void, Args...>(func_name, argArray);
        }
        else
        {
            return packed_func<R, Args...>(func_name, std::nullopt, argArray);
        }
    }
} // namespace details

namespace server
{
    // TODO: Server-side asynchronous functions (will probably have to return vs. reference)

    template<typename Serial, typename R, typename... Args>
    packed_func<R, Args...> create_func(R (*/*unused*/)(Args...), const Serial& obj)
    {
        return serial_adapter<Serial>::template to_packed_func<R, Args...>(obj);
    }

    template<typename R, typename... Args>
    void run_callback(std::function<R(Args...)> func, packed_func<R, Args...>& pack)
    {
        unsigned arg_count = 0;
        std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
            details::decode_argument<std::remove_cv_t<std::remove_reference_t<Args>>, R, Args...>(
                pack, arg_count)...
        };

        if constexpr (std::is_void_v<R>)
        {
            std::apply(func, args);
        }
        else
        {
            auto result = std::apply(func, args);
            pack.set_result(result);
        }
    }

    template<typename R, typename... Args>
    void run_callback(R (*func)(Args...), packed_func<R, Args...>& fc)
    {
        return run_callback(std::function<R(Args...)>(func), fc);
    }

    // NOTE: dispatch is to be implemented server-side
    template<typename Serial>
    extern void dispatch(Serial& serial_obj);
}

inline namespace client
{
    template<typename Serial, typename R = void, typename... Args>
    Serial serialize_call(const std::string& func_name, Args&&... args)
    {
        auto packed = details::pack_call<R, Args...>(func_name, std::forward<Args>(args)...);
        return serial_adapter<Serial>::template from_packed_func<R, Args...>(packed);
    }

    template<typename Serial, typename R = void, typename... Args>
    std::future<Serial> async_serialize_call(const std::string& func_name, Args&&... args)
    {
        auto packed = details::pack_call<R, Args...>(func_name, std::forward<Args>(args)...);
        return std::async(serial_adapter<Serial>::template from_packed_func<R, Args...>, packed);
    }

    // NOTE: send_to_server to be implemented client-side
    template<typename Serial, typename Client>
    extern void send_to_server(const Serial& serial_obj, Client& client);

    // NOTE: get_server_response to be implemented client-side
    template<typename Serial, typename Client>
    extern Serial get_server_response(Client& client);

    template<typename Serial, typename Client, typename R, typename... Args>
    packed_func<R, Args...> call(Client& client, const std::string& func_name, Args&&... args)
    {
        const auto serial_obj =
            serialize_call<Serial, R, Args...>(func_name, std::forward<Args>(args)...);

        send_to_server(serial_obj, client);
        const auto resp_obj = get_server_response<Serial>(client);
        return serial_adapter<Serial>::template to_packed_func<R, Args...>(resp_obj);
    }
} // namespace client
} // namespace rpc
