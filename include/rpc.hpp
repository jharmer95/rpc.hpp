///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.6.2
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

#if defined(RPC_HPP_DOXYGEN_GEN)
///@brief Enables server-side caching abilities
#    define RPC_HPP_ENABLE_SERVER_CACHE
///@brief Indicates that rpc.hpp is being consumed by a client translation unit
#    define RPC_HPP_CLIENT_IMPL
///@brief Indicates that rpc.hpp is being consumed by a module (dynamically loaded .dll/.so) translation unit
#    define RPC_HPP_MODULE_IMPL
///@brief Indicates that rpc.hpp is being consumed by a server translation unit
#    define RPC_HPP_SERVER_IMPL
#endif

#if !defined(RPC_HPP_CLIENT_IMPL) && !defined(RPC_HPP_SERVER_IMPL) && !defined(RPC_HPP_MODULE_IMPL)
#    error At least one implementation type must be defined using 'RPC_HPP_{CLIENT, SERVER, MODULE}_IMPL'
#endif

#include <cassert>     // for assert
#include <cstddef>     // for size_t
#include <optional>    // for nullopt, optional
#include <stdexcept>   // for runtime_error
#include <string>      // for string
#include <tuple>       // for tuple, forward_as_tuple
#include <type_traits> // for declval, false_type, is_same, integral_constant
#include <utility>     // for move, index_sequence, make_index_sequence

#if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
#    include <unordered_map> // for unordered_map
#endif

#define RPC_HPP_PRECONDITION(EXPR) assert(EXPR)
#define RPC_HPP_POSTCONDITION(EXPR) assert(EXPR)

///@brief Top-level namespace for rpc.hpp classes and functions
namespace rpc
{
namespace exceptions
{
    enum class Type
    {
        None,
        FuncNotFound,
        RemoteExec,
        Serialization,
        DeSerialization,
        SignatureMismatch,
        ClientSend,
        ClientReceive,
        ServerSend,
        ServerReceive,
    };

    class rpc_exception : public std::runtime_error
    {
    public:
        explicit rpc_exception(const std::string& mesg, Type type) noexcept
            : std::runtime_error(mesg), m_type(type)
        {
        }

        explicit rpc_exception(const char* mesg, Type type) noexcept
            : std::runtime_error(mesg), m_type(type)
        {
        }

        Type get_type() const noexcept { return m_type; }

    private:
        Type m_type;
    };

    class function_not_found : public rpc_exception
    {
    public:
        explicit function_not_found(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::FuncNotFound)
        {
        }

        explicit function_not_found(const char* mesg) noexcept
            : rpc_exception(mesg, Type::FuncNotFound)
        {
        }
    };

    class remote_exec_error : public rpc_exception
    {
    public:
        explicit remote_exec_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::RemoteExec)
        {
        }

        explicit remote_exec_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::RemoteExec)
        {
        }
    };

    class serialization_error : public rpc_exception
    {
    public:
        explicit serialization_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::Serialization)
        {
        }

        explicit serialization_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::Serialization)
        {
        }
    };

    class deserialization_error : public rpc_exception
    {
    public:
        explicit deserialization_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::DeSerialization)
        {
        }

        explicit deserialization_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::DeSerialization)
        {
        }
    };

    class function_mismatch : public rpc_exception
    {
    public:
        explicit function_mismatch(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::SignatureMismatch)
        {
        }

        explicit function_mismatch(const char* mesg) noexcept
            : rpc_exception(mesg, Type::SignatureMismatch)
        {
        }
    };

    class client_send_error : public rpc_exception
    {
    public:
        explicit client_send_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::ClientSend)
        {
        }

        explicit client_send_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::ClientSend)
        {
        }
    };

    class client_receive_error : public rpc_exception
    {
    public:
        explicit client_receive_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::ClientReceive)
        {
        }

        explicit client_receive_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::ClientReceive)
        {
        }
    };

    class server_send_error : public rpc_exception
    {
    public:
        explicit server_send_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::ServerSend)
        {
        }

        explicit server_send_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::ServerSend)
        {
        }
    };

    class server_receive_error : public rpc_exception
    {
    public:
        explicit server_receive_error(const std::string& mesg) noexcept
            : rpc_exception(mesg, Type::ServerReceive)
        {
        }

        explicit server_receive_error(const char* mesg) noexcept
            : rpc_exception(mesg, Type::ServerReceive)
        {
        }
    };
} // namespace exceptions

///@brief Namespace for implementation details, should not be used outside of the library
namespace details
{
#if !defined(RPC_HPP_DOXYGEN_GEN)
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
    struct is_serializable
        : std::integral_constant<bool,
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
    struct is_container : std::integral_constant<bool,
                              has_size<C>::value && has_begin<C>::value && has_end<C>::value>
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

#    if defined(RPC_HPP_CLIENT_IMPL)
    template<typename... Args, size_t... Is>
    constexpr void tuple_bind(
        const std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>& src,
        std::index_sequence<Is...>, Args&&... dest)
    {
        using expander = int[];
        std::ignore = expander{ 0,
            (
                (void)[](auto&& x, auto&& y)
                {
                    if constexpr (
                        std::is_reference_v<
                            decltype(x)> && !std::is_const_v<std::remove_reference_t<decltype(x)>>)
                    {
                        x = std::forward<decltype(y)>(y);
                    }
                }(dest, std::get<Is>(src)),
                0)... };
    }

    template<typename... Args>
    constexpr void tuple_bind(
        const std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>& src, Args&&... dest)
    {
        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }
#    endif

    template<typename... Args>
    class packed_func_base
    {
    public:
        using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

        packed_func_base(std::string func_name, args_t args) noexcept
            : m_func_name(std::move(func_name)), m_args(std::move(args))
        {
            RPC_HPP_POSTCONDITION(!m_func_name.empty());
        }

        explicit operator bool() const noexcept { return m_except_type == exceptions::Type::None; }
        const std::string& get_err_mesg() const noexcept { return m_err_mesg; }
        const std::string& get_func_name() const noexcept { return m_func_name; }
        exceptions::Type get_except_type() const noexcept { return m_except_type; }

        void set_exception(std::string&& mesg, exceptions::Type type) & noexcept
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
                using namespace exceptions;

                case Type::FuncNotFound:
                    throw function_not_found(m_err_mesg);

                case Type::RemoteExec:
                    throw remote_exec_error(m_err_mesg);

                case Type::Serialization:
                    throw serialization_error(m_err_mesg);

                case Type::DeSerialization:
                    throw deserialization_error(m_err_mesg);

                case Type::SignatureMismatch:
                    throw function_mismatch(m_err_mesg);

                case Type::ClientSend:
                    throw client_send_error(m_err_mesg);

                case Type::ClientReceive:
                    throw client_receive_error(m_err_mesg);

                case Type::ServerSend:
                    throw server_send_error(m_err_mesg);

                case Type::ServerReceive:
                    throw server_receive_error(m_err_mesg);

                case Type::None:
                default:
                    throw rpc_exception(m_err_mesg, Type::None);
            }
        }

    private:
        exceptions::Type m_except_type{ exceptions::Type::None };
        std::string m_func_name;
        std::string m_err_mesg{};
        args_t m_args;
    };

    template<typename T_Serial, typename T_Bytes = std::string>
    struct serial_adapter
    {
        using serial_t = T_Serial;
        using bytes_t = T_Bytes;

        // nodiscard because a potentially expensive parse and copy is being done
        template<typename T>
        [[nodiscard]] static serial_t serialize(const T& val);

        // nodiscard because a potentially expensive parse and copy is being done
        template<typename T>
        [[nodiscard]] static T deserialize(const serial_t& serial_obj);

        // nodiscard because input bytes are lost after conversion
        [[nodiscard]] static serial_t from_bytes(bytes_t&& bytes);

        // nodiscard because input object is lost after conversion
        [[nodiscard]] static bytes_t to_bytes(serial_t&& serial_obj);
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

        void set_result(const R& value) & { m_result = value; }

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
#endif
} // namespace details

///@brief Class collecting several functions for interoperating between serial objects and a packed_func
///
///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
template<typename Serial>
class pack_adapter
{
public:
    ///@brief Serializes a packed_func to a serial object
    ///
    ///@tparam R Return type of the packed_func
    ///@tparam Args Variadic argument type(s) of the packed_func
    ///@param pack packed_func to be serialized
    ///@return Serial::serial_t Serial representation of the packed_func

    // nodiscard because a potentially expensive parse and copy is being done
    template<typename R, typename... Args>
    [[nodiscard]] static typename Serial::serial_t serialize_pack(
        const details::packed_func<R, Args...>& pack);

    ///@brief De-serializes a serial object to a packed_func
    ///
    ///@tparam R Return type of the packed_func
    ///@tparam Args Variadic argument type(s) of the packed_func
    ///@param serial_obj Serial object to be serialized
    ///@return packed_func<R, Args...> resulting packed_func

    // nodiscard because a potentially expensive parse and copy is being done
    template<typename R, typename... Args>
    [[nodiscard]] static details::packed_func<R, Args...> deserialize_pack(
        const typename Serial::serial_t& serial_obj);

    ///@brief Extracts the function name of a serialized function call
    ///
    ///@param serial_obj Serial object to be parsed
    ///@return std::string Name of the contained function

    // nodiscard because a potentially expensive parse and string allocation is being done
    [[nodiscard]] static std::string get_func_name(const typename Serial::serial_t& serial_obj);

    static void set_exception(
        typename Serial::serial_t& serial_obj, const exceptions::rpc_exception& ex);
};

#if defined(RPC_HPP_SERVER_IMPL) || defined(RPC_HPP_MODULE_IMPL)
///@brief Namespace containing functions and classes only relevant to "server-side" implentations
///
///@note Is only compiled by defining either @ref RPC_HPP_SERVER_IMPL AND/OR @ref RPC_HPP_MODULE_IMPL
inline namespace server
{
    ///@brief Class defining an interface for serving functions via RPC
    ///
    ///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
    template<typename Serial>
    class server_interface
    {
    public:
        using adapter_t = Serial;

        virtual ~server_interface() noexcept = default;
        server_interface() noexcept = default;

        server_interface(const server_interface&) = delete;
        server_interface& operator=(const server_interface&) = delete;
        server_interface& operator=(server_interface&&) = delete;

#    if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        template<typename Val>
        std::unordered_map<typename Serial::bytes_t, Val>& get_func_cache(
            const std::string& func_name)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            update_all_cache<Val>(func_name);
            return *static_cast<std::unordered_map<typename Serial::bytes_t, Val>*>(
                m_cache_map.at(func_name));
        }

        void clear_all_cache() noexcept { m_cache_map.clear(); }
#    endif

        ///@brief Parses the received serialized data and determines which function to call
        ///
        ///@param bytes Data to be parsed into/back out of a serial object
        void dispatch(typename Serial::bytes_t& bytes)
        {
            auto serial_obj = Serial::from_bytes(std::move(bytes));

            try
            {
                dispatch_impl(serial_obj);
            }
            catch (const exceptions::rpc_exception& ex)
            {
                pack_adapter<Serial>::set_exception(serial_obj, ex);
            }

            bytes = Serial::to_bytes(std::move(serial_obj));
        }

    protected:
        server_interface(server_interface&&) noexcept = default;

#    if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        ///@brief Deserializes the serial object to a packed_func, calls the callback function, then serializes the result back to the serial object, using a server-side cache for performance.
        ///
        ///@note This feature is enabled by defining @ref RPC_HPP_ENABLE_SERVER_CACHE
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func pointer to the callback function
        ///@param serial_obj Serial representing the function call
        template<typename R, typename... Args>
        void dispatch_cached_func(R (*func)(Args...), typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto pack = [&serial_obj]
            {
                try
                {
                    return pack_adapter<Serial>::template deserialize_pack<R, Args...>(serial_obj);
                }
                catch (const exceptions::rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw exceptions::deserialization_error(ex.what());
                }
            }();

            auto& result_cache = get_func_cache<R>(pack.get_func_name());

            if constexpr (!std::is_void_v<R>)
            {
                auto bytes = Serial::to_bytes(std::move(serial_obj));

                const auto it = result_cache.find(bytes);

                if (it != result_cache.end())
                {
                    pack.set_result(it->second);

                    try
                    {
                        serial_obj =
                            pack_adapter<Serial>::template serialize_pack<R, Args...>(pack);
                        return;
                    }
                    catch (const exceptions::rpc_exception&)
                    {
                        throw;
                    }
                    catch (const std::exception& ex)
                    {
                        throw exceptions::serialization_error(ex.what());
                    }
                }

                run_callback(func, pack);
                result_cache[std::move(bytes)] = pack.get_result();
            }
            else
            {
                run_callback(func, pack);
            }

            try
            {
                serial_obj = pack_adapter<Serial>::template serialize_pack<R, Args...>(pack);
            }
            catch (const exceptions::rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw exceptions::serialization_error(ex.what());
            }
        }

#    else
        template<typename R, typename... Args>
        void dispatch_cached_func(R (*func)(Args...), typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            dispatch_func(func, serial_obj);
        }
#    endif

        ///@brief Deserializes the serial object to a packed_func, calls the callback function, then serializes the result back to the serial object
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func pointer to the callback function
        ///@param serial_obj Serial representing the function call
        template<typename R, typename... Args>
        static void dispatch_func(R (*func)(Args...), typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto pack = [&serial_obj]
            {
                try
                {
                    return pack_adapter<Serial>::template deserialize_pack<R, Args...>(serial_obj);
                }
                catch (const exceptions::rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw exceptions::deserialization_error(ex.what());
                }
            }();

            run_callback(func, pack);

            try
            {
                serial_obj = pack_adapter<Serial>::template serialize_pack<R, Args...>(pack);
            }
            catch (const exceptions::rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw exceptions::serialization_error(ex.what());
            }
        }

    private:
        virtual void dispatch_impl(typename Serial::serial_t& serial_obj) = 0;

        ///@brief Runs the callback function and updates the result (and any changes to mutable arguments) in the packed_func
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func pointer to the callback function
        ///@param pack packed_func representing the callback
        template<typename R, typename... Args>
        static void run_callback(R (*func)(Args...), details::packed_func<R, Args...>& pack)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto& args = pack.get_args();

            if constexpr (std::is_void_v<R>)
            {
                try
                {
                    std::apply(func, args);
                }
                catch (const std::exception& ex)
                {
                    throw exceptions::remote_exec_error(ex.what());
                }
            }
            else
            {
                try
                {
                    auto result = std::apply(func, args);
                    pack.set_result(std::move(result));
                }
                catch (const std::exception& ex)
                {
                    throw exceptions::remote_exec_error(ex.what());
                }
            }
        }

#    if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        template<typename Val>
        static void* get_func_cache_impl(const std::string& func_name)
        {
            static std::unordered_map<std::string,
                std::unordered_map<typename Serial::bytes_t, Val>>
                cache{};

            return &cache[func_name];
        }

        template<typename Val>
        void update_all_cache(const std::string& func_name)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            m_cache_map.insert_or_assign(func_name, get_func_cache_impl<Val>(func_name));
        }

        std::unordered_map<std::string, void*> m_cache_map{};
#    endif
    };
} // namespace server
#endif

#if defined(RPC_HPP_CLIENT_IMPL)
#    define call_header_func(FUNCNAME, ...) call_header_func_impl(FUNCNAME, #    FUNCNAME, __VA_ARGS__)

///@brief Namespace containing functions and classes only relevant to "client-side" implentations
///
///@note Is only compiled by defining @ref RPC_HPP_CLIENT_IMPL
inline namespace client
{
    ///@brief Class defining an interface for calling into an RPC server or module
    ///
    ///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
    template<typename Serial>
    class client_interface
    {
    public:
        virtual ~client_interface() noexcept = default;
        client_interface() noexcept = default;

        client_interface(const client_interface&) = delete;
        client_interface(client_interface&&) noexcept = default;
        client_interface& operator=(const client_interface&) = delete;
        client_interface& operator=(client_interface&&) = delete;

        ///@brief Sends an RPC call request to a server, waits for a response, then returns the result
        ///
        ///@tparam R Return type of the remote function to call
        ///@tparam Args Variadic argument type(s) of the remote function to call
        ///@param func_name Name of the remote function to call
        ///@param args Argument(s) for the remote function
        ///@return R Result of the function call, will throw with server's error message if the result does not exist

        // nodiscard because an expensive remote procedure call is being performed
        template<typename R = void, typename... Args>
        [[nodiscard]] R call_func(const std::string& func_name, Args&&... args)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            details::packed_func<R, Args...> pack = [&]() noexcept
            {
                if constexpr (std::is_void_v<R>)
                {
                    return details::packed_func<void, Args...>{ func_name,
                        std::forward_as_tuple(args...) };
                }
                else
                {
                    return details::packed_func<R, Args...>{ func_name, std::nullopt,
                        std::forward_as_tuple(args...) };
                }
            }();

            auto serial_obj = [&pack]
            {
                try
                {
                    return pack_adapter<Serial>::serialize_pack(pack);
                }
                catch (const exceptions::rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw exceptions::serialization_error(ex.what());
                }
            }();

            auto bytes = Serial::to_bytes(std::move(serial_obj));

            try
            {
                send(std::move(bytes));
            }
            catch (const std::exception& ex)
            {
                throw exceptions::client_send_error(ex.what());
            }

            try
            {
                bytes = receive();
            }
            catch (const std::exception& ex)
            {
                throw exceptions::client_receive_error(ex.what());
            }

            serial_obj = Serial::from_bytes(std::move(bytes));

            try
            {
                pack = pack_adapter<Serial>::template deserialize_pack<R, Args...>(serial_obj);
            }
            catch (const exceptions::rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw exceptions::deserialization_error(ex.what());
            }

            // Assign values back to any (non-const) reference members
            details::tuple_bind(pack.get_args(), std::forward<Args>(args)...);
            return pack.get_result();
        }

        ///@brief Sends an RPC call request to a server, waits for a response, then returns the result
        ///
        ///@tparam R Return type of the remote function to call
        ///@tparam Args Variadic argument type(s) of the remote function to call
        ///@param func_name Name of the remote function to call
        ///@param args Argument(s) for the remote function
        ///@return R Result of the function call, will throw with server's error message if the result does not exist

        // nodiscard because an expensive remote procedure call is being performed
        template<typename R = void, typename... Args>
        [[nodiscard]] R call_func(std::string&& func_name, Args&&... args)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            details::packed_func<R, Args...> pack = [&]() noexcept
            {
                if constexpr (std::is_void_v<R>)
                {
                    return details::packed_func<void, Args...>{ std::move(func_name),
                        std::forward_as_tuple(args...) };
                }
                else
                {
                    return details::packed_func<R, Args...>{ std::move(func_name), std::nullopt,
                        std::forward_as_tuple(args...) };
                }
            }();

            auto serial_obj = [&pack]
            {
                try
                {
                    return pack_adapter<Serial>::serialize_pack(pack);
                }
                catch (const exceptions::rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw exceptions::serialization_error(ex.what());
                }
            }();

            auto bytes = Serial::to_bytes(std::move(serial_obj));

            try
            {
                send(std::move(bytes));
            }
            catch (const std::exception& ex)
            {
                throw exceptions::client_send_error(ex.what());
            }

            try
            {
                bytes = receive();
            }
            catch (const std::exception& ex)
            {
                throw exceptions::client_receive_error(ex.what());
            }

            serial_obj = Serial::from_bytes(std::move(bytes));

            try
            {
                pack = pack_adapter<Serial>::template deserialize_pack<R, Args...>(serial_obj);
            }
            catch (const exceptions::rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw exceptions::deserialization_error(ex.what());
            }

            // Assign values back to any (non-const) reference members
            details::tuple_bind(pack.get_args(), std::forward<Args>(args)...);
            return pack.get_result();
        }

        // nodiscard because an expensive remote procedure call is being performed
        template<typename R, typename... Args>
        [[nodiscard]] R call_header_func_impl(
            [[maybe_unused]] R (*func)(Args...), const std::string& func_name, Args&&... args)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            return call_func<R, Args...>(func_name, std::forward<Args>(args)...);
        }

        // nodiscard because an expensive remote procedure call is being performed
        template<typename R, typename... Args>
        [[nodiscard]] R call_header_func_impl(
            [[maybe_unused]] R (*func)(Args...), std::string&& func_name, Args&&... args)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            return call_func<R, Args...>(std::move(func_name), std::forward<Args>(args)...);
        }

    protected:
        ///@brief Sends serialized data to a server or module
        ///
        ///@param bytes Serialized data to be sent
        virtual void send(const typename Serial::bytes_t& bytes) = 0;

        ///@brief Sends serialized data to a server or module
        ///
        ///@param bytes Serialized data to be sent
        virtual void send(typename Serial::bytes_t&& bytes) = 0;

        ///@brief Receives serialized data from a server or module
        ///
        ///@return Serial::bytes_t Received serialized data
        virtual typename Serial::bytes_t receive() = 0;
    };
} // namespace client
#endif
} // namespace rpc
