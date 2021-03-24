///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.3.3
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2021, Jackson Harmer
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

#include <algorithm>
#include <any>
#include <array>
#include <cstddef>
#include <functional>
#include <future>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace rpc
{
///@brief Namespace for functions/variables that should be used only from within the library.
/// Using anything in this namespace in your project is discouraged
namespace details
{
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
    struct is_serializable
        : std::integral_constant<bool,
              is_serializable_base<Value, typename Serial::doc_type(const Value&)>::value
                  && is_deserializable_base<Value,
                      Value(const typename Serial::value_type&)>::value>
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
    struct is_container : std::integral_constant<bool,
                              has_size<C>::value && has_begin<C>::value && has_end<C>::value>
    {
    };

    /// @brief Helper variable for @ref is_container
    ///
    /// @tparam C Type to check
    template<typename C>
    inline constexpr bool is_container_v = is_container<C>::value;

    ///@brief Structure to decay arrays to pointers
    ///
    ///@tparam T Type to decay
    template<typename T>
    struct ptr_decay
    {
    private:
        using U = std::remove_cv_t<std::remove_reference_t<T>>;

    public:
        using type =
            std::conditional_t<std::is_array_v<U>, typename std::remove_extent<U>::type*, T>;
    };

    ///@brief Helper for ptr_decay struct
    ///
    ///@tparam T Type to decay
    template<typename T>
    using ptr_decay_t = typename ptr_decay<T>::type;

    ///@brief Dummy struct for all_true
    template<bool...>
    struct bool_pack;

    ///@brief SFINAE struct for checking if all packed bool parameters are true
    ///
    ///@tparam bs Packed bools to check
    template<bool... bs>
    using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

    ///@brief Helper for all_true
    ///
    ///@tparam bs Packed bools to check
    template<bool... bs>
    inline constexpr bool all_true_v = all_true<bs...>::value;

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

    ///@brief Dynamically sized generic array class. Has a fixed max capacity and a dynamic size
    ///
    ///@tparam T Type to hold in array
    template<typename T>
    class dyn_array
    {
    public:
        ~dyn_array() { delete[] m_ptr; }

        ///@brief Construct a new dyn_array object
        ///
        ///@param capacity The maximum possible size for the array, cannot be changed by the user
        explicit dyn_array(const size_t capacity) : m_capacity(capacity), m_ptr(new T[m_capacity])
        {
        }

        dyn_array(const dyn_array& other)
            : m_capacity(other.m_capacity), m_size(other.m_size), m_ptr(new T[m_capacity])
        {
            std::copy_n(other.m_ptr, m_size, m_ptr);
        }

        dyn_array(dyn_array&& other) noexcept
            : m_capacity(other.m_capacity), m_size(other.m_size), m_ptr(std::move(other.m_ptr))
        {
            other.m_capacity = 0;
            other.m_size = 0;
            other.m_ptr = nullptr;
        }

        dyn_array& operator=(const dyn_array& other) &
        {
            if (&other != this)
            {
                m_capacity = other.m_capacity;
                m_size = other.m_size;
                std::copy_n(other.m_ptr, m_size, m_ptr);
            }

            return *this;
        }

        dyn_array& operator=(dyn_array&& other) & noexcept
        {
            if (&other != this)
            {
                m_capacity = other.m_capacity;
                m_size = other.m_size;
                m_ptr = std::move(other.m_ptr);
                other.m_capacity = 0;
                other.m_size = 0;
                other.m_ptr = nullptr;
            }

            return *this;
        }

        ///@brief Get the max capacity of the array
        ///
        ///@return size_t The max capacity
        [[nodiscard]] size_t capacity() const noexcept { return m_capacity; }

        ///@brief Get the current size of the array
        ///
        ///@return size_t The current size
        [[nodiscard]] size_t size() const noexcept { return m_size; }

        ///@brief Pushes an item to the end of the array via a copy
        ///
        ///@param val Value to push
        void push_back(const T& val) &
        {
            if (m_size == m_capacity)
            {
                throw std::runtime_error("Cannot push_back, array is full!");
            }

            m_ptr[m_size++] = val;
        }

        ///@brief Pushes an item to the end of the array via a move
        ///
        ///@param val Value to push
        void push_back(T&& val) &
        {
            if (m_size == m_capacity)
            {
                throw std::runtime_error("Cannot push_back, array is full!");
            }

            m_ptr[m_size++] = std::move(val);
        }

        ///@brief Pushes an item to the beginning of the array via a copy
        ///
        ///@param val Value to push
        void push_front(const T& val) &
        {
            if (m_size == m_capacity)
            {
                throw std::runtime_error("Cannot push_front, array is full!");
            }

            for (size_t i = m_size; i > 0; --i)
            {
                m_ptr[i] = std::move(m_ptr[i - 1]);
            }

            m_ptr[0] = val;
            ++m_size;
        }

        ///@brief Pushes an item to the beginning of the array via a move
        ///
        ///@param val Value to push
        void push_front(T&& val) &
        {
            if (m_size == m_capacity)
            {
                throw std::runtime_error("Cannot push_front, array is full!");
            }

            for (size_t i = m_size; i > 0; --i)
            {
                m_ptr[i] = std::move(m_ptr[i - 1]);
            }

            m_ptr[0] = std::move(val);
            ++m_size;
        }

        ///@brief Returns the pointer to the data in the array
        ///
        ///@return T* Pointer held by array
        [[nodiscard]] T* data() & noexcept { return m_ptr; }

        ///@brief Returns the pointer to the data in the array
        ///
        ///@return const T* Pointer held by array
        [[nodiscard]] const T* data() const& noexcept { return m_ptr; }

        ///@brief Returns the pointer to the beginning of the array
        ///
        ///@return T* Pointer to beginning of array
        [[nodiscard]] T* begin() & noexcept { return m_ptr; }

        ///@brief Returns the pointer to the beginning of the array
        ///
        ///@return const T* Pointer to beginning of array
        [[nodiscard]] const T* begin() const& noexcept { return m_ptr; }

        ///@brief Returns the pointer to one past the end of the array
        ///
        ///@return T* Pointer to one past end of array
        [[nodiscard]] T* end() & noexcept { return &m_ptr[m_size]; }

        ///@brief Returns the pointer to one past the end of the array
        ///
        ///@return const T* Pointer to one past end of array
        [[nodiscard]] const T* end() const& noexcept { return &m_ptr[m_size]; }

        ///@brief Returns the first element in the array by value
        ///
        ///@exception std::runtime_error Thrown if array is empty
        ///@return T First element of array
        [[nodiscard]] T front() &&
        {
            if (m_size == 0)
            {
                throw std::runtime_error("Array is empty!");
            }

            return std::move(m_ptr[0]);
        }

        ///@brief Returns a reference to the first element in the array
        ///
        ///@exception std::runtime_error Thrown if array is empty
        ///@return T& Reference to first element of array
        [[nodiscard]] T& front() &
        {
            if (m_size == 0)
            {
                throw std::runtime_error("Array is empty!");
            }

            return m_ptr[0];
        }

        ///@brief Returns a reference to the first element in the array
        ///
        ///@exception std::runtime_error Thrown if array is empty
        ///@return const T& Reference to first element of array
        [[nodiscard]] const T& front() const&
        {
            if (m_size == 0)
            {
                throw std::runtime_error("Array is empty!");
            }

            return m_ptr[0];
        }

        ///@brief Returns the last element in the array by value
        ///
        ///@exception std::runtime_error Thrown if array is empty
        ///@return T Last element of array
        [[nodiscard]] T back() &&
        {
            if (m_size == 0)
            {
                throw std::runtime_error("Array is empty!");
            }

            return std::move(m_ptr[m_size - 1]);
        }

        ///@brief Returns a reference to the last element in the array
        ///
        ///@exception std::runtime_error Thrown if array is empty
        ///@return T& Reference to last element of array
        [[nodiscard]] T& back() &
        {
            if (m_size == 0)
            {
                throw std::runtime_error("Array is empty!");
            }

            return m_ptr[m_size - 1];
        }

        ///@brief Returns a reference to the last element in the array
        ///
        ///@exception std::runtime_error Thrown if array is empty
        ///@return const T& Reference to last element of array
        [[nodiscard]] const T& back() const&
        {
            if (m_size == 0)
            {
                throw std::runtime_error("Array is empty!");
            }

            return m_ptr[m_size - 1];
        }

    private:
        size_t m_capacity;
        size_t m_size{ 0 };
        T* m_ptr;
    };

    ///@brief Polymorphic base class for \ref packed_func
    template<typename... Args>
    class packed_func_base
    {
    public:
        using args_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

        virtual ~packed_func_base() = default;

        packed_func_base(std::string func_name, args_type args)
            : m_func_name(std::move(func_name)), m_args(std::move(args))
        {
        }

        // Prevents slicing
        packed_func_base& operator=(const packed_func_base&) = delete;
        packed_func_base& operator=(packed_func_base&&) = delete;

        [[nodiscard]] std::string& get_func_name() & noexcept { return m_func_name; }
        [[nodiscard]] const std::string& get_func_name() const& noexcept { return m_func_name; }
        [[nodiscard]] std::string get_func_name() && noexcept { return std::move(m_func_name); }

        [[nodiscard]] std::string& get_err_mesg() & noexcept { return m_err_mesg; }
        [[nodiscard]] const std::string& get_err_mesg() const& noexcept { return m_err_mesg; }
        [[nodiscard]] std::string get_err_mesg() && noexcept { return std::move(m_err_mesg); }

        void set_err_mesg(const std::string& mesg) & noexcept { m_err_mesg = mesg; }
        void set_err_mesg(std::string&& mesg) & noexcept { m_err_mesg = std::move(mesg); }

        explicit virtual operator bool() const noexcept { return true; }

        [[nodiscard]] args_type& get_args() & noexcept { return m_args; }
        [[nodiscard]] const args_type& get_args() const& noexcept { return m_args; }
        [[nodiscard]] args_type get_args() && noexcept { return std::move(m_args); }

        template<typename T, size_t Index>
        T& get_arg() &
        {
            return std::get<Index>(m_args);
        }

        template<typename T, size_t Index>
        const T& get_arg() const&
        {
            return std::get<Index>(m_args);
        }

        template<typename T, size_t Index>
        T get_arg() &&
        {
            return std::move(std::get<Index>(m_args));
        }

        ///@brief Set all arguments
        ///
        ///@param args A tuple containing the list of arguments to set
        void set_args(args_type&& args) & { m_args = std::move(args); }

    protected:
        packed_func_base(const packed_func_base&) = default;
        packed_func_base(packed_func_base&&) noexcept = default;

    private:
        std::string m_func_name;
        std::string m_err_mesg{};
        args_type m_args;

#if defined(RPC_HPP_ENABLE_POINTERS)
        std::array<size_t, sizeof...(Args)> m_arg_sz_arr{};

        template<typename T>
        void update_arg_arr_helper(
            [[maybe_unused]] std::array<std::any, sizeof...(Args)>&& arg_arr, size_t& count) &
        {
            if constexpr (std::is_pointer_v<T>)
            {
                const auto& arr = std::any_cast<
                    const details::dyn_array<std::remove_cv_t<std::remove_pointer_t<T>>>&>(
                    std::move(arg_arr[count]));

                m_arg_sz_arr[count] = arr.capacity();
            }

            ++count;
        }

    public:
        ///@brief Get the indexed argument size
        ///
        ///@param index The index to get the arg size for
        ///@return size_t The size (number of elements) for the given argument
        [[nodiscard]] size_t get_arg_arr_sz(const size_t index) const
        {
            return m_arg_sz_arr[index];
        }

        ///@brief Set the size for the indexed argument
        ///
        ///@param index The index to set the arg size for
        ///@param sz The size (number of elements) for the given argument
        void set_arg_arr_sz(const size_t index, const size_t sz) & { m_arg_sz_arr[index] = sz; }

        ///@brief Update the argument size array
        ///
        ///@param arg_arr Array of arguments to reference
        void update_arg_arr(std::array<std::any, sizeof...(Args)> arg_arr) &
        {
            size_t count = 0;
            using expander = int[];
            (void)expander{ 0,
                ((void)update_arg_arr_helper<Args>(std::move(arg_arr), count), 0)... };
        }
#endif
    };
} // namespace rpc::details

///@brief Class reprensenting a function call including its name, result, and parameters
///
///@tparam R The return type
///@tparam Args The list of parameter type(s)
template<typename R, typename... Args>
class packed_func final : public details::packed_func_base<Args...>
{
public:
    ///@brief The type of the packed_func's result
    using result_type = R;
    using typename details::packed_func_base<Args...>::args_type;

    ///@brief Construct a new packed_func object
    ///
    ///@param func_name Name of the function (case-sensitive)
    ///@param result Function call result (if no result yet, use std::nullopt)
    ///@param args List of parameters for the function call
    packed_func(std::string func_name, std::optional<result_type> result, args_type args)
        : details::packed_func_base<Args...>(std::move(func_name), std::move(args)),
          m_result(std::move(result))
    {
    }

    packed_func(const packed_func&) = default;
    packed_func(packed_func&&) noexcept = default;
    packed_func& operator=(const packed_func&) & = default;
    packed_func& operator=(packed_func&&) & = default;

    explicit operator bool() const noexcept override { return m_result.has_value(); }

    [[nodiscard]] R get_result() const
    {
        if (m_result.has_value())
        {
            return m_result.value();
        }

        throw std::runtime_error(this->get_err_mesg());
    }

    ///@brief Set the result
    ///
    ///@param value Value of type R to set as the result
    void set_result(R value) & noexcept { m_result = std::move(value); }

    ///@brief Sets the result back to null
    void clear_result() & noexcept { m_result = std::nullopt; }

private:
    std::optional<result_type> m_result{ std::nullopt };
};

///@brief Class reprensenting a function call (with void result) including its name and parameters
///
///@tparam Args The list of parameter type(s)
template<typename... Args>
class packed_func<void, Args...> final : public details::packed_func_base<Args...>
{
public:
    using result_type = void;
    using typename details::packed_func_base<Args...>::args_type;

    packed_func(std::string func_name, args_type args)
        : details::packed_func_base<Args...>(std::move(func_name), std::move(args))
    {
    }

    packed_func(const packed_func&) = default;
    packed_func(packed_func&&) noexcept = default;
    packed_func& operator=(const packed_func&) & = default;
    packed_func& operator=(packed_func&&) & = default;
};

template<typename Value_T, typename Doc_T = Value_T>
struct serial_t
{
    using value_type = Value_T;
    using doc_type = Doc_T;
};

///@brief Template class that provides an interface for going to and from a \ref packed_func and a serial object
///
///@note Member functions of serial_adapter are to be implemented by an adapter
///@tparam Serial The serial object type
template<typename Serial>
class serial_adapter
{
public:
    using value_type = typename Serial::value_type;
    using doc_type = typename Serial::doc_type;

    ///@brief Converts a serial object into a specific \ref packed_func
    ///
    ///@tparam R The result type for the \ref packed_func
    ///@tparam Args The list of paramter type(s) for the \ref packed_func
    ///@param serial_obj The serial object to be converted
    ///@return packed_func<R, Args...> The packaged function call
    template<typename R, typename... Args>
    [[nodiscard]] static packed_func<R, Args...> to_packed_func(const doc_type& serial_obj);

    ///@brief Converts a \ref packed_func into a serial object
    ///
    ///@tparam R The result type for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param pack The packaged function call to be converted
    ///@return value_type The serial object
    template<typename R, typename... Args>
    [[nodiscard]] static doc_type from_packed_func(packed_func<R, Args...>&& pack);

    ///@brief Converts a serial object to a readable std::string
    ///
    ///@param serial_obj The serial object to be converted
    ///@return std::string The string representation of the serial object
    [[nodiscard]] static std::string to_string(const doc_type& serial_obj);

    ///@brief Parses a std::string into a serial object
    ///
    ///@param str The string to be parsed
    ///@return value_type The serial object represented by the string
    [[nodiscard]] static doc_type from_string(const std::string& str);

    ///@brief Retrieve the function name from a serial object
    ///
    ///@param obj The serial object to retrieve the function name from
    ///@return std::string The function name (case-sensitive)
    [[nodiscard]] static std::string extract_func_name(const value_type& obj);

    static void set_err_mesg(doc_type& serial_obj, const std::string& str);

    ///@brief Creates a serial object from inside another serial object
    ///
    ///@param obj The original object to extract the sub-object from
    ///@param index The index of the sub-object, relative to the original object
    ///@return value_type The serial object representing an inner object of the original
    [[nodiscard]] static doc_type make_sub_object(const value_type& obj, unsigned index);

    ///@brief Creates a serial object from inside another serial object
    ///
    ///@param obj The original object to extract the sub-object from
    ///@param name The name of the member of the original object to copy out
    ///@return value_type The serial object representing an inner object of the original
    [[nodiscard]] static doc_type make_sub_object(const value_type& obj, const std::string& name);

    ///@brief Extract a value from a serial object
    ///
    ///@tparam T The type of the object to extract
    ///@param obj The serial object to extract from
    ///@return T The extracted type
    template<typename T>
    [[nodiscard]] static T get_value(const value_type& obj);

    template<typename R>
    static void set_result(doc_type& serial_obj, R val);

    ///@brief Populate a container with the contents of a serial object
    ///
    ///@tparam Container Type of container used, must satisfy \ref is_container
    ///@param obj The serial object to populate from
    ///@param container Reference to the container to populate
    template<typename Container>
    static void populate_array(const value_type& obj, Container& container);

    [[nodiscard]] static size_t get_num_args(const value_type& obj);

#if defined(RPC_HPP_ENABLE_POINTERS)
    ///@brief Converts a serial object into a specific \ref packed_func (accepts pointers)
    ///
    ///@tparam R The result type for the \ref packed_func
    ///@tparam Args The list of paramter type(s) for the \ref packed_func
    ///@param serial_obj The serial object to be converted
    ///@param arg_arr Array containing the pointer arguments
    ///@return packed_func<R, Args...> The packaged function call
    template<typename R, typename... Args>
    [[nodiscard]] static packed_func<R, Args...> to_packed_func_w_ptr(
        const value_type& serial_obj, const std::array<std::any, sizeof...(Args)>& arg_arr);

    ///@brief Parse a serial object to create a \ref dyn_array of objects
    ///
    ///@tparam Value The type of object held in the array
    ///@param arg_obj value_type object representing an arg array.
    /// Arg arrays have two data members:
    /// "c" containing a uint64 indicating the max capacity of the array,
    /// and "d" containing an array of the actual data held
    ///@return details::dyn_array<Value> The dynamic array containing the values held in the serial object
    template<typename Value>
    [[nodiscard]] static details::dyn_array<Value> parse_arg_arr(const value_type& arg_obj);
#endif
};

///@brief value_typeizes a generic object to a serial object
///
///@note This template must be instantiated for every custom struct/class that needs to be passed
/// as a result or parameter via RPC
///@tparam Serial The type of serial object to use
///@tparam Value The type of generic object to use
///@param val The object to be serialized
///@return value_type The serialized value
template<typename Serial, typename Value>
[[nodiscard]] typename Serial::doc_type serialize(const Value& val);

///@brief De-serializes a serial object to a generic object
///
///@note This template must be instantiated for every custom struct/class that needs to be passed as a result or parameter via RPC
///@tparam Serial The type of serial object to use
///@tparam Value The type of generic object to use
///@param serial_obj The serial object to be de-serialized
///@return Value The de-serialized value
template<typename Serial, typename Value>
[[nodiscard]] Value deserialize(const typename Serial::value_type& serial_obj);

namespace details
{
    ///@brief Retrieves a single argument value from a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam Value The type of the argument to be retrieved
    ///@param obj The serial object containing the value
    ///@return std::remove_cv_t<std::remove_reference_t<Value>> The retrieved argument value
    template<typename Serial, typename Value>
    std::remove_cv_t<std::remove_reference_t<Value>> arg_from_serial(
        const typename Serial::value_type& obj)
    {
        using no_ref_t = std::remove_cv_t<std::remove_reference_t<Value>>;

        static_assert(!std::is_pointer_v<no_ref_t>, "Pointers should not reach this function");

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
        else if constexpr (details::is_serializable_v<Serial, no_ref_t>)
        {
            // Attempt to find static T::deserialize function
            return no_ref_t::deserialize(obj);
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
        const typename Serial::value_type& obj, unsigned& arg_index)
    {
        const auto args = serial_adapter<Serial>::make_sub_object(obj, "args");
        const auto& sub_obj = serial_adapter<Serial>::make_sub_object(args, arg_index++);
        return arg_from_serial<Serial, Value>(sub_obj);
    }

#if defined(RPC_HPP_ENABLE_POINTERS)
    ///@brief Retrieves the argument values from a serial object (accepts pointers)
    ///
    ///@tparam Serial The type of serial object
    ///@tparam Value The type of the argument to be retrieved
    ///@tparam N Size of array (auto deduced)
    ///@param serial_obj The serial object containing the value
    ///@param arg_arr The array containing pointer arguments
    ///@param arg_index The index of the argument to be retrieved (is iterated in a parameter pack when called from a tuple)
    ///@return std::remove_cv_t<std::remove_reference_t<Value>> The retrieved argument value
    template<typename Serial, typename Value, size_t N>
    std::remove_cv_t<std::remove_reference_t<Value>> args_from_serial_w_ptr(
        const typename Serial::value_type& serial_obj, const std::array<std::any, N>& arg_arr,
        unsigned& arg_index)
    {
        if constexpr (std::is_pointer_v<Value>)
        {
            const auto& arr =
                std::any_cast<const dyn_array<std::remove_cv_t<std::remove_pointer_t<Value>>>&>(
                    arg_arr[arg_index++]);
            auto* x = const_cast<Value>(arr.data());
            return x;
        }
        else
        {
            return args_from_serial<Serial, Value>(serial_obj, arg_index);
        }
    }

    ///@brief Helper function to be used by unpacking the parameters from \ref populate_arg_arr
    ///
    ///@tparam T The type of argument to look for
    ///@tparam Serial The serial object type
    ///@tparam N Size of the array (auto deduced)
    ///@param arg_list The serial object representing the argument list
    ///@param arg_arr The argument array
    ///@param count The index of the argument
    template<typename T, typename Serial, size_t N>
    void populate_arg_arr_helper(const typename Serial::value_type& arg_list,
        std::array<std::any, N>& arg_arr, unsigned& count)
    {
        if constexpr (std::is_pointer_v<T>)
        {
            const auto& arg = serial_adapter<Serial>::make_sub_object(arg_list, count);
            auto arr = serial_adapter<Serial>::template parse_arg_arr<
                std::remove_cv_t<std::remove_pointer_t<T>>>(arg);
            arg_arr[count++] = std::move(arr);
        }
        else
        {
            arg_arr[count++] = std::nullopt;
        }
    }

    ///@brief Creates an array containing dynamic arrays for each pointer argument (and nullopt for non-pointer args)
    ///
    ///@tparam Serial The serial object type to parse
    ///@tparam Args The list of argument types
    ///@param serial_obj The serial object to parse
    ///@return std::array<std::any, sizeof...(Args)> Array containing pointer arguments as \ref dyn_array
    template<typename Serial, typename... Args>
    std::array<std::any, sizeof...(Args)> populate_arg_arr(
        const typename Serial::value_type& serial_obj)
    {
        const auto& arg_list = serial_adapter<Serial>::make_sub_object(serial_obj, "args");
        unsigned count = 0;
        std::array<std::any, sizeof...(Args)> arg_arr;

        using expander = int[];
        (void)expander{ 0,
            ((void)populate_arg_arr_helper<Args, Serial>(arg_list, arg_arr, count), 0)... };

        return arg_arr;
    }
#endif
} // namespace rpc::details

///@brief Namespace for server-specific functions and variables
/// Client-side code should not need to use anything in this namespace
namespace server
{
    ///@brief Create a \ref packed_func object from a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param unused Function pointer to the function to extract R and Args from
    ///@param obj The serial object to be converted
    ///@return packed_func<R, Args...> The packaged function call
    template<typename Serial, typename R, typename... Args>
    packed_func<R, Args...> create_func(
        [[maybe_unused]] R (*unused)(Args...), const typename Serial::doc_type& obj)
    {
        return serial_adapter<Serial>::template to_packed_func<R, Args...>(obj);
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
        auto args = pack.get_args();

        if constexpr (std::is_void_v<R>)
        {
            std::apply(func, args);
            pack.set_args(std::move(args));
        }
        else
        {
            try
            {
                auto result = std::apply(func, args);
                pack.set_result(std::move(result));
            }
            catch (const std::exception&)
            {
                pack.clear_result();
                throw;
            }

            pack.set_args(std::move(args));
        }
    }

#if defined(RPC_HPP_ENABLE_POINTERS)
    ///@brief Create a \ref packed_func object from a serial object (accepts pointers)
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param unused Function pointer to the function to extract R and Args from
    ///@param arg_arr The argument array containing the pointer arguments
    ///@param obj The serial object to be converted
    ///@return packed_func<R, Args...> The packaged function call
    template<typename Serial, typename R, typename... Args>
    packed_func<R, Args...> create_func_w_ptr([[maybe_unused]] R (*unused)(Args...),
        const std::array<std::any, sizeof...(Args)>& arg_arr, const typename Serial::doc_type& obj)
    {
        return serial_adapter<Serial>::template to_packed_func_w_ptr<R, Args...>(obj, arg_arr);
    }
#endif

    template<typename Serial>
    std::unordered_map<std::string, std::any>& get_cache()
    {
        static std::unordered_map<std::string, std::any> result_cache;
        return result_cache;
    }

    template<typename Serial, typename R>
    bool check_cache(typename Serial::doc_type& serial_obj)
    {
        const std::string objStr = serial_adapter<Serial>::to_string(serial_obj);
        auto& result_cache = get_cache<Serial>();

        const auto it = result_cache.find(objStr);

        if (it != result_cache.end())
        {
            serial_adapter<Serial>::set_result(serial_obj, std::any_cast<R>(result_cache[objStr]));
            return true;
        }

        return false;
    }

    template<typename Serial, typename R>
    void update_cache(std::string&& key, R val)
    {
        get_cache<Serial>()[std::move(key)] = val;
    }

    ///@brief Dispatches the function call based on the received serial object
    ///
    ///@note This function must be implemented in the server-side code (or by using the macros found in rpc_dispatch_helper.hpp)
    ///@tparam Serial The type of serial object
    ///@param serial_obj The serial object to be used by the function call, will (potentially) be modified by calling the function
    template<typename Serial>
    void dispatch(typename Serial::doc_type& serial_obj);

    ///@brief Dispatches an individual function (to be called from \ref dispatch)
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The return type of the function
    ///@tparam Args The argument list of the function
    ///@param func Pointer to the function
    ///@param serial_obj Serial object to be used by the function call, will (potentially) be modified by calling the function
    ///@param cacheable Determines whether the function is cacheable or not (default false)
    template<typename Serial, typename R, typename... Args>
    void dispatch_func(
        R (*func)(Args...), typename Serial::doc_type& serial_obj, bool cacheable = false)
    {
        try
        {
#if defined(RPC_HPP_ENABLE_SERVER_CACHE)
            if constexpr (!std::is_void_v<R>)
            {
                if (cacheable && check_cache<Serial, R>(serial_obj))
                {
                    return;
                }
            }
#endif

#if defined(RPC_HPP_ENABLE_POINTERS)
            if constexpr (
                details::all_true_v<!(
                    std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<
                        Args>>> || std::is_array_v<std::remove_cv_t<std::remove_reference_t<Args>>>)...>)
            {
                auto pack = create_func<Serial>(func, serial_obj);
                run_callback(func, pack);

#    if defined(RPC_HPP_ENABLE_SERVER_CACHE)
                if constexpr (!std::is_void_v<R>)
                {
                    if (cacheable)
                    {
                        update_cache<Serial, R>(
                            serial_adapter<Serial>::to_string(serial_obj), pack.get_result());
                    }
                }
#    endif

                serial_obj = serial_adapter<Serial>::from_packed_func(std::move(pack));
            }
            else
            {
                const auto arg_arr = details::populate_arg_arr<Serial, Args...>(serial_obj);
                auto pack = create_func_w_ptr<Serial>(func, arg_arr, serial_obj);
                run_callback(func, pack);

#    if defined(RPC_HPP_ENABLE_SERVER_CACHE)
                if constexpr (!std::is_void_v<R>)
                {
                    if (cacheable)
                    {
                        update_cache<Serial, R>(
                            serial_adapter<Serial>::to_string(serial_obj), pack.get_result());
                    }
                }
#    endif

                serial_obj = serial_adapter<Serial>::from_packed_func(std::move(pack));
            }
#else
            auto pack = create_func<Serial>(func, serial_obj);
            run_callback(func, pack);

#    if defined(RPC_HPP_ENABLE_SERVER_CACHE)
            if constexpr (!std::is_void_v<R>)
            {
                if (cacheable)
                {
                    update_cache<Serial, R>(
                        serial_adapter<Serial>::to_string(serial_obj), pack.get_result());
                }
            }
#    endif

            serial_obj = serial_adapter<Serial>::from_packed_func(std::move(pack));
#endif
        }
        catch (const std::exception& ex)
        {
            serial_adapter<Serial>::set_err_mesg(serial_obj, ex.what());
        }
    }
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

#if defined(RPC_HPP_ENABLE_POINTERS)
    ///@brief Packs an argument based on whether it is a pointer, array, or other
    ///
    /// If the argument is an array, it will record the size of the array and then decay to a pointer for return.
    /// If it is a pointer (not array), it will set a size of 1 and then return.
    /// Otherwise it will simply return
    ///@tparam T The type of argument to pack
    ///@param val The value to pack
    ///@param arg_sz_arr Pointer to an array containing the size of each argument
    ///@param index The index of the argument to pack
    ///@return details::ptr_decay_t<T> The decayed value of val
    template<typename T>
    details::ptr_decay_t<T> pack_arg(T& val, size_t* arg_sz_arr, unsigned& index)
    {
        if constexpr (std::is_array_v<std::remove_reference_t<T>>)
        {
            arg_sz_arr[index++] = sizeof(val) / sizeof(val[0]);
            return &val[0];
        }
        else if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
        {
            arg_sz_arr[index++] = val == nullptr ? 0 : 1;
            return val;
        }
        else
        {
            return val;
        }
    }

    ///@brief Packages a function call into a \ref packed_func
    ///
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param func_name The name of the function to call (case-sensitive)
    ///@param args List of arguments to be packaged
    ///@return packed_func<R, Args...> The packaged function call
    template<typename R, typename... Args>
    packed_func<R, details::ptr_decay_t<Args>...> pack_call(std::string&& func_name, Args&&... args)
    {
        std::array<size_t, sizeof...(Args)> arg_sz_arr{};
        unsigned i = 0;

        typename packed_func<R, details::ptr_decay_t<Args>...>::args_type argTup{ pack_arg(
            args, arg_sz_arr.data(), i)... };

        if constexpr (std::is_void_v<R>)
        {
            packed_func<void, details::ptr_decay_t<Args>...> pack(
                std::move(func_name), std::move(argTup));

            for (size_t j = 0; j < sizeof...(Args); ++j)
            {
                pack.set_arg_arr_sz(j, arg_sz_arr[j]);
            }

            return pack;
        }
        else
        {
            packed_func<R, details::ptr_decay_t<Args>...> pack(
                std::move(func_name), std::nullopt, std::move(argTup));

            for (size_t j = 0; j < sizeof...(Args); ++j)
            {
                pack.set_arg_arr_sz(j, arg_sz_arr[j]);
            }

            return pack;
        }
    }
#else
    ///@brief Packages a function call into a \ref packed_func
    ///
    ///@tparam R The type of the result for the \ref packed_func
    ///@tparam Args The list of parameter type(s) for the \ref packed_func
    ///@param func_name The name of the function to call (case-sensitive)
    ///@param args List of arguments to be packaged
    ///@return packed_func<R, Args...> The packaged function call
    template<typename R, typename... Args>
    packed_func<R, Args...> pack_call(std::string&& func_name, Args&&... args)
    {
        if constexpr (std::is_void_v<R>)
        {
            return packed_func<void, Args...>(std::move(func_name), std::forward_as_tuple(args...));
        }
        else
        {
            return packed_func<R, Args...>(
                std::move(func_name), std::nullopt, std::forward_as_tuple(args...));
        }
    }
#endif

    ///@brief Transforms a function call to a serial object
    ///
    ///@tparam Serial The type of serial object
    ///@tparam R The type of the result for the function call
    ///@tparam Args The list of parameter type(s) for the function call
    ///@param func_name The name of the function to call (case-sensitive)
    ///@param args The list of parameters for the function call
    ///@return Serial The serial object representing the function call
    template<typename Serial, typename R = void, typename... Args>
    typename Serial::doc_type serialize_call(std::string&& func_name, Args&&... args)
    {
        auto packed = pack_call<R, Args...>(std::move(func_name), std::forward<Args>(args)...);

        return serial_adapter<Serial>::template from_packed_func<R, details::ptr_decay_t<Args>...>(
            std::move(packed));
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
    std::future<typename Serial::doc_type> async_serialize_call(
        const std::string& func_name, Args&&... args)
    {
        auto packed = pack_call<R, Args...>(func_name, std::forward<Args>(args)...);
        return std::async(
            serial_adapter<Serial>::template from_packed_func<R, Args...>, std::move(packed));
    }

    ///@brief Sends a serialized function call to the server
    ///
    ///@tparam Serial The type of serial object
    ///@param serial_obj The serial object representing the function call
    ///@param client The client object to send from
    template<typename Serial>
    void send_to_server(const typename Serial::doc_type& serial_obj, client_base& client)
    {
        client.send(serial_adapter<Serial>::to_string(serial_obj));
    }

    ///@brief Gets the server's response to the client call
    ///
    ///@tparam Serial The type of serial object
    ///@param client The client object to receive from
    ///@return Serial The received serial object representing the function call result
    template<typename Serial>
    typename Serial::doc_type get_server_response(client_base& client)
    {
        return serial_adapter<Serial>::from_string(client.receive());
    }

#if defined(RPC_HPP_ENABLE_POINTERS)
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
    packed_func<R, details::ptr_decay_t<Args>...> call(
        client_base& client, std::string&& func_name, Args&&... args)
    {
        const auto serial_obj =
            serialize_call<Serial, R, Args...>(std::move(func_name), std::forward<Args>(args)...);

        send_to_server<Serial>(serial_obj, client);
        const auto resp_obj = get_server_response<Serial>(client);

        if constexpr (
            details::all_true_v<!(
                std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<
                    Args>>> || std::is_array_v<std::remove_cv_t<std::remove_reference_t<Args>>>)...>)
        {
            // None of the arguments are pointers/arrays
            return serial_adapter<Serial>::template to_packed_func<R,
                details::ptr_decay_t<Args>...>(resp_obj);
        }
        else
        {
            thread_local std::array<std::any, sizeof...(Args)> arg_arr;

            arg_arr = details::populate_arg_arr<Serial, details::ptr_decay_t<Args>...>(resp_obj);

            return serial_adapter<Serial>::template to_packed_func_w_ptr<R,
                details::ptr_decay_t<Args>...>(resp_obj, arg_arr);
        }
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
    std::future<packed_func<R, details::ptr_decay_t<Args>...>> async_call(
        client_base& client, std::string&& func_name, Args&&... args)
    {
        return std::async(call<Serial, R, Args...>, client, std::move(func_name), args...);
    }
#else
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
    packed_func<R, Args...> call(client_base& client, std::string&& func_name, Args&&... args)
    {
        static_assert(
            details::all_true_v<!(
                std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<
                    Args>>> || std::is_array_v<std::remove_cv_t<std::remove_reference_t<Args>>>)...>,
            "Calling functions with pointer arguments is disabled by default as it adds overhead, "
            "please consider refactoring your API to avoid pointers."
            "If you must use pointers, define 'RPC_HPP_ENABLE_POINTERS'.");

        const auto serial_obj =
            serialize_call<Serial, R, Args...>(std::move(func_name), std::forward<Args>(args)...);

        send_to_server<Serial>(serial_obj, client);
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
        client_base& client, std::string&& func_name, Args&&... args)
    {
        return std::async(call<Serial, R, Args...>, client, std::move(func_name), args...);
    }
#endif
} // namespace rpc::client
} // namespace rpc
