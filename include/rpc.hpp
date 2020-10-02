///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.2.0
///@date 10-02-2020
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
///@brief Namespace for functions/variables that should be used only from within the library.
/// Using anything in this namespace in your project is discouraged
namespace details
{
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
    struct is_container : std::integral_constant<bool,
                              has_size<C>::value && has_begin<C>::value && has_end<C>::value>
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
} // namespace rpc::details

///@brief Polymorphic base class for \ref packed_func
class packed_func_base
{
public:
    virtual ~packed_func_base() = default;
    packed_func_base() = delete;

    ///@brief Construct a new packed_func_base object
    ///
    ///@param func_name Name of the function (case-sensitive)
    explicit packed_func_base(std::string&& func_name) : m_func_name(std::move(func_name)) {}

    packed_func_base(const packed_func_base&) = default;
    packed_func_base(packed_func_base&&) = default;

    ///@brief Get the function name
    ///
    ///@return std::string Name of the stored function (case-sensitive)
    [[nodiscard]] std::string get_func_name() const noexcept { return m_func_name; }

    ///@brief Get the error message
    ///
    ///@return std::string The error message (if any)
    [[nodiscard]] std::string get_err_mesg() const noexcept { return m_err_mesg; }

    ///@brief Set the error message
    ///
    ///@param mesg String to set as the error message
    void set_err_mesg(const std::string& mesg) noexcept { m_err_mesg = mesg; }

private:
    const std::string m_func_name;
    std::string m_err_mesg{};
};

///@brief Class reprensenting a function call including its name, result, and parameters
///
///@tparam R The return type
///@tparam Args The list of parameter type(s)
template<typename R, typename... Args>
class packed_func final : public packed_func_base
{
public:
    ///@brief The type of the packed_func's result
    using result_type = R;

    packed_func() = delete;

    ///@brief Construct a new packed_func object
    ///
    ///@param func_name Name of the function (case-sensitive)
    ///@param result Function call result (if no result yet, use std::nullopt)
    ///@param args List of parameters for the function call
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

    ///@brief Get the result of the function call (if it exists)
    ///
    ///@return std::optional<R> If the function has not been called, or resulted in an error: std::nullopt, else: the result of the function call
    [[nodiscard]] std::optional<R> get_result() const noexcept { return m_result; }

    ///@brief Set the result
    ///
    ///@param value Value of type R to set as the result
    void set_result(R value) noexcept { m_result = value; }

    ///@brief Sets the result back to null
    void clear_result() noexcept { m_result = std::nullopt; }

    ///@brief Set a particular argument
    ///
    ///@tparam T Type of the argument to be set
    ///@param arg_index The index (0 start) of the argument to change
    ///@param value The value to set the argument to
    template<typename T>
    void set_arg(size_t arg_index, T value)
    {
        if (arg_index > m_args.size())
        {
            throw std::logic_error("Index out of bounds for argument list: "
                + std::to_string(arg_index) + " > " + std::to_string(m_args.size()) + "!");
        }

        if (m_args[arg_index].type() != typeid(T))
        {
            const std::string t_name = typeid(T).name();
            throw std::runtime_error("Invalid argument type: \"" + t_name + "\" provided!");
        }

        m_args[arg_index] = value;
    }

    ///@brief Set all arguments
    ///
    ///@param args A tuple containing the list of arguments to set
    void set_args(const std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>& args)
    {
        size_t i = 0;
        details::for_each_tuple(args, [&i, this](auto x) { m_args[i++] = x; });
    }

    ///@brief Get the arg object
    ///
    ///@tparam T Type to be acquired from the argument list (via std::any_cast)
    ///@param arg_index The index (0 start) of the argument to retrieve
    ///@return std::remove_cv_t<std::remove_reference_t<T>> The argument's value
    template<typename T>
    [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> get_arg(size_t arg_index) const
    {
        // TODO: Remove the remove_cv/remove_reference?
        return std::any_cast<std::remove_cv_t<std::remove_reference_t<T>>>(m_args[arg_index]);
    }

private:
    std::optional<result_type> m_result{ std::nullopt };
    std::array<std::any, sizeof...(Args)> m_args;
};

///@brief Class reprensenting a function call (with void result) including its name and parameters
///
///@tparam Args The list of parameter type(s)
template<typename... Args>
class packed_func<void, Args...> final : public packed_func_base
{
public:
    ///@brief The type of the result (void)
    using result_type = void;

    packed_func() = delete;

    ///@brief Construct a new packed_func object
    ///
    ///@param func_name Name of the function (case-sensitive)
    ///@param args List of parameters for the function call
    packed_func(std::string func_name, std::array<std::any, sizeof...(Args)> args)
        : packed_func_base(std::move(func_name)), m_args(std::move(args))
    {
    }

    packed_func(const packed_func&) = default;
    packed_func(packed_func&&) = default;
    packed_func& operator=(const packed_func&) = default;
    packed_func& operator=(packed_func&&) = default;

    explicit operator bool() const noexcept { return true; }

    ///@brief Set a particular argument
    ///
    ///@tparam T Type of the argument to be set
    ///@param arg_index The index (0 start) of the argument to change
    ///@param value The value to set the argument to
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

    ///@brief Set all arguments
    ///
    ///@param args A tuple containing the list of arguments to set
    void set_args(const std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>& args)
    {
        size_t i = 0;
        details::for_each_tuple(args, [&i, this](auto x) { m_args[i++] = x; });
    }

    ///@brief Get the arg object
    ///
    ///@tparam T Type to be acquired from the argument list (via std::any_cast)
    ///@param arg_index The index (0 start) of the argument to retrieve
    ///@return std::remove_cv_t<std::remove_reference_t<T>> The argument's value
    template<typename T>
    [[nodiscard]] std::remove_cv_t<std::remove_reference_t<T>> get_arg(size_t arg_index) const
    {
        // TODO: Remove the remove_cv/remove_reference?
        return std::any_cast<std::remove_cv_t<std::remove_reference_t<T>>>(m_args[arg_index]);
    }

private:
    std::array<std::any, sizeof...(Args)> m_args;
};

///@brief Template class that provides an interface for going to and from a \ref packed_func and a serial object
///
///@note Member functions of serial_adapter are to be implemented by an adapter
///@tparam Serial The serial object type
template<typename Serial>
class serial_adapter
{
public:
    ///@brief Converts a serial object into a specific \ref packed_func
    ///
    ///@tparam R The result type for the \ref packed_func
    ///@tparam Args The list of paramter type(s) for the \ref packed_func
    ///@param serial_obj The serial object to be converted
    ///@return packed_func<R, Args...> The packaged function call
    template<typename R, typename... Args>
    [[nodiscard]] static packed_func<R, Args...> to_packed_func(const Serial& serial_obj);

    ///@brief Converts a \ref packed_func into a serial object
    ///
    ///@tparam R The result type for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param pack The packaged function call to be converted
    ///@return Serial The serial object
    template<typename R, typename... Args>
    [[nodiscard]] static Serial from_packed_func(const packed_func<R, Args...>& pack);

    ///@brief Converts a serial object to a readable std::string
    ///
    ///@param serial_obj The serial object to be converted
    ///@return std::string The string representation of the serial object
    [[nodiscard]] static std::string to_string(const Serial& serial_obj);

    ///@brief Parses a std::string into a serial object
    ///
    ///@param str The string to be parsed
    ///@return Serial The serial object represented by the string
    [[nodiscard]] static Serial from_string(const std::string& str);

    ///@brief Retrieve the function name from a serial object
    ///
    ///@param obj The serial object to retrieve the function name from
    ///@return std::string The function name (case-sensitive)
    [[nodiscard]] static std::string extract_func_name(const Serial& obj);

    ///@brief Creates a serial object from inside another serial object
    ///
    ///@param obj The original object to extract the sub-object from
    ///@param index The index of the sub-object, relative to the original object
    ///@return Serial The serial object representing an inner object of the original
    [[nodiscard]] static Serial make_sub_object(const Serial& obj, unsigned index);

    ///@brief Creates a serial object from inside another serial object
    ///
    ///@param obj The original object to extract the sub-object from
    ///@param name The name of the member of the original object to copy out
    ///@return Serial The serial object representing an inner object of the original
    [[nodiscard]] static Serial make_sub_object(const Serial& obj, const std::string& name);

    // TODO: Change get_value to return std::optional?

    ///@brief Extract a value from a serial object
    ///
    ///@tparam T The type of the object to extract
    ///@param obj The serial object to extract from
    ///@return T The extracted type
    template<typename T>
    [[nodiscard]] static T get_value(const Serial& obj);

    ///@brief Populate a container with the contents of a serial object
    ///
    ///@tparam Container Type of container used, must satisfy \ref is_container
    ///@param obj The serial object to populate from
    ///@param container Reference to the container to populate
    template<typename Container>
    static void populate_array(const Serial& obj, Container& container);
};

///@brief Serializes a generic object to a serial object
///
///@note This template must be instantiated for every custom struct/class that needs to be passed
/// as a result or parameter via RPC
///@tparam Serial The type of serial object to use
///@tparam Value The type of generic object to use
///@param val The object to be serialized
///@return Serial The serialized value
template<typename Serial, typename Value>
[[nodiscard]] Serial serialize(const Value& val);

///@brief De-serializes a serial object to a generic object
///
///@note This template must be instantiated for every custom struct/class that needs to be passed as a result or parameter via RPC
///@tparam Serial The type of serial object to use
///@tparam Value The type of generic object to use
///@param serial_obj The serial object to be de-serialized
///@return Value The de-serialized value
template<typename Serial, typename Value>
[[nodiscard]] Value deserialize(const Serial& serial_obj);

namespace details
{
    ///@brief Retrieves a single argument value from a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam Value The type of the argument to be retrieved
    ///@param obj The serial object containing the value
    ///@return std::remove_cv_t<std::remove_reference_t<Value>> The retrieved argument value
    template<typename Serial, typename Value>
    std::remove_cv_t<std::remove_reference_t<Value>> arg_from_serial(const Serial& obj)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<Value>>;

#if defined(RPC_HPP_ENABLE_POINTERS)
        if constexpr (std::is_pointer_v<no_ref_t>)
        {
            // TODO: Implement pointer
        }
#else
        static_assert(!std::is_pointer_v<no_ref_t>,
            "Passing pointers across the RPC interface is not recommended. Please consider "
            "refactoring your RPC calls or define RPC_HPP_ENABLE_POINTERS to ignore this "
            "error.");
#endif

        if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
        {
            return serial_adapter<Serial>::template get_value<no_ref_t>(obj);
        }
        else if constexpr (is_container_v<no_ref_t>)
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

    ///@brief Retrieves the argument values from a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam Value The type of the argument to be retrieved
    ///@param obj The serial object containing the value
    ///@param arg_index The index of the argument to be retrieved (is iterated in a parameter pack when called from a tuple)
    ///@return std::remove_cv_t<std::remove_reference_t<Value>> The retrieved argument value
    template<typename Serial, typename Value>
    std::remove_cv_t<std::remove_reference_t<Value>> args_from_serial(
        const Serial& obj, unsigned& arg_index)
    {
        const auto args = serial_adapter<Serial>::make_sub_object(obj, "args");
        const auto sub_obj = serial_adapter<Serial>::make_sub_object(args, arg_index++);
        return arg_from_serial<Serial, Value>(sub_obj);
    }

    ///@brief Unpacks the argument values from a \ref packed_func
    ///
    ///@tparam Value The type of the argument to be unpacked
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param pack The packaged function call to unpack
    ///@param arg_index The index of the argument to be unpacked (is iterated in a parameter pack when called from a tuple)
    ///@return std::remove_cv_t<std::remove_reference_t<Value>> The unpacked argument value
    template<typename Value, typename R, typename... Args>
    std::remove_cv_t<std::remove_reference_t<Value>> args_from_packed(
        const packed_func<R, Args...>& pack, unsigned& arg_index)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<Value>>;

#if defined(RPC_HPP_ENABLE_POINTERS)
        if constexpr (std::is_pointer_v<no_ref_t>)
        {
            // TODO: Implement pointer
        }
#else
        static_assert(!std::is_pointer_v<no_ref_t>,
            "Passing pointers across the RPC interface is not recommended. Please consider "
            "refactoring your RPC calls or define RPC_HPP_ENABLE_POINTERS to ignore this "
            "error.");
#endif

        return pack.template get_arg<no_ref_t>(arg_index++);
    }
} // namespace rpc::details

///@brief Namespace for server-specific functions and variables
/// Client-side code should not need to use anything in this namespace
namespace server
{
    ///@brief Converts a \ref packed_func_base to a specific templated \ref packed_func
    ///
    ///@tparam R Return type for the \ref packed_func
    ///@tparam Args List of parameter type(s) for the \ref packed_func
    ///@param unused Function object to derive R and Args from
    ///@param pack packed_func_base reference to be converted
    ///@return packed_func<R, Args...>& A casted reference to a specific \ref packed_func
    template<typename R, typename... Args>
    packed_func<R, Args...>& convert_func(
        [[maybe_unused]] std::function<R(Args...)> unused, const packed_func_base& pack)
    {
        return dynamic_cast<packed_func<R, Args...>&>(pack);
    }

    ///@brief Converts a \ref packed_func_base to a specific templated \ref packed_func
    ///
    ///@tparam R Return type for the \ref packed_func
    ///@tparam Args List of parameter type(s) for the \ref packed_func
    ///@param unused Function pointer to derive R and Args from
    ///@param pack packed_func_base reference to be converted
    ///@return packed_func<R, Args...>& A casted reference to a specific \ref packed_func
    template<typename R, typename... Args>
    packed_func<R, Args...>& convert_func(
        [[maybe_unused]] R (*unused)(Args...), const packed_func_base& pack)
    {
        return dynamic_cast<packed_func<R, Args...>&>(pack);
    }

    // TODO: Server-side asynchronous functions (will probably have to return vs. reference)

    ///@brief Create a \ref packed_func object from a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param unused Function pointer to the function to extract R and Args from
    ///@param obj The serial object to be converted
    ///@return packed_func<R, Args...> The packaged function call
    template<typename Serial, typename R, typename... Args>
    packed_func<R, Args...> create_func([[maybe_unused]] R (*unused)(Args...), const Serial& obj)
    {
        return serial_adapter<Serial>::template to_packed_func<R, Args...>(obj);
    }

    ///@brief Runs the callback function and populates the \ref packed_func with the result and/or updated arguments
    ///
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param func The function object to call
    ///@param pack The packaged function call to get/set result and/or parameters
    template<typename R, typename... Args>
    void run_callback(std::function<R(Args...)> func, packed_func<R, Args...>& pack)
    {
        unsigned arg_count = 0;

        std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
            details::args_from_packed<Args, R, Args...>(pack, arg_count)...
        };

        if constexpr (std::is_void_v<R>)
        {
            std::apply(func, args);
            pack.set_args(args);
        }
        else
        {
            auto result = std::apply(func, args);
            pack.set_args(args);
            pack.set_result(result);
        }
    }

    ///@brief Runs the callback function and populates the \ref packed_func with the result and/or updated arguments
    ///
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param func Pointer to the function to call
    ///@param pack The packaged function call to get/set result and/or parameters
    template<typename R, typename... Args>
    void run_callback(R (*func)(Args...), packed_func<R, Args...>& pack)
    {
        return run_callback(std::function<R(Args...)>(func), pack);
    }

    ///@brief Dispatches the function call based on the received serial object
    ///
    ///@note This function must be implemented in the server-side code (or by using the macros found in rpc_dispatch_helper.hpp)
    ///@tparam Serial The type of serial object
    ///@param serial_obj The serial object to be used by the function call, will (potentially) be modified by calling the function
    template<typename Serial>
    void dispatch(Serial& serial_obj);
} // namespace rpc::server

///@brief Namespace containing client-specific functions and classes
/// Server-side code should not need anything from this namespace
inline namespace client
{
    ///@brief Polymorphic base class for sending and receiving data to/from the server
    ///@note client_base must be inherited by a client-side class
    class client_base
    {
    public:
        virtual ~client_base() = default;
        virtual void send(const std::string& mesg) = 0;
        virtual std::string receive() = 0;
    };

    ///@brief Packages a function call into a \ref packed_func
    ///
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param func_name The name of the function to call (case-sensitive)
    ///@param args List of arguments to be packaged
    ///@return packed_func<R, Args...> The packaged function call
    template<typename R, typename... Args>
    packed_func<R, Args...> pack_call(const std::string& func_name, Args&&... args)
    {
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

    ///@brief Transforms a function call to a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param func_name The name of the function to call (case-sensitive)
    ///@param args The list of parameters for the function call
    ///@return Serial The serial object representing the function call
    template<typename Serial, typename R = void, typename... Args>
    Serial serialize_call(const std::string& func_name, Args&&... args)
    {
        auto packed = pack_call<R, Args...>(func_name, std::forward<Args>(args)...);
        return serial_adapter<Serial>::template from_packed_func<R, Args...>(packed);
    }

    ///@brief Transforms a function call to a serial object (asynchronously)
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param func_name The name of the function to call (case-sensitive)
    ///@param args The list of parameters for the function call
    ///@return std::future<Serial> Future of the serial object representing the function call
    template<typename Serial, typename R = void, typename... Args>
    std::future<Serial> async_serialize_call(const std::string& func_name, Args&&... args)
    {
        auto packed = pack_call<R, Args...>(func_name, std::forward<Args>(args)...);
        return std::async(serial_adapter<Serial>::template from_packed_func<R, Args...>, packed);
    }

    ///@brief Sends a serialized function call to the server
    ///
    ///@tparam Serial The type of serial object
    ///@param serial_obj The serial object representing the function call
    ///@param client The client object to send from
    template<typename Serial>
    void send_to_server(const Serial& serial_obj, client_base& client)
    {
        client.send(serial_adapter<Serial>::to_string(serial_obj));
    }

    ///@brief Gets the server's response to the client call
    ///
    ///@tparam Serial The type of serial object
    ///@param client The client object to receive from
    ///@return Serial The received serial object representing the function call result
    template<typename Serial>
    Serial get_server_response(client_base& client)
    {
        return serial_adapter<Serial>::from_string(client.receive());
    }

    ///@brief Packages and sends/receives a serialized function call in one easy function
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param client The client object to send/receive to/from
    ///@param func_name The name of the function to call on the server (case-sensitive)
    ///@param args The list of parameters for the function call
    ///@return packed_func<R, Args...> A packaged function call with the result and updated parameters
    template<typename Serial, typename R = void, typename... Args>
    packed_func<R, Args...> call(client_base& client, const std::string& func_name, Args&&... args)
    {
        const auto serial_obj =
            serialize_call<Serial, R, Args...>(func_name, std::forward<Args>(args)...);

        send_to_server(serial_obj, client);
        const auto resp_obj = get_server_response<Serial>(client);
        return serial_adapter<Serial>::template to_packed_func<R, Args...>(resp_obj);
    }

    ///@brief Packages and sends/receives a serialized function call in one easy function (asynchronously)
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param client The client object to send/receive to/from
    ///@param func_name The name of the function to call on the server (case-sensitive)
    ///@param args The list of parameters for the function call
    ///@return std::future<packed_func<R, Args...>> Future of the packaged function call with the result and updated parameters
    template<typename Serial, typename R = void, typename... Args>
    std::future<packed_func<R, Args...>> async_call(
        client_base& client, const std::string& func_name, Args&&... args)
    {
        return std::async(call<Serial, R, Args...>, client, func_name, args...);
    }
} // namespace rpc::client
} // namespace rpc
