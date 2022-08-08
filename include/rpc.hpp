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

namespace rpc_hpp
{
namespace adapters
{
    template<typename T>
    struct serial_traits;
}

namespace detail
{
#ifdef __cpp_lib_remove_cvref
    using std::remove_cvref;
    using std::remove_cvref_t;
#else
    // backport for C++20's remove_cvref
    template<typename T>
    struct remove_cvref
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;
#endif

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
} //namespace detail

struct bind_args_tag
{
};

template<typename... Args>
class func_request
{
public:
    using args_t = std::tuple<detail::remove_cvref_t<Args>...>;

    func_request(std::string func_name, args_t args)
        : m_func_name(std::move(func_name)), m_args(std::move(args))
    {
    }

    func_request(std::string func_name, detail::remove_cvref_t<Args>&&... args)
        : m_func_name(std::move(func_name)), m_args(std::forward_as_tuple(args...))
    {
    }

    func_request([[maybe_unused]] bind_args_tag tag, std::string func_name,
        detail::remove_cvref_t<Args>&&... args)
        : m_bind_args(true), m_func_name(std::move(func_name)),
          m_args(std::forward_as_tuple(args...))
    {
    }

    func_request([[maybe_unused]] bind_args_tag tag, std::string func_name, args_t args)
        : m_bind_args(true), m_func_name(std::move(func_name)), m_args(std::move(args))
    {
    }

    bool has_bound_args() const { return m_bind_args; }
    const std::string& get_func_name() const { return m_func_name; }
    const args_t& get_args() const { return m_args; }

private:
    bool m_bind_args{ false };
    std::string m_func_name;
    args_t m_args;
};

template<typename R>
class func_result
{
public:
    func_result(std::string func_name, R result)
        : m_func_name(std::move(func_name)), m_result(std::move(result))
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const R& get_result() const { return m_result; }

private:
    std::string m_func_name;
    R m_result;
};

template<>
class func_result<void>
{
public:
    func_result(std::string func_name) : m_func_name(std::move(func_name)) {}

    const std::string& get_func_name() const { return m_func_name; }

private:
    std::string m_func_name;
};

template<typename R, typename... Args>
class func_result_w_bind
{
public:
    using args_t = std::tuple<detail::remove_cvref_t<Args>...>;

    func_result_w_bind(std::string func_name, R result, args_t args)
        : m_result(std::move(result)), m_func_name(std::move(func_name)), m_args(std::move(args))
    {
    }

    func_result_w_bind(std::string func_name, R result, detail::remove_cvref_t<Args>&&... args)
        : m_result(std::move(result)), m_func_name(std::move(func_name)),
          m_args(std::forward_as_tuple<Args>(args)...)
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const R& get_result() const { return m_result; }
    const args_t& get_args() const { return m_args; }

private:
    R m_result;
    std::string m_func_name;
    args_t m_args;
};

template<typename... Args>
class func_result_w_bind<void, Args...>
{
public:
    using args_t = std::tuple<detail::remove_cvref_t<Args>...>;

    func_result_w_bind(std::string func_name, args_t args)
        : m_func_name(std::move(func_name)), m_args(std::move(args))
    {
    }

    func_result_w_bind(std::string func_name, detail::remove_cvref_t<Args>&&... args)
        : m_func_name(std::move(func_name)), m_args(std::forward_as_tuple<Args>(args)...)
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const args_t& get_args() const { return m_args; }

private:
    std::string m_func_name;
    args_t m_args;
};

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

class rpc_object_type_mismatch : public rpc_exception
{
public:
    explicit rpc_object_type_mismatch(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }

    explicit rpc_object_type_mismatch(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }
};

class func_error
{
public:
    func_error(std::string func_name, const rpc_exception& except)
        : m_except_type(except.get_type()), m_func_name(std::move(func_name)),
          m_err_mesg(except.what())
    {
    }

    func_error(std::string func_name, exception_type ex_type, std::string err_mesg)
        : m_except_type(ex_type), m_func_name(std::move(func_name)), m_err_mesg(std::move(err_mesg))
    {
    }

    const std::string& get_func_name() const { return m_func_name; }
    const std::string& get_err_mesg() const { return m_err_mesg; }
    exception_type get_except_type() const { return m_except_type; }

    [[noreturn]] void rethrow() const
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

            case exception_type::rpc_object_mismatch:
                throw rpc_object_type_mismatch(m_err_mesg);

            case exception_type::none:
            default:
                throw rpc_exception(m_err_mesg, exception_type::none);
        }
    }

private:
    exception_type m_except_type;
    std::string m_func_name;
    std::string m_err_mesg;
};

class callback_install_request
{
public:
    callback_install_request(std::string func_name, uint64_t id)
        : m_id(id), m_func_name(std::move(func_name))
    {
    }

    bool is_uninstall() const { return m_uninstall; }
    void set_uninstall(bool uninstall) { m_uninstall = uninstall; }
    uint64_t get_id() const { return m_id; }
    const std::string& get_func_name() const { return m_func_name; }

private:
    bool m_uninstall{ false };
    uint64_t m_id;
    std::string m_func_name;
};

enum class rpc_object_type
{
    callback_error,
    callback_install_request,
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

    template<typename R>
    explicit rpc_object(func_result<R> result)
        : m_obj(Serial::template serialize_result<R>(std::move(result)))
    {
    }

    template<typename... Args>
    explicit rpc_object(func_request<Args...> request)
        : m_obj(Serial::template serialize_request<Args...>(std::move(request)))
    {
    }

    explicit rpc_object(func_error error) : m_obj(Serial::serialize_error(std::move(error))) {}

    template<typename R, typename... Args>
    explicit rpc_object(func_result_w_bind<R, Args...> result)
        : m_obj(Serial::template serialize_result_w_bind<R, Args...>(std::move(result)))
    {
    }

    bytes_t to_bytes() const { return Serial::to_bytes(m_obj); }
    std::string get_func_name() const { return Serial::get_func_name(m_obj); }

    template<typename R>
    R get_result() const
    {
        switch (type())
        {
            case rpc_object_type::func_result:
            case rpc_object_type::func_result_w_bind:
            case rpc_object_type::callback_result:
            case rpc_object_type::callback_result_w_bind:
                if constexpr (std::is_void_v<R>)
                {
                    return;
                }
                else
                {
                    return Serial::template get_result<R>(m_obj).get_result();
                }

            case rpc_object_type::func_error:
            case rpc_object_type::callback_error:
                Serial::get_error(m_obj).rethrow();

            default:
                throw rpc_object_type_mismatch("Invalid rpc_object type detected");
        }
    }

    template<typename... Args>
    typename func_request<Args...>::args_t get_args() const
    {
        switch (type())
        {
            case rpc_object_type::func_request:
            case rpc_object_type::func_result_w_bind:
            case rpc_object_type::callback_request:
            case rpc_object_type::callback_result_w_bind:
                return Serial::template get_request<Args...>(m_obj).get_args();

            default:
                throw rpc_object_type_mismatch("Invalid rpc_object type detected");
        }
    }

    bool is_callback_uninstall() const
    {
        if (type() != rpc_object_type::callback_install_request)
        {
            return false;
        }

        return Serial::get_callback(m_obj).is_uninstall();
    }

    uint64_t get_callback_id() const
    {
        if (type() != rpc_object_type::callback_install_request)
        {
            throw rpc_object_type_mismatch("Invalid rpc_object type detected");
        }

        return Serial::get_callback(m_obj).get_id();
    }

    exception_type get_error_type() const
    {
        if (!is_error())
        {
            throw rpc_object_type_mismatch("Invalid rpc_object type detected");
        }

        return Serial::get_error(m_obj).get_except_type();
    }

    std::string get_error_mesg() const
    {
        if (!is_error())
        {
            throw rpc_object_type_mismatch("Invalid rpc_object type detected");
        }

        return Serial::get_error(m_obj).get_error_mesg();
    }

    bool has_bound_args() const
    {
        switch (type())
        {
            case rpc_object_type::func_request:
            case rpc_object_type::callback_request:
                return Serial::has_bound_args(m_obj);

            case rpc_object_type::func_result_w_bind:
            case rpc_object_type::callback_result_w_bind:
                return true;

            default:
                throw rpc_object_type_mismatch("Invalid rpc_object type detected");
        }
    }

    bool is_error() const
    {
        const auto rtype = type();
        return rtype == rpc_object_type::func_error || rtype == rpc_object_type::callback_error;
    }

    rpc_object_type type() const { return Serial::get_type(m_obj); }

private:
    rpc_object(const serial_t& serial) : m_obj(serial) {}
    rpc_object(serial_t&& serial) : m_obj(std::move(serial)) {}

    serial_t m_obj;
};

template<typename Adapter>
class serial_adapter_base
{
public:
    using serial_t = typename adapters::serial_traits<Adapter>::serial_t;
    using bytes_t = typename adapters::serial_traits<Adapter>::bytes_t;

    static serial_t from_bytes(bytes_t&& bytes) = delete;
    static bytes_t to_bytes(const serial_t& serial_obj) = delete;

    static std::string get_func_name(const serial_t& serial_obj) = delete;
    static rpc_object_type get_type(const serial_t& serial_obj) = delete;

    template<typename R>
    static func_result<R> get_result(const serial_t& serial_obj) = delete;

    template<typename R>
    static serial_t serialize_result(const func_result<R>& result) = delete;

    template<typename R, typename... Args>
    static serial_t serialize_result_w_bind(const func_result_w_bind<R, Args...>& result) = delete;

    template<typename... Args>
    static func_request<Args...> get_request(const serial_t& serial_obj) = delete;

    template<typename... Args>
    static serial_t serialize_request(const func_request<Args...>& request) = delete;

    static func_error get_error(const serial_t& serial_obj) = delete;
    static serial_t serialize_error(const func_error& error) = delete;
    static callback_install_request get_callback_install(const serial_t& serial_obj) = delete;
    static serial_t serialize_callback_install(
        const callback_install_request& callback_req) = delete;

    static bool has_bound_args(const serial_t& serial_obj) = delete;
};
} //namespace rpc_hpp
