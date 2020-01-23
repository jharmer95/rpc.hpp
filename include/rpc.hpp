///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.1.0.0
///@date 01-21-2020
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
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

// NOTHROW can be defined at compile-time to add additional 'noexcept' specifiers to functions to
// potentially improve performance when you are sure that your implentations will not throw an
// uncaught exception
#if defined(NOTHROW)
#    define RPC_HPP_EXCEPT noexcept
#else
#    define RPC_HPP_EXCEPT
#endif

namespace rpc
{
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

    template<typename Value>
    [[nodiscard]] Value get_value() const;

    template<typename Value>
    [[nodiscard]] Value get_value(const std::string& name) const;

    template<typename Value>
    [[nodiscard]] Value& get_value_ref();

    template<typename Value>
    [[nodiscard]] Value& get_value_ref(const std::string& name);

    template<typename Value>
    [[nodiscard]] Value& get_value_ref() const;

    template<typename Value>
    [[nodiscard]] Value& get_value_ref(const std::string& name) const;

    template<typename Value>
    void set_value(Value value);

    template<typename Value>
    void set_value(const std::string& name, Value value);

    template<typename Value>
    void push_back(Value value);

    template<typename Value>
    void append_value(Value value)
    {
        push_back(value);
    }

    template<typename Value>
    void append_value(const std::string& name, Value value);

    [[nodiscard]] std::string to_string() const;

    [[nodiscard]] bool is_array() const noexcept;

    [[nodiscard]] bool is_empty() const noexcept;

    template<typename SerialIterator>
    [[nodiscard]] SerialIterator begin() noexcept;

    template<typename SerialIterator>
    [[nodiscard]] SerialIterator end() noexcept;

    template<typename SerialConstIterator>
    [[nodiscard]] SerialConstIterator begin() const noexcept;

    template<typename SerialConstIterator>
    [[nodiscard]] SerialConstIterator end() const noexcept;

    template<typename SerialReverseIterator>
    [[nodiscard]] SerialReverseIterator rbegin() noexcept;

    template<typename SerialReverseIterator>
    [[nodiscard]] SerialReverseIterator rend() noexcept;

    template<typename SerialConstReverseIterator>
    [[nodiscard]] SerialConstReverseIterator rbegin() const noexcept;

    template<typename SerialConstReverseIterator>
    [[nodiscard]] SerialConstReverseIterator rend() const noexcept;

    [[nodiscard]] Serial get() const { return m_serial_object; }

    [[nodiscard]] size_t size() const noexcept;

    [[nodiscard]] Serial operator[](size_t n) const;

    [[nodiscard]] static Serial make_array() noexcept;

protected:
    Serial m_serial_object{};
};

template<typename Serial>
extern std::string dispatch(const std::string& func_name, const Serial& obj);

template<typename Serial, typename Value>
[[nodiscard]] Serial serialize(const Value&) RPC_HPP_EXCEPT;

template<typename Serial, typename Value>
[[nodiscard]] Value deserialize(const Serial&) RPC_HPP_EXCEPT;

namespace details
{
    using namespace std::string_literals;

    constexpr auto DEFAULT_BUFFER_SIZE = 64U * 1024U;

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

        // // explicit copy of arg_buffer
        // arg_buffer clone() const
        // {
        //     arg_buffer tmp(m_buffer_sz);
        //     tmp.count = count;
        //     std::copy(m_buffer.get(), m_buffer.get() + m_buffer_sz, tmp.m_buffer.get());
        //     return tmp;
        // }

        // // explicit compare of arg_buffer
        // bool compare(const arg_buffer& other) const
        // {
        //     if (count != other.count)
        //     {
        //         return false;
        //     }

        //     if (other.m_buffer_sz > m_buffer_sz)
        //     {
        //         if (other.data()[m_buffer_sz] != 0)
        //         {
        //             return false;
        //         }

        //         const auto result = memcmp(data(), other.data(), m_buffer_sz);

        //         if (result != 0)
        //         {
        //             return false;
        //         }

        //         for (size_t i = m_buffer_sz; i < other.m_buffer_sz; ++i)
        //         {
        //             if (other.data()[i] != 0)
        //             {
        //                 return false;
        //             }
        //         }
        //     }
        //     else
        //     {
        //         if (data()[other.m_buffer_sz] != 0)
        //         {
        //             return false;
        //         }

        //         const auto result = memcmp(data(), other.data(), other.m_buffer_sz);

        //         if (result != 0)
        //         {
        //             return false;
        //         }

        //         for (size_t i = other.m_buffer_sz; i < m_buffer_sz; ++i)
        //         {
        //             if (data()[i] != 0)
        //             {
        //                 return false;
        //             }
        //         }
        //     }

        //     return true;
        // }

        [[nodiscard]] uint8_t* data() const { return m_buffer.get(); }

        size_t count = 0;

    protected:
        size_t m_buffer_sz;
        std::unique_ptr<uint8_t[]> m_buffer;
    };

    template<typename, typename T>
    struct is_serializable_base
    {
        static_assert(std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type");
    };

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

    template<typename, typename T>
    struct is_deserializable_base
    {
        static_assert(std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type");
    };

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

    template<typename Serial, typename Value>
    struct is_serializable : std::integral_constant<bool,
                                 is_serializable_base<Value, Serial(const Value&)>::value
                                     && is_deserializable_base<Value, Value(const Serial&)>::value>
    {
    };

    template<typename Serial, typename Value>
    inline constexpr bool is_serializable_v = is_serializable<Serial, Value>::value;

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

    template<typename C>
    struct is_container : std::integral_constant<bool,
                              has_size<C>::value && has_begin<C>::value && has_end<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_container_v = is_container<C>::value;

    template<typename T>
    struct function_traits;

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

    template<typename R, typename... Args>
    inline constexpr size_t function_param_count_v =
        function_traits<std::function<R(Args...)>>::nargs;

    template<typename R, typename... Args>
    using function_result_t = typename function_traits<std::function<R(Args...)>>::type;

    template<size_t i, typename R, typename... Args>
    using function_args_t =
        typename function_traits<std::function<R(Args...)>>::template arg<i>::type;

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
        else
        {
            return deserialize<Serial, Value>(obj);
        }
    }

    template<typename Serial, typename Value>
    void encode_arguments(
        Serial& obj, [[maybe_unused]] const size_t count, const Value& val) RPC_HPP_EXCEPT
    {
#ifdef _DEBUG
        [[maybe_unused]] const auto t_name = typeid(Value).name();
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

    template<typename F, typename... Ts, size_t... Is>
    void for_each_tuple(
        const std::tuple<Ts...>& tuple, const F& func, std::index_sequence<Is...>) RPC_HPP_EXCEPT
    {
        using expander = int[];
        (void)expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func) RPC_HPP_EXCEPT
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

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

// Support for other Windows (x86) calling conventions
#if defined(_WIN32) && !defined(_WIN64)
template<typename Serial, typename R, typename... Args>
std::string run_callback(const Serial& obj, std::function<R __stdcall(Args...)> func) RPC_HPP_EXCEPT
{
    unsigned arg_count = 0;
    serial_adapter<Serial> adapter(obj);

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
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

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.to_string();
}

template<typename Serial, typename R, typename... Args>
std::string run_callback(const Serial& obj, R(__stdcall* func)(Args...)) RPC_HPP_EXCEPT
{
    return run_callback(obj, std::function<R __stdcall(Args...)>(func));
}

template<typename Serial, typename R, typename... Args>
std::string run_callback(
    const Serial& obj, std::function<R __fastcall(Args...)> func) RPC_HPP_EXCEPT
{
    unsigned arg_count = 0;
    serial_adapter<Serial> adapter(obj);

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
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

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.to_string();
}

template<typename Serial, typename R, typename... Args>
std::string run_callback(const Serial& obj, R(__fastcall* func)(Args...)) RPC_HPP_EXCEPT
{
    return run_callback(obj, std::function<R __fastcall(Args...)>(func));
}

#    if !defined(__MINGW32__)
template<typename Serial, typename R, typename... Args>
std::string run_callback(
    const Serial& obj, std::function<R __vectorcall(Args...)> func) RPC_HPP_EXCEPT
{
    unsigned arg_count = 0;
    serial_adapter<Serial> adapter(obj);

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
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

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.to_string();
}

template<typename Serial, typename R, typename... Args>
std::string run_callback(const Serial& obj, R(__vectorcall* func)(Args...)) RPC_HPP_EXCEPT
{
    return run_callback(obj, std::function<R __vectorcall(Args...)>(func));
}
#    endif
#endif

// TODO: Find a way to template/lambda this to avoid copy/paste for WIN32
template<typename Serial, typename R, typename... Args>
std::string run_callback(const Serial& obj, std::function<R(Args...)> func) RPC_HPP_EXCEPT
{
    unsigned arg_count = 0;
    serial_adapter<Serial> adapter(obj);

    std::array<details::arg_buffer, details::function_param_count_v<R, Args...>> arg_buffers;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        details::decode_argument<Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter[arg_count], arg_buffers[arg_count].data(), &(arg_buffers[arg_count].count),
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

    details::for_each_tuple(args, [&argList, &arg_buffers, &arg_count](const auto& x) {
        details::encode_arguments(argList, arg_buffers[arg_count].count, x);

        using P = std::remove_cv_t<std::remove_reference_t<decltype(x)>>;

        if constexpr (std::is_pointer_v<P> && std::is_class_v<std::remove_pointer_t<P>>)
        {
            details::free_buffer<P>(arg_buffers[arg_count]);
        }

        ++arg_count;
    });

    return retSer.to_string();
}

template<typename Serial, typename R, typename... Args>
std::string run_callback(const Serial& obj, R (*func)(Args...)) RPC_HPP_EXCEPT
{
    return run_callback(obj, std::function<R(Args...)>(func));
}

template<typename Serial>
std::string run(const Serial& obj) RPC_HPP_EXCEPT
{
    const auto adapter = serial_adapter<Serial>(obj);
    const auto func_name = adapter.template get_value<std::string>("function");
    const auto argList = adapter.template get_value<Serial>("args");

    try
    {
        return dispatch<Serial>(func_name, argList);
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
        serial_adapter<Serial> result;
        result.set_value("result", -1);
        return result.to_string();
    }
}

template<typename Serial>
std::string run(std::string_view obj_str) RPC_HPP_EXCEPT
{
    const auto adapter = serial_adapter<Serial>(obj_str);
    const auto func_name = adapter.template get_value<std::string>("function");
    const auto argList = adapter.template get_value<Serial>("args");

    try
    {
        return dispatch<Serial>(func_name, argList);
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
        serial_adapter<Serial> result;
        result.set_value("result", -1);
        return result.to_string();
    }
}
} // namespace rpc
