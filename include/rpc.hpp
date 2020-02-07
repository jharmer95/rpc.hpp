///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.1.0.0
///@date 02-07-2020
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

#include <array>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

/// @brief NOTHROW can be defined at compile-time to add additional 'noexcept' specifiers to functions to
/// potentially improve performance when you are sure that your implentations will not throw an
/// uncaught exception
#if defined(NOTHROW)
#    define RPC_HPP_EXCEPT noexcept
#else
#    define RPC_HPP_EXCEPT
#endif

/// @brief Namespace for rpc.hpp functions and variables
namespace rpc
{
/// @brief Class that allows any serialization type to interact with rpc.hpp
///
/// The user can provide their own implementations of this class for a serialization type or
/// use one of the built-in variants (found in the 'apaters' directory)
/// @tparam Serial Containing class for the serialization library
template<typename Serial>
class serial_adapter
{
public:
    ~serial_adapter() = default;
    serial_adapter() = default;

    explicit serial_adapter(Serial obj) noexcept : m_serial_object(std::move(obj)) {}
    serial_adapter(const serial_adapter& other) noexcept : m_serial_object(other.m_serial_object) {}
    serial_adapter(serial_adapter&& other) noexcept
        : m_serial_object(std::move(other.m_serial_object))
    {
    }

    /// @brief Construct a new serial_adapter object from a string
    ///
    /// Provides a way to provide a string-representation of the serialization object to be converted to
    /// an actual serialization object
    /// @param obj_str String representation of the serial object
    serial_adapter(std::string_view obj_str);

    serial_adapter& operator=(const serial_adapter& other) noexcept
    {
        if (this != &other)
        {
            m_serial_object = other.m_serial_object;
        }

        return *this;
    }

    serial_adapter& operator=(serial_adapter&& other) noexcept
    {
        if (this != &other)
        {
            m_serial_object = std::move(other.m_serial_object);
        }

        return *this;
    }

    /// @brief Get the value of the serial object
    ///
    /// Returns the representation of the object as designated by \p Value, if possible
    /// @tparam Value The type of object to get from the serialization object
    /// @return Value The value of the object
    template<typename Value>
    [[nodiscard]] Value get_value() const;

    /// @brief Get a value from inside the serial object, given the value's name and type
    ///
    /// Returns the representation of the internal object, with \p name as designated by \p Value, if possible
    /// @tparam Value The type of object to get from the serialization object
    /// @param name The name of the internal object to retrieve from the top-level serial object
    /// @return Value The value of the internal object
    template<typename Value>
    [[nodiscard]] Value get_value(const std::string& name) const;

    template<typename Value>
    [[nodiscard]] Value get_value(size_t index) const;

    /// @brief Get the value, by reference, of the serial object
    ///
    /// Returns the representation of the object, by reference, as designated by \p Value, if possible
    /// @tparam Value The type of object to get from the serialization object
    /// @return Value& The value of the object (reference)
    template<typename Value>
    [[nodiscard]] Value& get_value_ref();

    /// @brief Get a value from inside the serial object, by reference, given the value's name and type
    ///
    /// Returns the representation of the internal object, by reference, with \p name, as designated by \p Value, if possible
    /// @tparam Value The type of object to get from the serialization object
    /// @param name The name of the internal object to retrieve from the top-level serial object
    /// @return Value& The value of the internal object (reference)
    template<typename Value>
    [[nodiscard]] Value& get_value_ref(const std::string& name);

    template<typename Value>
    [[nodiscard]] Value& get_value_ref(size_t index);

    /// @brief Get the value, by (const) reference, of the serial object
    ///
    /// Returns the representation of the object, by (const) reference, as designated by \p Value , if possible
    /// @tparam Value The type of object to get from the serialization object
    /// @return Value& The value of the object (const reference)
    template<typename Value>
    [[nodiscard]] const Value& get_value_ref() const;

    /// @brief Get a value from inside the serial object, by (const) reference, given the value's name and type
    ///
    /// Returns the representation of the internal object, by (const) reference, with \p name, as designated by \p Value, if possible
    /// @tparam Value The type of object to get from the serialization object
    /// @param name The name of the internal object to retrieve from the top-level serial object
    /// @return Value& The value of the internal object (const reference)
    template<typename Value>
    [[nodiscard]] const Value& get_value_ref(const std::string& name) const;

    template<typename Value>
    [[nodiscard]] const Value& get_value_ref(size_t index) const;

    /// @brief Sets the value of the serial object
    ///
    /// Assigns the serialization object to the given type and value, if possible
    /// @tparam Value The type of object the serialization object should represent
    /// @param value The value to assign
    template<typename Value>
    void set_value(Value value);

    /// @brief Sets the value of an internal object within the serial object
    ///
    /// Assigns the an object inside the serialization object, by name, creating it if does not already exist
    /// @tparam Value The type of object the internal object should represent
    /// @param name The name of the internal object
    /// @param value The value to assign
    template<typename Value>
    void set_value(const std::string& name, Value value);

    template<typename Value>
    void set_value(size_t index, Value value);

    /// @brief Pushes back a value to the serial object, adding to the array
    ///
    /// Adds a value to the end of the serialization object's array representation, if the object is not already an array,
    /// it is re-assigned as an array, with its previous value becoming the first object in the array
    /// @tparam Value The type of object to be pushed back
    /// @param value The value to push back
    template<typename Value>
    void push_back(Value value);

    /// @brief Alias for @ref push_back
    ///
    /// Calls push_back
    /// @tparam Value The type of object to be pushed back
    /// @param value The value to push back
    template<typename Value>
    void append_value(Value value)
    {
        push_back(value);
    }

    /// @brief Pushes back a value to an internal object in the serial object
    ///
    /// Adds a value to the end of the internal object, given by \p name. If the object does not exist it will be created.
    /// If the object exists and is not already an array, it is re-assigned as an array, with its previous value
    /// becoming the first object in the array
    /// @tparam Value The type of object to be pushed back
    /// @param name The name of the internal object
    /// @param value The value to push back
    template<typename Value>
    void append_value(const std::string& name, Value value);

    /// @brief Converts the serial object to a string
    ///
    /// Creates a string representation of the top-level serialization object (including its internal objects)
    /// @return std::string String representation of the serial object
    [[nodiscard]] std::string to_string() const;

    /// @brief Checks whether the serial object is an array
    ///
    /// Determines if the top-level serialization object has been allocated as an array (an iterable and/or indexed sequence)
    /// @return true Serial object is an array
    /// @return false Serial object is not an array (single value or null)
    [[nodiscard]] bool is_array() const noexcept;

    /// @brief Checks whether the serial object has a value
    ///
    /// Determines if the top-level serialization object is null or empty
    /// @return true Serial object is null or an array with length == 0
    /// @return false Serial object has a value or is array with length > 0
    [[nodiscard]] bool is_empty() const noexcept;

    /// @brief Gets the starting iterator (pointing to the first element) of the serial object
    ///
    /// Returns an iterator type pointing to the first value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialIterator Iterator type for the serialization object (may be a simple pointer)
    /// @return SerialIterator Iterator pointing to the beginning of the serial object
    template<typename SerialIterator>
    [[nodiscard]] SerialIterator begin() noexcept;

    /// @brief Gets the ending iterator (pointing to one past the last element) of the serial object
    ///
    /// Returns an iterator type pointing to one past the last value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialIterator Iterator type for the serialization object (may be a simple pointer)
    /// @return SerialIterator Iterator pointing to one past the end of the serial object
    template<typename SerialIterator>
    [[nodiscard]] SerialIterator end() noexcept;

    /// @brief Gets the starting (const) iterator (pointing to the first element) of the serial object
    ///
    /// Returns an (const) iterator type pointing to the first value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialConstIterator Iterator type for the serialization object (may be a simple pointer)
    /// @return SerialConstIterator Iterator pointing to the beginning of the serial object
    template<typename SerialConstIterator>
    [[nodiscard]] SerialConstIterator begin() const noexcept;

    /// @brief Gets the ending (const) iterator (pointing to one past the last element) of the serial object
    ///
    /// Returns an (const) iterator type pointing to one past the last value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialConstIterator Iterator type for the serialization object (may be a simple pointer)
    /// @return SerialConstIterator Iterator pointing to one past the end of the serial object
    template<typename SerialConstIterator>
    [[nodiscard]] SerialConstIterator end() const noexcept;

    /// @brief Gets the reverse starting iterator (pointing to the last element) of the serial object
    ///
    /// Returns an iterator type pointing to the last value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialReverseIterator Reversed iterator type for the serialization object (may be a pointer, but beware: you will have to use '--')
    /// @return SerialReverseIterator Iterator pointing to the end of the serial object
    template<typename SerialReverseIterator>
    [[nodiscard]] SerialReverseIterator rbegin() noexcept;

    /// @brief Gets the reverse ending iterator (pointing to one past the first element) of the serial object
    ///
    /// Returns an iterator type pointing to one past the first value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialReverseIterator Reversed iterator type for the serialization object (may be a pointer, but beware: you will have to use '--')
    /// @return SerialReverseIterator Iterator pointing to one past the beginning of the serial object
    template<typename SerialReverseIterator>
    [[nodiscard]] SerialReverseIterator rend() noexcept;

    /// @brief Gets the reverse starting (const) iterator (pointing to the last element) of the serial object
    ///
    /// Returns an (const) iterator type pointing to the last value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialConstReverseIterator Reversed iterator type for the serialization object (may be a pointer, but beware: you will have to use '--')
    /// @return SerialConstReverseIterator Iterator pointing to the end of the serial object
    template<typename SerialConstReverseIterator>
    [[nodiscard]] SerialConstReverseIterator rbegin() const noexcept;

    /// @brief Gets the reverse ending (const) iterator (pointing to one past the first element) of the serial object
    ///
    /// Returns an (const) iterator type pointing to one past the first value of the top-level serialization object.
    /// If the object is not an array, the iterator simply points to the value
    /// @tparam SerialConstReverseIterator Reversed iterator type for the serialization object (may be a pointer, but beware: you will have to use '--')
    /// @return SerialConstReverseIterator Iterator pointing to one past the beginning of the serial object
    template<typename SerialConstReverseIterator>
    [[nodiscard]] SerialConstReverseIterator rend() const noexcept;

    /// @brief Get the actual serial object
    ///
    /// Returns the serialization object without doing any conversion
    /// @return Serial The contained serial object
    [[nodiscard]] Serial get() const { return m_serial_object; }

    /// @brief Get the size of the serial object
    ///
    /// Returns the length of the top-level serialization object if it is an array, if not returns 1 or 0
    /// @return size_t Size of the serial object
    [[nodiscard]] size_t size() const noexcept;

    /// @brief Retrieves an internal object by index
    ///
    /// Returns the specified internal object within the top-level serialization object, provided by an index, \p n
    /// @param n The index to get the object from
    /// @return Serial& Reference to the serial object found at index \p n
    [[nodiscard]] Serial& operator[](size_t n);

    /// @brief Retrieves an internal object by index
    ///
    /// Returns the specified internal object within the top-level serialization object, provided by an index, \p n
    /// @param n The index to get the object from
    /// @return Serial& Const reference to the serial object found at index \p n
    [[nodiscard]] const Serial& operator[](size_t n) const;

    /// @brief Create an empty array, according to the serial object's type
    ///
    /// Uses the serialization object's methods to create an object consisting of an array of length \p sz
    // (if \p sz > 0, elements are assigned to a default value, as determined by the serialization object)
    ///@param sz Number of elements to populate (default 0)
    /// @return Serial The empty array serial object
    [[nodiscard]] static Serial make_array(size_t sz = 0) noexcept;

protected:
    Serial m_serial_object{};
};

/// @brief Serializes an object to the desired format
///
/// Takes an object of type \c Value and converts it to a serialization object of type \c Serial
/// @tparam Serial Serialization type
/// @tparam Value Type of object to be converted
/// @param val The value to be serialized
/// @return Serial Serialized version of the object
template<typename Serial, typename Value>
[[nodiscard]] Serial serialize(const Value& val) RPC_HPP_EXCEPT;

/// @brief De-serializes an object from the desired format
///
/// Takes a serial object of type \c Serial and converts it back to an object of type \c Value
/// @tparam Serial Serialization type
/// @tparam Value Type of object to be converted to
/// @param ser The value to be de-serialized
/// @return Value De-serialized version of the serial object
template<typename Serial, typename Value>
[[nodiscard]] Value deserialize(const Serial& ser) RPC_HPP_EXCEPT;

/// @brief Internal namespace for @ref rpc. Not to be used by users
namespace details
{
    using namespace std::string_literals;

    /// @brief Size of memory buffer to use when encoding arguments (default 64kB)
    constexpr auto DEFAULT_BUFFER_SIZE = 64U * 1024U;

    /// @brief Class used to store encoded arguments in memory
    class arg_buffer
    {
    public:
        ~arg_buffer() = default;
        arg_buffer(size_t buffer_size = DEFAULT_BUFFER_SIZE)
            : m_buffer_sz(buffer_size), m_buffer(std::make_unique<uint8_t[]>(m_buffer_sz))
        {
        }

        // prevent implicit copy of arg_buffer
        arg_buffer(const arg_buffer&) = delete;
        arg_buffer& operator=(const arg_buffer&) = delete;

        // arg_buffer cannot be implicitly compared
        bool operator==(const arg_buffer&) = delete;

        arg_buffer(arg_buffer&& other) noexcept
            : count(other.count), m_buffer_sz(other.m_buffer_sz),
              m_buffer(std::move(other.m_buffer))
        {
            other.count = 0;
            other.m_buffer_sz = 0;
        }

        arg_buffer& operator=(arg_buffer&& other) noexcept
        {
            if (this != &other)
            {
                m_buffer.reset();
                m_buffer_sz = other.m_buffer_sz;
                m_buffer = std::move(other.m_buffer);
                count = other.count;
                other.m_buffer_sz = 0;
                other.count = 0;
            }

            return *this;
        }

        /// @brief Gets a raw pointer to the underlying data
        ///
        /// Returns a pointer to the bytes stored in the buffer
        /// @return uint8_t* Pointer to the buffer's data
        [[nodiscard]] uint8_t* data() const { return m_buffer.get(); }

        /// @brief The number of encoded arguments currently stored in the buffer
        size_t count = 0;

    protected:
        size_t m_buffer_sz;
        std::unique_ptr<uint8_t[]> m_buffer;
    };

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
    struct is_container : std::integral_constant<bool,
                              has_size<C>::value && has_begin<C>::value && has_end<C>::value>
    {
    };

    /// @brief Helper variable for @ref is_container
    ///
    /// @tparam C Type to check
    template<typename C>
    inline constexpr bool is_container_v = is_container<C>::value;

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

        template<size_t i>
        struct arg
        {
            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        };
    };

    /// @brief Helper variable for getting parameter count from @ref function_traits
    ///
    /// @tparam R The return type of the function
    /// @tparam Args The argument types of the function
    template<typename R, typename... Args>
    inline constexpr size_t function_param_count_v =
        function_traits<std::function<R(Args...)>>::nargs;

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
    using function_args_t =
        typename function_traits<std::function<R(Args...)>>::template arg<i>::type;

    template<typename T, size_t sz>
    inline constexpr size_t get_array_len(T (&)[sz])
    {
        return sz;
    }

    /// @brief Decodes a container-like type from a serial object
    ///
    /// This is called when the type \c Value has been determined to be a container.
    /// This function fills the container, based on the contents of \p obj
    /// @tparam Serial Serialization type
    /// @tparam Value The container-like type to decode to
    /// @param obj The serialization object to parse
    /// @param elem_count The number of elements placed into the container
    /// @return Value A container holding the elements represented by \p obj
    template<typename Serial, typename Value>
    Value decode_container_argument(const Serial& obj, size_t* elem_count) RPC_HPP_EXCEPT
    {
#ifdef _DEBUG
        [[maybe_unused]] const auto t_name = typeid(Value).name();
#endif

        Value container;
        *elem_count = 0;
        serial_adapter<Serial> adapter(obj);

        if (adapter.is_array())
        {
            // Multi-value container (array)
            using P = typename Value::value_type;
            static_assert(!std::is_void_v<P>,
                "Void containers are not supported, either cast to a different type or do the "
                "conversion manually!");

            if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
            {
                for (const auto& val : adapter.get())
                {
                    container.push_back(val);
                }
            }
            else if constexpr (is_container_v<P>)
            {
                for (const Serial& ser : adapter)
                {
                    size_t ncount = 0;
                    container.push_back(decode_container_argument<Serial, P>(ser, &ncount));
                    *elem_count += ncount;
                }
            }
            else if constexpr (is_serializable_v<Serial, P>)
            {
                for (const auto& ser : adapter.get())
                {
                    container.push_back(P::deserialize(ser));
                }
            }
            else
            {
                for (const auto& ser : adapter.get())
                {
                    container.push_back(deserialize<Serial, P>(ser));
                }
            }

            if (*elem_count == 0)
            {
                *elem_count = container.size();
            }

            return container;
        }

        // Single value container
        using P = typename Value::value_type;
        static_assert(!std::is_void_v<P>,
            "Void containers are not supported, either cast to a different type or do the "
            "conversion "
            "manually!");

        if constexpr (is_serializable_v<Serial, P>)
        {
            container.push_back(P::deserialize(obj));
        }
        else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            container.push_back(adapter.template get_value<P>());
        }
        else if constexpr (std::is_same_v<P, char>)
        {
            const auto str = adapter.template get_value<std::string>();
            std::copy(str.begin(), str.end(), std::back_inserter(container));
        }
        else if constexpr (is_container_v<P>)
        {
            size_t ncount = 0;
            container.push_back(decode_container_argument<Serial, P>(obj, &ncount));
            *elem_count += ncount;
        }
        else
        {
            container.push_back(deserialize<Serial, P>(obj));
        }

        if (*elem_count == 0)
        {
            *elem_count = 1;
        }

        return container;
    }

    /// @brief Decodes a pointer type from a serial object
    ///
    /// This is called when the type \c Value has been determined to be a pointer.
    /// This function populates the value pointed to based on the contents of \p obj
    /// @tparam Serial Serialization type
    /// @tparam Value The pointer type to decode to
    /// @param obj The serialization object to parse
    /// @param buf Pointer to the buffer in which the decoded values are to be stored
    /// @param elem_count The number of elements attached to the pointer (if pointer is array)
    /// @return Value Pointer representing the data decoded from \p obj
    template<typename Serial, typename Value>
    Value decode_pointer_argument(
        const Serial& obj, uint8_t* const buf, size_t* const elem_count) RPC_HPP_EXCEPT
    {
#ifdef _DEBUG
        [[maybe_unused]] const auto t_name = typeid(Value).name();
#endif

        serial_adapter<Serial> adapter(obj);

        if (adapter.is_empty())
        {
            *elem_count = 0;
            return nullptr;
        }

        if (adapter.is_array())
        {
            // Multi-value pointer (array)
            using P = std::remove_cv_t<std::remove_pointer_t<Value>>;
            static_assert(!std::is_void_v<P>,
                "Void pointers are not supported, either cast to a different type or do the "
                "conversion "
                "manually!");

            if constexpr (is_serializable_v<Serial, P>)
            {
                for (size_t i = 0; i < adapter.size(); ++i)
                {
                    const auto value = P::deserialize(adapter[i]);
                    memcpy(&buf[i * sizeof(value)], &value, sizeof(value));
                }
            }
            else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
            {
                for (size_t i = 0; i < adapter.size(); ++i)
                {
                    const auto sub = serial_adapter<Serial>(adapter[i]);
                    const auto value = sub.template get_value<P>();
                    memcpy(&buf[i * sizeof(value)], &value, sizeof(value));
                }
            }
            else
            {
                for (size_t i = 0; i < adapter.size(); ++i)
                {
                    const auto value = deserialize<Serial, P>(adapter[i]);
                    memcpy(&buf[i * sizeof(value)], &value, sizeof(value));
                }
            }

            *elem_count = adapter.size();
            return reinterpret_cast<Value>(buf);
        }

        // Single value pointer
        using P = std::remove_cv_t<std::remove_pointer_t<Value>>;
        static_assert(!std::is_void_v<P>,
            "Void pointers are not supported, either cast to a different type or do the conversion "
            "manually!");

        if constexpr (is_serializable_v<Serial, P>)
        {
            new (buf) P(P::deserialize(obj));
        }
        else if constexpr (std::is_same_v<P, char>)
        {
            const auto str = adapter.template get_value<std::string>();
            std::copy(str.begin(), str.end(), reinterpret_cast<Value>(buf));
        }
        else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            new (buf) P(adapter.template get_value<P>());
        }
        else
        {
            new (buf) P(deserialize<Serial, P>(obj));
        }

        return reinterpret_cast<Value>(buf);
    }

    /// @brief Decodes a serial object to the proper type
    ///
    /// Checks the type based on the functions parameters and decodes the serialization object accordingly, returning the value
    /// @tparam Serial Serialization type
    /// @tparam Value The type to decode to
    /// @param obj The serialization object to parse
    /// @param buf Pointer to the buffer in which the decoded values are to be stored
    /// @param count The number of elements represented by the serial object (if array)
    /// @param param_num The current index of the function's parameter list being decoded
    /// @return Value The decoded argument value
    template<typename Serial, typename Value>
    Value decode_argument(const Serial& obj, [[maybe_unused]] uint8_t* const buf,
        size_t* const count, unsigned* param_num) RPC_HPP_EXCEPT
    {
#ifdef _DEBUG
        [[maybe_unused]] const auto t_name = typeid(Value).name();
#endif

        *param_num += 1;
        *count = 1;

        if constexpr (std::is_pointer_v<Value>)
        {
            auto val = decode_pointer_argument<Serial, Value>(obj, buf, count);
            return val;
        }
        else if constexpr (is_serializable_v<Serial, Value>)
        {
            return Value::deserialize(obj);
        }
        else if constexpr (std::is_arithmetic_v<Value> || std::is_same_v<Value, std::string>)
        {
            const serial_adapter<Serial> adapter(obj);
            return adapter.template get_value<Value>();
        }
        else if constexpr (is_container_v<Value>)
        {
            return decode_container_argument<Serial, Value>(obj, count);
        }
        else if constexpr (std::is_array_v<Value>)
        {
            const serial_adapter<Serial> adapter(obj);
            Value arr;

            for (size_t i = 0; i < get_array_len(arr); ++i)
            {
                arr[i] = adapter.get_value(i);
            }

            return arr;
        }
        else
        {
            return deserialize<Serial, Value>(obj);
        }
    }

    /// @brief Encodes parameter values to a serial object
    ///
    /// Takes the value and type of the function parameter to create serialization objects representing each argument
    /// @tparam Serial The serialization object to encode to
    /// @tparam Value The type to encode from
    /// @param obj Reference to the resulting encoded object
    /// @param count Number of values stored in \p val
    /// @param val The value to be encoded from
    template<typename Serial, typename Value>
    void encode_arguments(
        Serial& obj, [[maybe_unused]] const size_t count, const Value& val) RPC_HPP_EXCEPT
    {
#ifdef _DEBUG
        [[maybe_unused]] const auto t_name = typeid(Value).name();
#endif

#ifdef RPC_HPP_SERIAL_MIN
        if constexpr (!std::is_pointer_v<Value> && !std::is_reference_v<Value>)
        {
            // Pass-by-value does not need to be re-serialized
            return;
        }
        else if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<Value>>>)
        {
            // Const ref and pointer to const do not need to be re-serialized
            return;
        }
#endif

        serial_adapter<Serial> adapter(obj);

        if constexpr (std::is_pointer_v<Value>)
        {
            if (val == nullptr)
            {
                adapter.push_back(Serial{});
            }
            else
            {
                using P = std::remove_cv_t<std::remove_pointer_t<Value>>;

                for (size_t i = 0; i < count; ++i)
                {
                    if constexpr (is_serializable_v<Serial, P>)
                    {
                        adapter.push_back(P::serialize(val[i]));
                    }
                    else if constexpr (std::is_same_v<P, char>)
                    {
                        if (val[0] == '\0')
                        {
                            adapter.push_back(""s);
                        }
                        else
                        {
                            adapter.push_back(std::string(val));
                        }
                    }
                    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
                    {
                        adapter.push_back(val[i]);
                    }
                    else
                    {
                        adapter.push_back(serialize<Serial, P>(val[i]));
                    }
                }
            }
        }
        else if constexpr (std::is_arithmetic_v<Value> || std::is_same_v<Value, std::string>)
        {
            adapter.push_back(val);
        }
        else if constexpr (is_container_v<Value>)
        {
            using P = typename Value::value_type;
            auto argList = serial_adapter<Serial>::make_array();

            if constexpr (std::is_same_v<P, std::string>)
            {
                std::copy(val.begin(), val.end(), std::back_inserter(adapter));
            }
            else if constexpr (is_container_v<P>)
            {
                for (const auto& c : val)
                {
                    encode_arguments<P>(argList, c.size(), c);
                    adapter.push_back(argList);
                }
            }
            else
            {
                for (const auto& v : val)
                {
                    if constexpr (is_serializable_v<Serial, P>)
                    {
                        argList.push_back(P::serialize(v));
                    }
                    else if constexpr (std::is_same_v<P, char>)
                    {
                        argList.push_back(std::string(val, count));
                    }
                    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
                    {
                        argList.push_back(v);
                    }
                    else
                    {
                        argList.push_back(serialize<Serial, P>(v));
                    }
                }

                adapter.push_back(argList);
            }
        }
        else if constexpr (std::is_array_v<Value>)
        {
            using P = decltype(*val);

            auto argList = serial_adapter<Serial>::make_array();

            if constexpr (is_serializable_v<Serial, P>)
            {
                for (const auto& v : val)
                {
                    argList.push_back(P::serialize(v));
                }
            }
            else if constexpr (std::is_same_v<P, char>)
            {
                argList.push_back(std::string(val));
            }
            else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
            {
                for (const auto& v : val)
                {
                    argList.push_back(v);
                }
            }
            else
            {
                for (const auto& v : val)
                {
                    argList.push_back(serialize<Serial>(v));
                }
            }

            adapter.push_back(argList);
        }
        else
        {
            if constexpr (is_serializable_v<Serial, Value>)
            {
                adapter.push_back(Value::serialize(val));
            }
            else
            {
                adapter.push_back(serialize<Serial, Value>(val));
            }
        }

        obj = adapter.get();
    }

    /// @brief Default implementation of meta-programming function
    ///
    /// @tparam F Function type
    /// @tparam Ts Tuple types (generic)
    /// @tparam Is Index sequence to iterate over
    /// @param tuple Tuple to iterate over
    /// @param func Function to apply to each value
    template<typename F, typename... Ts, size_t... Is>
    void for_each_tuple(
        const std::tuple<Ts...>& tuple, const F& func, std::index_sequence<Is...>) RPC_HPP_EXCEPT
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
    void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func) RPC_HPP_EXCEPT
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

    /// @brief Frees a dynamically allocated @ref arg_buffer
    ///
    /// Due to the way arg_buffers work, they must be constructed using a 'placement new' and therefore do not
    /// have any helpful RAII method of being cleaned up.
    /// This function therefore, must be called to ensure their memory does not leak
    /// @tparam T Pointer type of data contained in \p buffer
    /// @param buffer The arg_buffer to free
    template<typename T>
    void free_buffer(const arg_buffer& buffer) RPC_HPP_EXCEPT
    {
        auto ptr = reinterpret_cast<T>(buffer.data());

        for (size_t i = 0; i < buffer.count; ++i)
        {
            using X = std::remove_reference_t<decltype(*ptr)>;
            ptr[i].~X();
        }
    }
} // namespace details

/// @brief Function used to callback from, given a function's name and some parameters using a serial object
///
/// Defines the way in which @ref run_callback will be called for each function
/// @tparam Serial Serialization type
/// @param func_name Name of the function to call, represented in a string
/// @param obj A serialization object containing the parameters to run the callback with
/// @return std::string The result of the function call, including the return value, and any altered parameters, converted to a string
template<typename Serial>
extern Serial dispatch(const serial_adapter<Serial>& adapter);

// Support for other Windows (x86) calling conventions
#if defined(_WIN32) && !defined(_WIN64)
///@brief Runs a function as a callback, returning a serial object representing the result
///
/// This function decodes the arguments of \p func based on the serialization adapter \p adapter.
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object
///@tparam Serial The serialization object to utilize
///@tparam R The function return type
///@tparam Args The function argument types
///@param func The function to apply
///@param adapter The serial adapter containing the function arguments
///@return Serial Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
Serial run_callback(
    std::function<R __stdcall(Args...)> func, const serial_adapter<Serial>& adapter) RPC_HPP_EXCEPT
{
    auto& adapter_args = adapter.template get_value_ref<Serial>("args");
    unsigned arg_count = 0;

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter_args[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
            &arg_count)...
    };

    serial_adapter<Serial> retSer;

    if constexpr (std::is_void_v<R>)
    {
        std::apply(func, args);
        retSer.set_value("result", nullptr);
    }
    else
    {
        const auto result = std::apply(func, args);
        retSer.set_value("result", result);
    }

    retSer.set_value("args", retSer.make_array());
    auto& argList = retSer.template get_value_ref<Serial>("args");

    arg_count = 0;

    // TODO: Use enum 'serialization_level' to determine the encoding

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.get();
}

///@brief Runs a function as a callback, returning a serial object representing the result
///
/// This function decodes the arguments of \p func based on the serialization adapter \p adapter.
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object
///@tparam Serial The serialization object to utilize
///@tparam R The function return type
///@tparam Args The function argument types
///@param func The function to apply
///@param adapter The serial adapter containing the function arguments
///@return Serial Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
Serial run_callback(
    R(__stdcall* func)(Args...), const serial_adapter<Serial>& adapter) RPC_HPP_EXCEPT
{
    return run_callback(std::function<R(Args...)>(func), adapter);
}

///@brief Runs a function as a callback, returning a serial object representing the result
///
/// This function decodes the arguments of \p func based on the serialization adapter \p adapter.
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object
///@tparam Serial The serialization object to utilize
///@tparam R The function return type
///@tparam Args The function argument types
///@param func The function to apply
///@param adapter The serial adapter containing the function arguments
///@return Serial Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
Serial run_callback(
    std::function<R __fastcall(Args...)> func, const serial_adapter<Serial>& adapter) RPC_HPP_EXCEPT
{
    auto& adapter_args = adapter.template get_value_ref<Serial>("args");
    unsigned arg_count = 0;

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter_args[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
            &arg_count)...
    };

    serial_adapter<Serial> retSer;

    if constexpr (std::is_void_v<R>)
    {
        std::apply(func, args);
        retSer.set_value("result", nullptr);
    }
    else
    {
        const auto result = std::apply(func, args);
        retSer.set_value("result", result);
    }

    retSer.set_value("args", retSer.make_array());
    auto& argList = retSer.template get_value_ref<Serial>("args");

    arg_count = 0;

    // TODO: Use enum 'serialization_level' to determine the encoding

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.get();
}

///@brief Runs a function as a callback, returning a serial object representing the result
///
/// This function decodes the arguments of \p func based on the serialization adapter \p adapter.
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object
///@tparam Serial The serialization object to utilize
///@tparam R The function return type
///@tparam Args The function argument types
///@param func The function to apply
///@param adapter The serial adapter containing the function arguments
///@return Serial Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
Serial run_callback(
    R(__fastcall* func)(Args...), const serial_adapter<Serial>& adapter) RPC_HPP_EXCEPT
{
    return run_callback(std::function<R(Args...)>(func), adapter);
}
#endif // defined(_WIN32) && !defined(_WIN64)

// TODO: Find a way to template/lambda this to avoid copy/paste for WIN32

///@brief Runs a function as a callback, returning a serial object representing the result
///
/// This function decodes the arguments of \p func based on the serialization adapter \p adapter.
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object
///@tparam Serial The serialization object to utilize
///@tparam R The function return type
///@tparam Args The function argument types
///@param func The function to apply
///@param adapter The serial adapter containing the function arguments
///@return Serial Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
Serial run_callback(
    std::function<R(Args...)> func, const serial_adapter<Serial>& adapter) RPC_HPP_EXCEPT
{
    auto& adapter_args = adapter.template get_value_ref<Serial>("args");
    unsigned arg_count = 0;

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter_args[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
            &arg_count)...
    };

    serial_adapter<Serial> retSer;

    if constexpr (std::is_void_v<R>)
    {
        std::apply(func, args);
        retSer.set_value("result", nullptr);
    }
    else
    {
        const auto result = std::apply(func, args);
        retSer.set_value("result", result);
    }

    retSer.set_value("args", retSer.make_array());
    auto& argList = retSer.template get_value_ref<Serial>("args");

    arg_count = 0;

    // TODO: Use enum 'serialization_level' to determine the encoding

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.get();
}

///@brief Runs a function as a callback, returning a serial object representing the result
///
/// This function decodes the arguments of \p func based on the serialization adapter \p adapter.
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object
///@tparam Serial The serialization object to utilize
///@tparam R The function return type
///@tparam Args The function argument types
///@param func The function to apply
///@param adapter The serial adapter containing the function arguments
///@return Serial Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
Serial run_callback(R (*func)(Args...), const serial_adapter<Serial>& adapter) RPC_HPP_EXCEPT
{
    return run_callback(std::function<R(Args...)>(func), adapter);
}

template<typename Serial, typename... Args>
Serial run(std::string_view func_name, const Args&... args)
{
    serial_adapter<Serial> adapter;
    adapter.set_value("function", func_name);
    adapter.set_value("args", adapter.make_array());
    auto& argList = adapter.template get_value_ref<Serial>("args");

    size_t arg_count = 0;

    (details::encode_arguments(argList, 1, args), ...);

    try
    {
        return dispatch(adapter);
    }
    catch (std::exception& ex)
    {
        serial_adapter<Serial> result;
        result.set_value("error", ex.what());
        return result.get();
    }
}

template<typename Serial>
Serial run_string(std::string_view obj_str)
{
    serial_adapter<Serial> adapter(obj_str);

    try
    {
        return dispatch(adapter);
    }
    catch (std::exception& ex)
    {
        serial_adapter<Serial> result;
        result.set_value("error", ex.what());
        return result.get();
    }
}
} // namespace rpc
