#pragma once

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#define RPC_HPP_POSTCONDITION(EXPR) assert(EXPR)
#define RPC_HPP_PRECONDITION(EXPR) assert(EXPR)

#if defined(__GNUC__) && !defined(__clang__) \
    && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 3))
// Workaround for bug in GCC
#  define RPC_HPP_UNUSED [[gnu::unused]]
#else
#  define RPC_HPP_UNUSED [[maybe_unused]]
#endif

#if __has_cpp_attribute(nodiscard) >= 201907L
#  define RPC_HPP_NODISCARD(REASON) [[nodiscard(REASON)]]
#else
#  define RPC_HPP_NODISCARD(REASON) [[nodiscard]]
#endif

namespace rpc_hpp
{
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
    rpc_object_mismatch,
    callback_install,
    callback_missing,
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

class rpc_object_mismatch : public rpc_exception
{
public:
    explicit rpc_object_mismatch(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }

    explicit rpc_object_mismatch(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }
};

class callback_install_error : public rpc_exception
{
public:
    explicit callback_install_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::callback_install)
    {
    }

    explicit callback_install_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::callback_install)
    {
    }
};

class callback_missing_error : public rpc_exception
{
public:
    explicit callback_missing_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::callback_missing)
    {
    }

    explicit callback_missing_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::callback_missing)
    {
    }
};

namespace detail
{
    // backport for C++20's remove_cvref
    template<typename T>
    struct remove_cvref
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

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
        RPC_HPP_UNUSED std::index_sequence<Is...> iseq)
    {
        using expander = int[];
        std::ignore = expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func)
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

    template<typename T>
    struct decay_str
    {
        static_assert(!std::is_pointer_v<remove_cvref_t<T>>, "Pointer parameters are not allowed");

        static_assert(
            !std::is_array_v<remove_cvref_t<T>>, "C-style array parameters are not allowed");

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

    template<typename T>
    constexpr bool is_ref_arg()
    {
        return std::is_reference_v<
                   T> && !std::is_const_v<std::remove_reference_t<T>> && !std::is_pointer_v<std::remove_reference_t<T>>;
    }

    template<typename... Args>
    constexpr bool has_ref_args()
    {
        return (... || is_ref_arg<Args>());
    }

    template<typename... Args, size_t... Is>
    constexpr void tuple_bind(const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src,
        RPC_HPP_UNUSED std::index_sequence<Is...> iseq, Args&&... dest)
    {
        using expander = int[];
        std::ignore = expander{ 0,
            (
                (void)[](auto&& x, auto&& y) {
                    if constexpr (is_ref_arg<decltype(x)>())
                    {
                        x = std::forward<decltype(y)>(y);
                    }
                }(dest, std::get<Is>(src)),
                0)... };
    }

    template<typename... Args>
    constexpr void tuple_bind(
        const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src, Args&&... dest)
    {
        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }

    struct bind_args_tag
    {
    };

    template<bool IsCallback>
    struct rpc_base
    {
        static constexpr bool is_callback = IsCallback;
        std::string func_name{};
    };

    template<bool IsCallback, typename... Args>
    struct rpc_request : rpc_base<IsCallback>
    {
        using args_t = std::tuple<remove_cvref_t<Args>...>;

        rpc_request() = default;
        rpc_request(std::string t_func_name, args_t t_args)
            : rpc_base<IsCallback>{ std::move(t_func_name) }, args(std::move(t_args))
        {
        }

        rpc_request(RPC_HPP_UNUSED bind_args_tag tag, std::string t_func_name, args_t t_args)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              bind_args(true),
              args(std::move(t_args))
        {
        }

        bool bind_args{ false };
        args_t args{};
    };

    template<typename... Args>
    using func_request = rpc_request<false, Args...>;

    template<typename... Args>
    using callback_request = rpc_request<true, Args...>;

    template<bool IsCallback, typename R>
    struct rpc_result : rpc_base<IsCallback>
    {
        R result;
    };

    template<bool IsCallback>
    struct rpc_result<IsCallback, void> : rpc_base<IsCallback>
    {
    };

    template<typename R>
    using func_result = rpc_result<false, R>;

    template<typename R>
    using callback_result = rpc_result<true, R>;

    template<bool IsCallback, typename R, typename... Args>
    struct rpc_result_w_bind : rpc_base<IsCallback>
    {
        using args_t = std::tuple<remove_cvref_t<Args>...>;

        rpc_result_w_bind() = default;
        rpc_result_w_bind(std::string t_func_name, R t_result, args_t t_args)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              result(std::move(t_result)),
              args(std::move(t_args))
        {
        }

        R result{};
        args_t args{};
    };

    template<bool IsCallback, typename... Args>
    struct rpc_result_w_bind<IsCallback, void, Args...> : rpc_base<IsCallback>
    {
        using args_t = std::tuple<remove_cvref_t<Args>...>;

        rpc_result_w_bind() = default;
        rpc_result_w_bind(std::string t_func_name, args_t t_args)
            : rpc_base<IsCallback>{ std::move(t_func_name) }, args(std::move(t_args))
        {
        }

        args_t args{};
    };

    template<typename R, typename... Args>
    using func_result_w_bind = rpc_result_w_bind<false, R, Args...>;

    template<typename R, typename... Args>
    using callback_result_w_bind = rpc_result_w_bind<true, R, Args...>;

    template<bool IsCallback>
    struct rpc_error : rpc_base<IsCallback>
    {
        rpc_error() = default;
        rpc_error(std::string t_func_name, const rpc_exception& except)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              except_type(except.get_type()),
              err_mesg(except.what())
        {
        }

        rpc_error(std::string t_func_name, exception_type t_ex_type, std::string t_err_mesg)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              except_type(t_ex_type),
              err_mesg(std::move(t_err_mesg))
        {
        }

        [[noreturn]] void rethrow() const
        {
            switch (except_type)
            {
                case exception_type::func_not_found:
                    throw function_not_found(err_mesg);

                case exception_type::remote_exec:
                    throw remote_exec_error(err_mesg);

                case exception_type::serialization:
                    throw serialization_error(err_mesg);

                case exception_type::deserialization:
                    throw deserialization_error(err_mesg);

                case exception_type::signature_mismatch:
                    throw function_mismatch(err_mesg);

                case exception_type::client_send:
                    throw client_send_error(err_mesg);

                case exception_type::client_receive:
                    throw client_receive_error(err_mesg);

                case exception_type::server_send:
                    throw server_send_error(err_mesg);

                case exception_type::server_receive:
                    throw server_receive_error(err_mesg);

                case exception_type::rpc_object_mismatch:
                    throw rpc_object_mismatch(err_mesg);

                case exception_type::callback_install:
                    throw callback_install_error(err_mesg);

                case exception_type::callback_missing:
                    throw callback_missing_error(err_mesg);

                case exception_type::none:
                default:
                    throw rpc_exception(err_mesg, exception_type::none);
            }
        }

        exception_type except_type{ exception_type::none };
        std::string err_mesg{};
    };

    using func_error = rpc_error<false>;
    using callback_error = rpc_error<true>;
} //namespace detail

struct callback_install_request : detail::rpc_base<true>
{
    callback_install_request() = default;
    callback_install_request(std::string t_func_name) : rpc_base<true>{ std::move(t_func_name) } {}

    bool is_uninstall{ false };
};

enum class rpc_type
{
    callback_install_request,
    callback_error,
    callback_request,
    callback_result,
    callback_result_w_bind,
    func_error,
    func_request,
    func_result,
    func_result_w_bind,
};

template<typename Serial>
class rpc_object
{
public:
    using serial_t = typename Serial::serial_t;
    using bytes_t = typename Serial::bytes_t;

    RPC_HPP_NODISCARD("parsing consumes the original input")
    static std::optional<rpc_object> parse_bytes(bytes_t&& bytes)
    {
        try
        {
            return rpc_object{ Serial::from_bytes(std::move(bytes)) };
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }

    template<bool IsCallback, typename R>
    explicit rpc_object(detail::rpc_result<IsCallback, R> result)
        : m_obj(Serial::serialize_result(std::move(result)))
    {
    }

    template<bool IsCallback, typename... Args>
    explicit rpc_object(detail::rpc_request<IsCallback, Args...> request)
        : m_obj(Serial::serialize_request(std::move(request)))
    {
    }

    template<bool IsCallback>
    explicit rpc_object(detail::rpc_error<IsCallback> error)
        : m_obj(Serial::serialize_error(std::move(error)))
    {
    }

    template<bool IsCallback, typename R, typename... Args>
    explicit rpc_object(detail::rpc_result_w_bind<IsCallback, R, Args...> result)
        : m_obj(Serial::serialize_result_w_bind(std::move(result)))
    {
    }

    explicit rpc_object(callback_install_request callback_req)
        : m_obj(Serial::serialize_callback_install(std::move(callback_req)))
    {
    }

    RPC_HPP_NODISCARD("converting to bytes may be expensive") bytes_t to_bytes() const&
    {
        return Serial::to_bytes(m_obj);
    }

    RPC_HPP_NODISCARD("converting to bytes consumes object") bytes_t to_bytes() &&
    {
        return Serial::to_bytes(std::move(m_obj));
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    std::string get_func_name() const { return Serial::get_func_name(m_obj); }

    template<typename R, bool IsCallback = false>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    R get_result() const
    {
        switch (type())
        {
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
                if constexpr (std::is_void_v<R>)
                {
                    return;
                }
                else
                {
                    return Serial::template get_result<IsCallback, R>(m_obj).result;
                }

            case rpc_type::func_error:
            case rpc_type::callback_error:
                Serial::template get_error<IsCallback>(m_obj).rethrow();

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::func_request:
            default:
                throw rpc_object_mismatch("Invalid rpc_object type detected");
        }
    }

    template<bool IsCallback = false, typename... Args>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    typename detail::rpc_request<IsCallback, Args...>::args_t get_args() const
    {
        switch (type())
        {
            case rpc_type::func_request:
            case rpc_type::func_result_w_bind:
            case rpc_type::callback_request:
            case rpc_type::callback_result_w_bind:
                return Serial::template get_request<IsCallback, Args...>(m_obj).args;

            case rpc_type::callback_error:
            case rpc_type::callback_install_request:
            case rpc_type::callback_result:
            case rpc_type::func_error:
            case rpc_type::func_result:
            default:
                throw rpc_object_mismatch("Invalid rpc_object type detected");
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool is_callback_uninstall() const
    {
        if (type() != rpc_type::callback_install_request)
        {
            return false;
        }

        return Serial::get_callback_install(m_obj).is_uninstall;
    }

    template<bool IsCallback = false>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    exception_type get_error_type() const
    {
        if (!is_error())
        {
            throw rpc_object_mismatch("Invalid rpc_object type detected");
        }

        return Serial::template get_error<IsCallback>(m_obj).except_type;
    }

    template<bool IsCallback = false>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    std::string get_error_mesg() const
    {
        if (!is_error())
        {
            throw rpc_object_mismatch("Invalid rpc_object type detected");
        }

        return Serial::template get_error<IsCallback>(m_obj).error_mesg;
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool has_bound_args() const
    {
        switch (type())
        {
            case rpc_type::func_request:
            case rpc_type::callback_request:
                return Serial::has_bound_args(m_obj);

            case rpc_type::func_result_w_bind:
            case rpc_type::callback_result_w_bind:
                return true;

            case rpc_type::callback_error:
            case rpc_type::callback_install_request:
            case rpc_type::callback_result:
            case rpc_type::func_error:
            case rpc_type::func_result:
            default:
                throw rpc_object_mismatch("Invalid rpc_object type detected");
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive") bool is_error() const
    {
        const auto rtype = type();
        return rtype == rpc_type::func_error || rtype == rpc_type::callback_error;
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive") rpc_type type() const
    {
        return Serial::get_type(m_obj);
    }

private:
    explicit rpc_object(const serial_t& serial) : m_obj(serial) {}
    explicit rpc_object(serial_t&& serial) : m_obj(std::move(serial)) {}

    serial_t m_obj;
};

namespace adapters
{
    template<typename T>
    struct serial_traits;

    template<typename Adapter>
    class serial_adapter_base
    {
    public:
        using serial_t = typename serial_traits<Adapter>::serial_t;
        using bytes_t = typename serial_traits<Adapter>::bytes_t;

        static serial_t from_bytes(bytes_t&& bytes) = delete;
        static bytes_t to_bytes(const serial_t& serial_obj) = delete;
        static bytes_t to_bytes(serial_t&& serial_obj) = delete;
        static std::string get_func_name(const serial_t& serial_obj) = delete;
        static rpc_type get_type(const serial_t& serial_obj) = delete;

        template<bool IsCallback, typename R>
        static detail::rpc_result<IsCallback, R> get_result(const serial_t& serial_obj) = delete;

        template<bool IsCallback, typename R>
        static serial_t serialize_result(const detail::rpc_result<IsCallback, R>& result) = delete;

        template<bool IsCallback, typename R, typename... Args>
        static serial_t serialize_result_w_bind(
            const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) = delete;

        template<bool IsCallback, typename... Args>
        static detail::rpc_request<IsCallback, Args...> get_request(
            const serial_t& serial_obj) = delete;

        template<bool IsCallback, typename... Args>
        static serial_t serialize_request(
            const detail::rpc_request<IsCallback, Args...>& request) = delete;

        template<bool IsCallback>
        static detail::rpc_error<IsCallback> get_error(const serial_t& serial_obj) = delete;

        template<bool IsCallback>
        static serial_t serialize_error(const detail::rpc_error<IsCallback>& error) = delete;

        static callback_install_request get_callback_install(const serial_t& serial_obj) = delete;
        static serial_t serialize_callback_install(
            const callback_install_request& callback_req) = delete;

        static bool has_bound_args(const serial_t& serial_obj) = delete;
    };
} //namespace adapters

namespace detail
{
    template<bool IsCallback, typename Serial, typename R, typename... Args, typename F>
    void exec_func(F&& func, rpc_object<Serial>& rpc_obj)
    {
        auto args = rpc_obj.template get_args<IsCallback, Args...>();
        auto func_name = rpc_obj.get_func_name();
        const auto has_bound_args = rpc_obj.has_bound_args();

        try
        {
            if constexpr (std::is_void_v<R>)
            {
                std::apply(std::forward<F>(func), args);

                if (has_bound_args)
                {
                    rpc_obj =
                        rpc_object<Serial>{ detail::rpc_result_w_bind<IsCallback, void, Args...>{
                            std::move(func_name), std::move(args) } };
                }
                else
                {
                    rpc_obj = rpc_object<Serial>{ detail::rpc_result<IsCallback, void>{
                        std::move(func_name) } };
                }
            }
            else
            {
                auto ret_val = std::apply(std::forward<F>(func), args);

                if (has_bound_args)
                {
                    rpc_obj = rpc_object<Serial>{ detail::rpc_result_w_bind<IsCallback, R, Args...>{
                        std::move(func_name), std::move(ret_val), std::move(args) } };
                }
                else
                {
                    rpc_obj = rpc_object<Serial>{ detail::rpc_result<IsCallback, R>{
                        std::move(func_name), std::move(ret_val) } };
                }
            }
        }
        catch (const std::exception& ex)
        {
            throw remote_exec_error(ex.what());
        }
    }
} //namespace detail
} //namespace rpc_hpp
