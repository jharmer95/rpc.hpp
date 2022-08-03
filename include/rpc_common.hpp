///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.8.1
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
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

#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#define RPC_HPP_PRECONDITION(EXPR) assert(EXPR)
#define RPC_HPP_POSTCONDITION(EXPR) assert(EXPR)

#if defined(__GNUC__) || defined(__clang__)
#  define RPC_HPP_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define RPC_HPP_INLINE __forceinline
#else
#  define RPC_HPP_INLINE
#endif

///@brief Top-level namespace for rpc.hpp classes and functions
namespace rpc_hpp
{
///@brief Array containing the version information for rpc.hpp
static constexpr unsigned version[]{ 0, 8, 1 };

enum class exception_type
{
    none,
    func_not_found,
    remote_exec,
    serialization,
    deserialization,
    signature_mismatch,
    client_send,
    client_receive,
    server_send,
    server_receive,
};

class rpc_exception : public std::runtime_error
{
public:
    explicit rpc_exception(const std::string& mesg, exception_type type) noexcept
        : std::runtime_error(mesg), m_type(type)
    {
    }

    explicit rpc_exception(const char* mesg, exception_type type) noexcept
        : std::runtime_error(mesg), m_type(type)
    {
    }

    exception_type get_type() const noexcept { return m_type; }

private:
    exception_type m_type;
};

class function_not_found : public rpc_exception
{
public:
    explicit function_not_found(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }

    explicit function_not_found(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }
};

class remote_exec_error : public rpc_exception
{
public:
    explicit remote_exec_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }

    explicit remote_exec_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }
};

class serialization_error : public rpc_exception
{
public:
    explicit serialization_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::serialization)
    {
    }

    explicit serialization_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::serialization)
    {
    }
};

class deserialization_error : public rpc_exception
{
public:
    explicit deserialization_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }

    explicit deserialization_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }
};

class function_mismatch : public rpc_exception
{
public:
    explicit function_mismatch(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }

    explicit function_mismatch(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }
};

class client_send_error : public rpc_exception
{
public:
    explicit client_send_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::client_send)
    {
    }

    explicit client_send_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::client_send)
    {
    }
};

class client_receive_error : public rpc_exception
{
public:
    explicit client_receive_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }

    explicit client_receive_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }
};

class server_send_error : public rpc_exception
{
public:
    explicit server_send_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::server_send)
    {
    }

    explicit server_send_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::server_send)
    {
    }
};

class server_receive_error : public rpc_exception
{
public:
    explicit server_receive_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }

    explicit server_receive_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }
};

namespace adapters
{
    template<typename T>
    struct serial_traits;
}

///@brief Namespace for implementation details, should not be used outside of the library
namespace detail
{
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
    struct is_serializable :
        std::integral_constant<bool,
            is_serializable_base<Value, typename Serial::serial_t(const Value&)>::value
                && is_deserializable_base<Value, Value(const typename Serial::serial_t&)>::value>
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
    struct is_container :
        std::integral_constant<bool, has_size<C>::value && has_begin<C>::value && has_end<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_container_v = is_container<C>::value;

    template<typename F, typename... Ts, size_t... Is>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func,
        [[maybe_unused]] std::index_sequence<Is...> iseq)
    {
        using expander = int[];
        std::ignore = expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func)
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

    template<typename... Args>
    class packed_func_base
    {
    public:
        using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

        packed_func_base() = delete;
        packed_func_base(std::string func_name, args_t args) noexcept
            : m_func_name(std::move(func_name)), m_args(std::move(args))
        {
        }

        explicit operator bool() const noexcept { return m_except_type == exception_type::none; }
        const std::string& get_err_mesg() const noexcept { return m_err_mesg; }
        const std::string& get_func_name() const noexcept { return m_func_name; }
        exception_type get_except_type() const noexcept { return m_except_type; }

        void set_exception(std::string&& mesg, const exception_type type) & noexcept
        {
            m_except_type = type;
            m_err_mesg = std::move(mesg);
        }

        const args_t& get_args() const noexcept { return m_args; }
        args_t& get_args() noexcept { return m_args; }

    protected:
        ~packed_func_base() noexcept = default;
        packed_func_base(const packed_func_base&) = default;
        packed_func_base(packed_func_base&&) noexcept = default;
        packed_func_base& operator=(const packed_func_base&) & = default;
        packed_func_base& operator=(packed_func_base&&) & noexcept = default;

        [[noreturn]] void throw_ex() const noexcept(false)
        {
            switch (m_except_type)
            {
                case exception_type::func_not_found:
                    throw function_not_found(m_err_mesg);

                case exception_type::remote_exec:
                    throw remote_exec_error(m_err_mesg);

                case exception_type::serialization:
                    throw serialization_error(m_err_mesg);

                case exception_type::deserialization:
                    throw deserialization_error(m_err_mesg);

                case exception_type::signature_mismatch:
                    throw function_mismatch(m_err_mesg);

                case exception_type::client_send:
                    throw client_send_error(m_err_mesg);

                case exception_type::client_receive:
                    throw client_receive_error(m_err_mesg);

                case exception_type::server_send:
                    throw server_send_error(m_err_mesg);

                case exception_type::server_receive:
                    throw server_receive_error(m_err_mesg);

                case exception_type::none:
                default:
                    throw rpc_exception(m_err_mesg, exception_type::none);
            }
        }

    private:
        exception_type m_except_type{ exception_type::none };
        std::string m_func_name;
        std::string m_err_mesg{};
        args_t m_args;
    };

    template<typename R, typename... Args>
    class packed_func final : public packed_func_base<Args...>
    {
    public:
        using result_t = R;
        using typename packed_func_base<Args...>::args_t;

        packed_func(std::string func_name, std::optional<result_t> result, args_t args) noexcept
            : packed_func_base<Args...>(std::move(func_name), std::move(args)),
              m_result(std::move(result))
        {
        }

        explicit operator bool() const noexcept
        {
            return m_result.has_value() && packed_func_base<Args...>::operator bool();
        }

        const R& get_result() const
        {
            if (!static_cast<bool>(*this))
            {
                // throws exception based on except_type
                this->throw_ex();
            }

            return m_result.value();
        }

        void set_result(const R& value) & noexcept(std::is_nothrow_copy_assignable_v<R>)
        {
            m_result = value;
        }

        void set_result(R&& value) & noexcept(std::is_nothrow_move_assignable_v<R>)
        {
            m_result = std::move(value);
        }

        void clear_result() & noexcept { m_result.reset(); }

    private:
        std::optional<result_t> m_result{};
    };

    template<typename... Args>
    class packed_func<void, Args...> final : public packed_func_base<Args...>
    {
    public:
        using result_t = void;
        using typename packed_func_base<Args...>::args_t;
        using packed_func_base<Args...>::packed_func_base;
        using packed_func_base<Args...>::operator bool;

        void get_result() const
        {
            if (!static_cast<bool>(*this))
            {
                // throws exception based on except_type
                this->throw_ex();
            }
        }
    };

    template<typename Adapter>
    struct serial_adapter_base
    {
        using serial_t = typename adapters::serial_traits<Adapter>::serial_t;
        using bytes_t = typename adapters::serial_traits<Adapter>::bytes_t;

        static std::optional<serial_t> from_bytes(bytes_t&& bytes) = delete;
        static bytes_t to_bytes(serial_t&& serial_obj) = delete;
        static serial_t empty_object() = delete;

        template<typename R, typename... Args>
        static serial_t serialize_pack(const packed_func<R, Args...>& pack) = delete;

        template<typename R, typename... Args>
        static packed_func<R, Args...> deserialize_pack(const serial_t& serial_obj) = delete;

        static std::string get_func_name(const serial_t& serial_obj) = delete;
        static rpc_exception extract_exception(const serial_t& serial_obj) = delete;
        static void set_exception(serial_t& serial_obj, const rpc_exception& ex) = delete;
    };
} // namespace detail
} // namespace rpc_hpp
