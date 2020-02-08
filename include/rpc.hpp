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
    /// (if \p sz > 0, elements are assigned to a default value, as determined by the serialization object)
    ///@param sz Number of elements to populate (default 0)
    /// @return Serial The empty array serial object
    [[nodiscard]] static Serial make_array(size_t sz = 0) noexcept;

    /// @brief Creates a serial object from a string
    ///
    /// @param str String representing the serial object
    /// @return Serial The serial object
    [[nodiscard]] static Serial from_string(std::string_view str);

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

    /// @brief Size of memory buffer to use when encoding arguments (default 4kB)
    constexpr auto DEFAULT_BUFFER_SIZE = 4U * 1024U;

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

        /// @brief Reallocates to a larger buffer
        ///
        /// If it is detected that the size of an arg_buffer is insufficient to store all of the arguemnts,
        /// it can be reallocated to handle more data
        /// @param sz The new size to reallocate to
        void realloc(const size_t sz)
        {
            if (sz <= m_buffer_sz)
            {
                throw std::logic_error("Realloc size is <= current size!");
            }

            std::unique_ptr<uint8_t[]> tmp = std::make_unique<uint8_t[]>(sz);
            memcpy(tmp.get(), m_buffer.get(), m_buffer_sz);
            m_buffer.reset();
            m_buffer = std::move(tmp);
            m_buffer_sz = sz;
        }

        /// @brief Get the size of the arg_buffer
        ///
        /// @return size_t The size of the internal buffer
        size_t size() const noexcept { return m_buffer_sz; }

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

        using P = typename Value::value_type;
        static_assert(!std::is_void_v<P>,
            "Void containers are not supported, either cast to a different type or do the "
            "conversion "
            "manually!");

        if (adapter.is_array())
        {
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
    /// @param buf arg_buffer in which the decoded values are to be stored
    /// @param elem_count The number of elements attached to the pointer (if pointer is array)
    /// @return Value Pointer representing the data decoded from \p obj
    template<typename Serial, typename Value>
    Value decode_pointer_argument(
        const Serial& obj, arg_buffer& buf, size_t* const elem_count) RPC_HPP_EXCEPT
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

        using P = std::remove_cv_t<std::remove_pointer_t<Value>>;
        static_assert(!std::is_void_v<P>,
            "Void pointers are not supported, either cast to a different type or do the conversion "
            "manually!");

        size_t sz = buf.size();

        while (sizeof(P) * adapter.size() >= sz)
        {
            sz += DEFAULT_BUFFER_SIZE;
        }

        if (sz != buf.size())
        {
            buf.realloc(sz);
        }

        if (adapter.is_array())
        {
            // Multi-value pointer (array)
            if constexpr (is_serializable_v<Serial, P>)
            {
                for (size_t i = 0; i < adapter.size(); ++i)
                {
                    const auto value = P::deserialize(adapter[i]);
                    memcpy(&buf.data()[i * sizeof(value)], &value, sizeof(value));
                }
            }
            else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
            {
                for (size_t i = 0; i < adapter.size(); ++i)
                {
                    const auto sub = serial_adapter<Serial>(adapter[i]);
                    const auto value = sub.template get_value<P>();
                    memcpy(&buf.data()[i * sizeof(value)], &value, sizeof(value));
                }
            }
            else
            {
                for (size_t i = 0; i < adapter.size(); ++i)
                {
                    const auto value = deserialize<Serial, P>(adapter[i]);
                    memcpy(&buf.data()[i * sizeof(value)], &value, sizeof(value));
                }
            }

            *elem_count = adapter.size();
            return reinterpret_cast<Value>(buf.data());
        }

        // Single value pointer
        if constexpr (is_serializable_v<Serial, P>)
        {
            new (buf.data()) P(P::deserialize(obj));
        }
        else if constexpr (std::is_same_v<P, char>)
        {
            const auto str = adapter.template get_value<std::string>();
            std::copy(str.begin(), str.end(), reinterpret_cast<Value>(buf.data()));
        }
        else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            new (buf.data()) P(adapter.template get_value<P>());
        }
        else
        {
            new (buf.data()) P(deserialize<Serial, P>(obj));
        }

        return reinterpret_cast<Value>(buf.data());
    }

    /// @brief Decodes a serial object to the proper type
    ///
    /// Checks the type based on the functions parameters and decodes the serialization object accordingly, returning the value
    /// @tparam Serial Serialization type
    /// @tparam Value The type to decode to
    /// @param obj The serialization object to parse
    /// @param buf arg_buffer in which the decoded values are to be stored
    /// @param count The number of elements represented by the serial object (if array)
    /// @param param_num The current index of the function's parameter list being decoded
    /// @return Value The decoded argument value
    template<typename Serial, typename Value>
    Value decode_argument(const Serial& obj, [[maybe_unused]] arg_buffer& buf, size_t* const count,
        unsigned* param_num) RPC_HPP_EXCEPT
    {
#ifdef _DEBUG
        [[maybe_unused]] const auto t_name = typeid(Value).name();
#endif

        *param_num += 1;
        *count = 1;

        if constexpr (std::is_pointer_v<Value>)
        {
            return decode_pointer_argument<Serial, Value>(obj, buf, count);
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
            using Q = std::remove_cv_t<std::remove_reference_t<P>>;

            auto argList = serial_adapter<Serial>::make_array();

            if constexpr (is_serializable_v<Serial, Q>)
            {
                for (const auto& v : val)
                {
                    argList.push_back(P::serialize(v));
                }
            }
            else if constexpr (std::is_same_v<Q, char>)
            {
                argList.push_back(std::string(val));
            }
            else if constexpr (std::is_arithmetic_v<Q> || std::is_same_v<Q, std::string>)
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

/// @brief Class representing a function call
///
/// Stores the function name and parameter values into a serialization object
/// @tparam Serial Type of serial object to utilize
template<typename Serial>
class func_call
{
public:
    ~func_call() = default;
    func_call() = delete;

    explicit func_call(Serial obj) noexcept : m_adapter(std::move(obj)) {}
    func_call(const func_call& other) noexcept : m_adapter(other.m_adapter) {}
    func_call(func_call&& other) noexcept : m_adapter(std::move(other.m_adapter)) {}

    /// @brief Construct a new func_call object given a function name and parameters
    ///
    /// @tparam Args Type(s) of the parameter(s)
    /// @param func_name The name of the function
    /// @param args The value(s) of the parameter(s) to pass to the function
    template<typename... Args>
    func_call(std::string_view func_name, const Args&... args)
    {
        m_adapter.set_value("function", func_name);
        m_adapter.set_value("args", m_adapter.make_array());
        auto& argList = m_adapter.template get_value_ref<Serial>("args");

        (details::encode_arguments(argList, 1, args), ...);
    }

    func_call& operator=(const func_call& other) noexcept
    {
        if (this != &other)
        {
            m_adapter = other.m_adapter;
        }

        return *this;
    }

    func_call& operator=(func_call&& other) noexcept
    {
        if (this != &other)
        {
            m_adapter = std::move(other.m_adapter);
        }

        return *this;
    }

    /// @brief Get the arguments of the func_call, by reference
    ///
    /// @return Serial& Reference to a serial object containing the list of arguments
    [[nodiscard]] Serial& get_args_ref()
    {
        return m_adapter.template get_value_ref<Serial>("args");
    }

    /// @brief Get the args ref object
    ///
    /// @return const Serial& (const) Reference to a serial object containing the list of arguments
    [[nodiscard]] const Serial& get_args_ref() const
    {
        return m_adapter.template get_value_ref<Serial>("args");
    }

    /// @brief Get the name of the func_call
    ///
    /// @return std::string The name of the function represented
    [[nodiscard]] std::string get_func_name() const
    {
        return m_adapter.template get_value<std::string>("function");
    }

    /// @brief Returns the func_call as a string
    ///
    /// @return std::string String representing the func_call
    [[nodiscard]] std::string to_string() const
    {
        return m_adapter.to_string();
    }

    /// @brief Checks if the func_call is valid
    ///
    /// @return true The func_call contains a valid function name and a list of arguments
    /// @return false The func_call does not contain a valid function name and a list of arguments
    operator bool() const noexcept
    {
        try
        {
            const auto tmp1 = m_adapter.template get_value<std::string>("function");
            [[maybe_unused]] const auto tmp2 = m_adapter.template get_value<Serial>("args");
            return !tmp1.empty();
        }
        catch (...)
        {
            return false;
        }
    }

protected:
    serial_adapter<Serial> m_adapter;
};

/// @brief Class representing the result of a function
///
/// Stores the result and parameters into a serialization object
/// @tparam Serial Type of serial object to utilize
template<typename Serial>
class func_result
{
public:
    ~func_result() = default;
    func_result() = delete;

    explicit func_result(Serial obj) noexcept : m_adapter(std::move(obj)) {}
    func_result(const func_result& other) noexcept : m_adapter(other.m_adapter) {}
    func_result(func_result&& other) noexcept : m_adapter(std::move(other.m_adapter)) {}

    /// @brief Construct a new func_result object given a result value and parameter arguments
    ///
    /// @tparam Result Type of the result value
    /// @tparam Args Type(s) of the parameter(s)
    /// @param result The value of the result
    /// @param args The value(s) of the parameter(s)
    template<typename Result, typename... Args>
    func_result(Result result, const Args&... args)
    {
        m_adapter.template set_value<Result>("result", result);
        m_adapter.template set_value<Serial>("args", m_adapter.make_array());
        auto& argList = m_adapter.template get_value_ref<Serial>("args");

        (details::encode_arguments(argList, 1, args), ...);
    }

    /// @brief Creates a function result indicating an error
    ///
    /// When an RPC call fails, this should be called and returned to indicate a failure
    /// @param err_mesg The error message to store
    /// @return func_result A result object with an error contained inside
    static func_result generate_error_result(const std::string& err_mesg)
    {
        serial_adapter<Serial> adapter;
        adapter.template set_value<std::string>("error", err_mesg);
        return func_result<Serial>(adapter.get());
    }

    func_result& operator=(const func_result& other) noexcept
    {
        if (this != &other)
        {
            m_adapter = other.m_adapter;
        }

        return *this;
    }

    func_result& operator=(func_result&& other) noexcept
    {
        if (this != &other)
        {
            m_adapter = std::move(other.m_adapter);
        }

        return *this;
    }

    /// @brief Gets the result value
    ///
    /// The result value represents the return value of a function call
    /// @tparam Result The result's type
    /// @return Result The result's value
    template<typename Result>
    [[nodiscard]] Result get_result() const
    {
        return m_adapter.template get_value<Result>("result");
    }

    /// @brief Gets the argument by index
    ///
    /// Returns the value of the specified argument
    /// @tparam Value The type of value to return (default is as a serial object)
    /// @param n The index of the parameter to get the value of
    /// @return Value The value of the argument
    template<typename Value = Serial>
    [[nodiscard]] Value get_arg(const size_t n) const
    {
        const serial_adapter<Serial> tmp(get_args());
        return tmp.template get_value<Value>(n);
    }

    /// @brief Get the list of arguments as a serial object
    ///
    /// @return Serial The serial object representing the argument list
    [[nodiscard]] Serial get_args() const { return m_adapter.template get_value<Serial>("args"); }

    /// @brief Get the list of arguments, by reference, as a serial object
    ///
    /// @return Serial& The serial object reference representing the argument list
    [[nodiscard]] Serial& get_args_ref()
    {
        return m_adapter.template get_value_ref<Serial>("args");
    }

    /// @brief Get the list of arguments, by const reference, as a serial object
    ///
    /// @return const Serial& The serial object (const) reference representing the argument list
    [[nodiscard]] const Serial& get_args_ref() const
    {
        return m_adapter.template get_value_ref<Serial>("args");
    }

    /// @brief Get the number of arguments stored
    ///
    /// @return size_t The number of arguments
    [[nodiscard]] size_t get_arg_count() const
    {
        const serial_adapter<Serial> tmp(get_args());
        return tmp.size();
    }

    /// @brief Returns the func_result as a string
    ///
    /// @return std::string String representing the func_result
    [[nodiscard]] std::string to_string() const
    {
        return m_adapter.to_string();
    }

    /// @brief Checks if the func_result is valid
    ///
    /// @return true The func_result contains a valid result value
    /// @return false The func_result does not contain a valid result value (possibly contains an "error")
    operator bool() const noexcept
    {
        try
        {
            const auto tmp1 = m_adapter.template get_value<Serial>("result");
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

protected:
    serial_adapter<Serial> m_adapter;
};

/// @brief Function used to callback from, given a function's name and some parameters using a @ref func_call object
///
/// Defines the way in which @ref run_callback will be called for each function
/// @tparam Serial Serialization type
/// @param fc A func_call object containing the function call info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial>
extern func_result<Serial> dispatch(const func_call<Serial>& fc);

// Support for other Windows (x86) calling conventions
#if defined(_WIN32) && !defined(_WIN64)
/// @brief Runs a function as a callback, returning a func_result representing the result
///
/// This function decodes the arguments of \p func based on the values stored in \p fc
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object as a func_result
/// @tparam Serial The serialization object to utilize
/// @tparam R The function return type
/// @tparam Args The function argument types
/// @param func The function to apply
/// @param fc function_call object holding the function info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
func_result<Serial> run_callback(
    std::function<R __stdcall(Args...)> func, const func_call<Serial>& fc) RPC_HPP_EXCEPT
{
    auto& adapter_args = fc.get_args_ref();

    unsigned arg_count = 0;

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter_args[arg_count], arg_buffers[arg_count], &(arg_buffers[arg_count].count),
            &arg_count)...
    };

    std::unique_ptr<func_result<Serial>> fr;

    if constexpr (std::is_void_v<R>)
    {
        std::apply(func, args);
        fr = std::make_unique<func_result<Serial>>(nullptr);
    }
    else
    {
        const auto result = std::apply(func, args);
        fr = std::make_unique<func_result<Serial>>(result);
    }

    auto& argList = fr->get_args_ref();

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

    return *fr;
}

/// @brief Runs a function as a callback, returning a func_result representing the result
///
/// This function decodes the arguments of \p func based on the values stored in \p fc
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object as a func_result
/// @tparam Serial The serialization object to utilize
/// @tparam R The function return type
/// @tparam Args The function argument types
/// @param func The function to apply
/// @param fc function_call object holding the function info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
func_result<Serial> run_callback(R (__stdcall* func)(Args...), const func_call<Serial>& fc) RPC_HPP_EXCEPT
{
    return run_callback(std::function<R __stdcall(Args...)>(func), fc);
}

/// @brief Runs a function as a callback, returning a func_result representing the result
///
/// This function decodes the arguments of \p func based on the values stored in \p fc
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object as a func_result
/// @tparam Serial The serialization object to utilize
/// @tparam R The function return type
/// @tparam Args The function argument types
/// @param func The function to apply
/// @param fc function_call object holding the function info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
func_result<Serial> run_callback(
    std::function<R __fastcall(Args...)> func, const func_call<Serial>& fc) RPC_HPP_EXCEPT
{
    auto& adapter_args = fc.get_args_ref();

    unsigned arg_count = 0;

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter_args[arg_count], arg_buffers[arg_count], &(arg_buffers[arg_count].count),
            &arg_count)...
    };

    std::unique_ptr<func_result<Serial>> fr;

    if constexpr (std::is_void_v<R>)
    {
        std::apply(func, args);
        fr = std::make_unique<func_result<Serial>>(nullptr);
    }
    else
    {
        const auto result = std::apply(func, args);
        fr = std::make_unique<func_result<Serial>>(result);
    }

    auto& argList = fr->get_args_ref();

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

    return *fr;
}

/// @brief Runs a function as a callback, returning a func_result representing the result
///
/// This function decodes the arguments of \p func based on the values stored in \p fc
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object as a func_result
/// @tparam Serial The serialization object to utilize
/// @tparam R The function return type
/// @tparam Args The function argument types
/// @param func The function to apply
/// @param fc function_call object holding the function info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
func_result<Serial> run_callback(R (__fastcall* func)(Args...), const func_call<Serial>& fc) RPC_HPP_EXCEPT
{
    return run_callback(std::function<R __fastcall(Args...)>(func), fc);
}
#endif // defined(_WIN32) && !defined(_WIN64)

// TODO: Find a way to template/lambda this to avoid copy/paste for WIN32

/// @brief Runs a function as a callback, returning a func_result representing the result
///
/// This function decodes the arguments of \p func based on the values stored in \p fc
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object as a func_result
/// @tparam Serial The serialization object to utilize
/// @tparam R The function return type
/// @tparam Args The function argument types
/// @param func The function to apply
/// @param fc function_call object holding the function info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
func_result<Serial> run_callback(
    std::function<R(Args...)> func, const func_call<Serial>& fc) RPC_HPP_EXCEPT
{
    auto& adapter_args = fc.get_args_ref();

    unsigned arg_count = 0;

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter_args[arg_count], arg_buffers[arg_count], &(arg_buffers[arg_count].count),
            &arg_count)...
    };

    std::unique_ptr<func_result<Serial>> fr;

    if constexpr (std::is_void_v<R>)
    {
        std::apply(func, args);
        fr = std::make_unique<func_result<Serial>>(nullptr);
    }
    else
    {
        const auto result = std::apply(func, args);
        fr = std::make_unique<func_result<Serial>>(result);
    }

    auto& argList = fr->get_args_ref();

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

    return *fr;
}

/// @brief Runs a function as a callback, returning a func_result representing the result
///
/// This function decodes the arguments of \p func based on the values stored in \p fc
/// It then applies the actual function and encodes the results to a new serialization object
/// and returns that object as a func_result
/// @tparam Serial The serialization object to utilize
/// @tparam R The function return type
/// @tparam Args The function argument types
/// @param func The function to apply
/// @param fc function_call object holding the function info
/// @return func_result<Serial> Object containing the result and arguments of the function call
template<typename Serial, typename R, typename... Args>
func_result<Serial> run_callback(R (*func)(Args...), const func_call<Serial>& fc) RPC_HPP_EXCEPT
{
    return run_callback(std::function<R(Args...)>(func), fc);
}

/// @brief Entry point for rpc.hpp to dispatch a function call
///
/// Takes the function name and the provided arguments and encodes them into a func_call object
/// then calls the function using dispatch and returns a serialized representation of the result
/// @tparam Serial The serialization object to utilize
/// @tparam Args Type(s) of the function parameter(s)
/// @param func_name String containing the name of the function
/// @param args Variadic list of function parameters
/// @return func_result<Serial> A serial representation of the result of function call
template<typename Serial, typename... Args>
func_result<Serial> run(std::string_view func_name, const Args&... args)
{
    func_call<Serial> fc(func_name, args...);

    try
    {
        return dispatch(fc);
    }
    catch (std::exception& ex)
    {
        return func_result<Serial>::generate_error_result(ex.what());
    }
}

/// @brief Entry point for rpc.hpp to dispatch a function call (asynchronous)
///
/// Takes the function name and the provided arguments and encodes them into a serialization
/// object then calls the function using dispatch and returns a serialized representation of the result
/// @tparam Serial The serialization object to utilize
/// @tparam Args Type(s) of the function parameter(s)
/// @param func_name String containing the name of the function
/// @param args Variadic list of function parameters
/// @return std::future<func_result<Serial>> Future of a serial representation of the result of function call
template<typename Serial, typename... Args>
std::future<func_result<Serial>> async_run(std::string_view func_name, const Args&... args)
{
    return std::async(run<Serial, Args...>, func_name, args...);
}

/// @brief Entry point for rpc.hpp to dispatch a function call, using a serializable string
///
/// Creates a func_call object from the string then calls the function using dispatch and returns a serialized representation of the result
/// @tparam Serial The serialization object to utilize
/// @param obj_str String representing the serialization object containing the function call
/// @return func_result<Serial> A serial representation of the result of function call
template<typename Serial>
func_result<Serial> run_string(std::string_view obj_str)
{
    func_call<Serial> fc(serial_adapter<Serial>::from_string(obj_str));

    try
    {
        return dispatch(fc);
    }
    catch (std::exception& ex)
    {
        return func_result<Serial>::generate_error_result(ex.what());
    }
}

/// @brief Entry point for rpc.hpp to dispatch a function call, using a serializable string (asynchronous)
///
/// Creates a func_call object from the string then calls the function using dispatch and returns a serialized representation of the result
/// @tparam Serial The serialization object to utilize
/// @param obj_str String representing the serialization object containing the function call
/// @return std::future<func_result<Serial>> Future of a serial representation of the result of function call
template<typename Serial>
std::future<func_result<Serial>> async_run_string(std::string_view obj_str)
{
    return std::async(run_string<Serial>, obj_str);
}

/// @brief Entry point for rpc.hpp to dispatch a function call, using a func_call object
///
/// Calls the function using dispatch and returns a serialized representation of the result
/// @tparam Serial The serialization object to utilize
/// @param fc func_call object representing the function call
/// @return func_result<Serial> A serial representation of the result of function call
template<typename Serial>
func_result<Serial> run_object(const func_call<Serial>& fc)
{
    try
    {
        return dispatch(fc);
    }
    catch (std::exception& ex)
    {
        return func_result<Serial>::generate_error_result(ex.what());
    }
}

/// @brief Entry point for rpc.hpp to dispatch a function call, using a func_call object (asynchronous)
///
/// Calls the function using dispatch and returns a serialized representation of the result
/// @tparam Serial The serialization object to utilize
/// @param fc func_call object representing the function call
/// @return std::future<func_result<Serial>> Future of a serial representation of the result of function call
template<typename Serial>
std::future<func_result<Serial>> async_run_object(const func_call<Serial>& fc)
{
    return std::async(run_object<Serial>, fc);
}

/// @brief Packs a function call into a func_call object
///
/// Takes the function name and the provided arguments and encodes them into a func_call
/// object to be passed to a run call or sent to a server
/// @tparam Serial The serialization object to utilize
/// @tparam Args Type(s) of the function parameter(s)
/// @param func_name String containing the name of the function
/// @param args Variadic list of function parameters
/// @return func_call<Serial> A serial representation of the function call
template<typename Serial, typename... Args>
func_call<Serial> package(std::string_view func_name, const Args&... args)
{
    return func_call<Serial>(func_name, args...);
}

/// @brief Packs a function call into a func_call object (asynchronous)
///
/// Takes the function name and the provided arguments and encodes them into a func_call
/// object to be passed to a run call or sent to a server
/// @tparam Serial The serialization object to utilize
/// @tparam Args Type(s) of the function parameter(s)
/// @param func_name String containing the name of the function
/// @param args Variadic list of function parameters
/// @return std::future<func_call<Serial>> Future of a serial representation of the function call
template<typename Serial, typename... Args>
std::future<func_call<Serial>> async_package(std::string_view func_name, const Args&... args)
{
    return std::async(package<Serial, Args...>, func_name, args...);
}
} // namespace rpc
