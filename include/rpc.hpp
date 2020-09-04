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

/// @brief Default implementation for SFINAE struct
template<typename, typename T>
struct is_serializable_base
{
    static_assert(std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type");
};

/// @brief SFINAE struct checking a type for a 'serialize' member function
///
/// Checks whether the given type \c C has a member function to serialize it to the type given by \c R
/// @tparam C The type to check for 'serialize'
/// @tparam R The type to be serialized to
/// @tparam Args The types of arguments (generic)
template<typename C, typename R, typename... Args>
struct is_serializable_base<C, R(Args...)>
{
private:
    template<typename T>
    static constexpr auto check(T*) noexcept ->
        typename std::is_same<decltype(std::declval<T>().serialize(std::declval<Args>()...)),
            R>::type;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

/// @brief Default implementation for SFINAE struct
template<typename, typename T>
struct is_deserializable_base
{
    static_assert(std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type");
};

/// @brief SFINAE struct checking a type for a 'deserialize' member function
///
/// Checks whether the given type \c C has a member function to de-serialize it to the type given by \c R
/// @tparam C The type to check for 'deserialize'
/// @tparam R The type to be de-serialized to
/// @tparam Args They types of arguments (generic)
template<typename C, typename R, typename... Args>
struct is_deserializable_base<C, R(Args...)>
{
private:
    template<typename T>
    static constexpr auto check(T*) noexcept ->
        typename std::is_same<decltype(std::declval<T>().deserialize(std::declval<Args>()...)),
            R>::type;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

/// @brief SFINAE struct combining the logic of @ref is_serializable_base and @ref is_deserializable_base
///
/// Checks whether the given type \c Value can be serialized to and de-serialized from the serial type \c Serial
/// @tparam Serial The serial object type
/// @tparam Value The type of object to serialize/de-serialize
template<typename Serial, typename Value>
struct is_serializable : std::integral_constant<bool,
                             is_serializable_base<Value, Serial(const Value&)>::value
                                 && is_deserializable_base<Value, Value(const Serial&)>::value>
{
};

/// @brief Helper variable for @ref is_serializable
///
/// @tparam Serial The serial object type
/// @tparam Value The type of object to serialize/de-serialize
template<typename Serial, typename Value>
inline constexpr bool is_serializable_v = is_serializable<Serial, Value>::value;

/// @brief SFINAE struct for checking a type for a 'begin' member function
///
/// Checks whether the given type \c C has a function 'begin' that returns an iterator type
/// @tparam C Type to check for 'begin'
template<typename C>
struct has_begin
{
private:
    template<typename T>
    static constexpr auto check(T*) noexcept ->
        typename std::is_same<decltype(std::declval<T>().begin()), typename T::iterator>::type;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

/// @brief SFINAE struct for checking a type for a 'end' member function
///
/// Checks whether the given type \c C has a function 'end' that returns an iterator type
/// @tparam C Type to check for 'end'
template<typename C>
struct has_end
{
private:
    template<typename T>
    static constexpr auto check(T*) noexcept ->
        typename std::is_same<decltype(std::declval<T>().end()), typename T::iterator>::type;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

/// @brief SFINAE struct for checking a type for a 'size' member function
///
/// Checks whether the given type \c C has a function 'size' that returns a size type
/// @tparam C Type to check for 'size'
template<typename C>
struct has_size
{
private:
    template<typename T>
    static constexpr auto check(T*) noexcept ->
        typename std::is_same<decltype(std::declval<T>().size()), size_t>::type;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

/// @brief SFINAE struct to determine if a type is a container
///
/// Combines the logic of @ref has_size, @ref has_begin, and @ref has_end to determine if a given type \c C
/// is compatible with STL containers
/// @tparam C Type to check
template<typename C>
struct is_container
    : std::integral_constant<bool, has_size<C>::value && has_begin<C>::value && has_end<C>::value>
{
};

/// @brief Helper variable for @ref is_container
///
/// @tparam C Type to check
template<typename C>
inline constexpr bool is_container_v = is_container<C>::value;

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

    packed_func(std::string func_name, std::optional<result_type> result,
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
            const std::string t_name = typeid(T).name();
            throw std::runtime_error("Invalid argument type: \"" + t_name + "\" provided!");
        }

        m_args[arg_index] = value;
    }

    template<typename T>
    [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> get_arg(size_t arg_index) const
    {
        return std::any_cast<std::remove_cv_t<std::remove_reference_t<T>>>(m_args[arg_index]);
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
            const std::string t_name = typeid(T).name();
            throw std::runtime_error("Invalid argument type: \"" + t_name + "\" provided!");
        }

        m_args[arg_index] = value;
    }

    template<typename T>
    [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> get_arg(size_t arg_index) const
    {
        return std::any_cast<std::remove_cv_t<std::remove_reference_t<T>>>(m_args[arg_index]);
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
    [[nodiscard]] static Serial make_sub_object(const Serial& obj, unsigned index);
    [[nodiscard]] static Serial make_sub_object(const Serial& obj, const std::string& name);

    template<typename T>
    [[nodiscard]] static T get_value(const Serial& obj);

    template<typename Container>
    static void populate_array(const Serial& obj, Container& container);
};

template<typename Serial, typename Value>
[[nodiscard]] Serial serialize(const Value& val);

template<typename Serial, typename Value>
[[nodiscard]] Value deserialize(const Serial& serial_obj);

namespace details
{
    template<typename Value, typename R, typename... Args>
    std::remove_cv_t<std::remove_reference_t<Value>> args_from_packed(
        const packed_func<R, Args...>& pack, unsigned& arg_index)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<Value>>;

        if constexpr (std::is_pointer_v<no_ref_t>)
        {
#if defined(RPC_HPP_ENABLE_POINTERS)
            // TODO: Implement pointer
#else
            static_assert(false,
                "Passing pointers across the RPC interface is not recommended. Please consider "
                "refactoring your RPC calls or define RPC_HPP_ENABLE_POINTERS to ignore this "
                "error.");
#endif
        }
        else
        {
            return pack.template get_arg<no_ref_t>(arg_index++);
        }
    }

    template<typename Serial, typename Value>
    std::remove_cv_t<std::remove_reference_t<Value>> arg_from_serial(const Serial& obj)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<Value>>;

        if constexpr (std::is_pointer_v<no_ref_t>)
        {
#if defined(RPC_HPP_ENABLE_POINTERS)
            // TODO: Implement pointer
#else
            static_assert(false,
                "Passing pointers across the RPC interface is not recommended. Please consider "
                "refactoring your RPC calls or define RPC_HPP_ENABLE_POINTERS to ignore this "
                "error.");
#endif
        }
        else if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
        {
            return serial_adapter<Serial>::template get_value<no_ref_t>(obj);
        }
        else if constexpr (is_serializable_v<Serial, no_ref_t>)
        {
            // Call custom deserialize member function
            return no_ref_t::deserialize(obj);
        }
        else if constexpr (rpc::is_container_v<no_ref_t>)
        {
            no_ref_t container;
            serial_adapter<Serial>::populate_array(obj, container);
            return container;
        }
        else
        {
            // Attempt to find overloaded rpc::deserialize function
            return deserialize<Serial, no_ref_t>(obj);
        }
    }

    template<typename Serial, typename Value>
    std::remove_cv_t<std::remove_reference_t<Value>> args_from_serial(
        const Serial& obj, unsigned& arg_index)
    {
        const auto args = serial_adapter<Serial>::make_sub_object(obj, "args");
        const auto sub_obj = serial_adapter<Serial>::make_sub_object(args, arg_index++);
        return arg_from_serial<Serial, Value>(sub_obj);
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
            details::args_from_packed<std::remove_cv_t<std::remove_reference_t<Args>>, R, Args...>(
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
