#ifndef RPC_HPP
#define RPC_HPP

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

#if defined(__GNUC__)
#  define RPC_HPP_INLINE [[gnu::always_inline]]
#elif defined(_MSC_VER)
#  define RPC_HPP_INLINE __forceinline
#else
#  define RPC_HPP_INLINE
#endif

#if defined(RPC_HPP_SERVER_IMPL)
#  define RPC_HEADER_FUNC(RT, FNAME, ...) RT FNAME(__VA_ARGS__)
#  define RPC_HEADER_FUNC_EXTC(RT, FNAME, ...) extern "C" RT FNAME(__VA_ARGS__)
#  define RPC_HEADER_FUNC_NOEXCEPT(RT, FNAME, ...) RT FNAME(__VA_ARGS__) noexcept
#elif defined(RPC_HPP_CLIENT_IMPL)
#  define RPC_HEADER_FUNC(RT, FNAME, ...) inline RT (*FNAME)(__VA_ARGS__) = nullptr
#  define RPC_HEADER_FUNC_EXTC(RT, FNAME, ...) extern "C" inline RT (*FNAME)(__VA_ARGS__) = nullptr
#  define RPC_HEADER_FUNC_NOEXCEPT(RT, FNAME, ...) \
    inline RT (*FNAME)(__VA_ARGS__) noexcept = nullptr
#endif

namespace rpc_hpp
{
enum class exception_type : int
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
    explicit rpc_exception(const std::string& mesg, const exception_type type)
        : std::runtime_error(mesg), m_type(type)
    {
    }

    explicit rpc_exception(const char* const mesg, const exception_type type)
        : std::runtime_error(mesg), m_type(type)
    {
    }

    [[nodiscard]] exception_type get_type() const noexcept { return m_type; }

private:
    exception_type m_type;
};

class function_not_found : public rpc_exception
{
public:
    explicit function_not_found(const std::string& mesg)
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }

    explicit function_not_found(const char* const mesg)
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }
};

class remote_exec_error : public rpc_exception
{
public:
    explicit remote_exec_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }

    explicit remote_exec_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }
};

class serialization_error : public rpc_exception
{
public:
    explicit serialization_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::serialization)
    {
    }

    explicit serialization_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::serialization)
    {
    }
};

class deserialization_error : public rpc_exception
{
public:
    explicit deserialization_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }

    explicit deserialization_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }
};

class function_mismatch : public rpc_exception
{
public:
    explicit function_mismatch(const std::string& mesg)
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }

    explicit function_mismatch(const char* const mesg)
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }
};

class client_send_error : public rpc_exception
{
public:
    explicit client_send_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::client_send)
    {
    }

    explicit client_send_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::client_send)
    {
    }
};

class client_receive_error : public rpc_exception
{
public:
    explicit client_receive_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }

    explicit client_receive_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }
};

class server_send_error : public rpc_exception
{
public:
    explicit server_send_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::server_send)
    {
    }

    explicit server_send_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::server_send)
    {
    }
};

class server_receive_error : public rpc_exception
{
public:
    explicit server_receive_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }

    explicit server_receive_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }
};

class rpc_object_mismatch : public rpc_exception
{
public:
    explicit rpc_object_mismatch(const std::string& mesg)
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }

    explicit rpc_object_mismatch(const char* const mesg)
        : rpc_exception(mesg, exception_type::rpc_object_mismatch)
    {
    }
};

class callback_install_error : public rpc_exception
{
public:
    explicit callback_install_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::callback_install)
    {
    }

    explicit callback_install_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::callback_install)
    {
    }
};

class callback_missing_error : public rpc_exception
{
public:
    explicit callback_missing_error(const std::string& mesg)
        : rpc_exception(mesg, exception_type::callback_missing)
    {
    }

    explicit callback_missing_error(const char* const mesg)
        : rpc_exception(mesg, exception_type::callback_missing)
    {
    }
};

namespace detail
{
#if __has_cpp_attribute(__cpp_lib_remove_cvref)
    using std::remove_cvref;
    using std::remove_cvref_t;
#else
    // backport of C++20's remove_cvref
    template<typename T>
    struct remove_cvref
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;
#endif

    template<typename T>
    struct is_boolean_testable : std::bool_constant<std::is_convertible_v<T, bool>>
    {
    };

    template<typename T>
    inline constexpr bool is_boolean_testable_v = is_boolean_testable<T>::value;

    template<typename T>
    struct is_stringlike :
        std::bool_constant<(
            std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::string_view>)>
    {
    };

    template<typename T>
    inline constexpr bool is_stringlike_v = is_stringlike<T>::value;

    template<typename T>
    struct decay_str
    {
        static_assert(!std::is_pointer_v<remove_cvref_t<T>>, "Pointer parameters are not allowed");
        static_assert(!std::is_array_v<remove_cvref_t<T>>, "C array parameters are not allowed");

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

    template<>
    struct decay_str<std::string_view>
    {
        using type = const std::string&;
    };

    template<typename T>
    using decay_str_t = typename decay_str<T>::type;

    template<typename C>
    struct has_begin
    {
    private:
        template<typename T>
        static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept -> std::bool_constant<
            std::is_same_v<decltype(std::declval<T>().begin()), typename T::iterator>>;

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
        static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept -> std::bool_constant<
            std::is_same_v<decltype(std::declval<T>().end()), typename T::iterator>>;

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
        static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept
            -> std::bool_constant<std::is_same_v<decltype(std::declval<T>().size()), size_t>>;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct is_container : std::bool_constant<has_begin<C>::value && has_end<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_container_v = is_container<std::remove_cv_t<C>>::value;

    template<typename C>
    struct has_map_iterator
    {
    private:
        template<typename T>
        static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept
            -> std::bool_constant<std::is_same_v<
                decltype(std::declval<typename T::iterator>()->second), typename T::mapped_type>>;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct has_map_at
    {
    private:
        template<typename T>
        static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept
            -> std::bool_constant<std::is_same_v<
                decltype(std::declval<T>().at(typename T::key_type{})), typename T::mapped_type&>>;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct is_map : std::bool_constant<is_container_v<C> && has_map_iterator<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_map_v = is_map<std::remove_cv_t<C>>::value;

    template<typename C>
    struct is_multimap : std::bool_constant<is_map_v<C> && (!has_map_at<C>::value)>
    {
    };

    template<typename C>
    inline constexpr bool is_multimap_v = is_multimap<std::remove_cv_t<C>>::value;

    template<typename C>
    struct has_set_key
    {
    private:
        template<typename T>
        static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept
            -> std::bool_constant<std::is_same_v<typename T::value_type, typename T::key_type>>;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct is_set : std::bool_constant<is_container_v<C> && has_set_key<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_set_v = is_set<std::remove_cv_t<C>>::value;

    template<typename C>
    struct is_optional : std::false_type
    {
    };

    template<typename T>
    struct is_optional<std::optional<T>> : std::true_type
    {
    };

    template<typename C>
    inline constexpr bool is_optional_v = is_optional<std::remove_cv_t<C>>::value;

    template<typename C>
    struct is_pair : std::false_type
    {
    };

    template<typename T1, typename T2>
    struct is_pair<std::pair<T1, T2>> : std::true_type
    {
    };

    template<typename C>
    inline constexpr bool is_pair_v = is_pair<std::remove_cv_t<C>>::value;

    template<typename F, typename... Ts, size_t... Is>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, F&& func,
        RPC_HPP_UNUSED const std::index_sequence<Is...> iseq)
    {
        using expander = int[];
        std::ignore = expander{ 0, ((void)std::forward<F>(func)(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    RPC_HPP_INLINE constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, F&& func)
    {
        for_each_tuple(tuple, std::forward<F>(func), std::make_index_sequence<sizeof...(Ts)>());
    }

    template<typename T>
    constexpr bool is_ref_arg()
    {
        return std::is_reference_v<
                   T> && (!std::is_const_v<std::remove_reference_t<T>>)&&(!std::is_pointer_v<std::remove_reference_t<T>>);
    }

    template<typename... Args>
    constexpr bool has_ref_args()
    {
        return (... || is_ref_arg<Args>());
    }

    template<typename... Args, size_t... Is>
    constexpr void tuple_bind(const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src,
        RPC_HPP_UNUSED const std::index_sequence<Is...> iseq, Args&&... dest)
    {
        using expander = int[];
        std::ignore = expander{ 0,
            (
                (void)[](auto&& bound_arg, auto&& ref_arg) {
                    if constexpr (is_ref_arg<decltype(bound_arg)>())
                    {
                        std::forward<decltype(bound_arg)>(bound_arg) =
                            std::forward<decltype(ref_arg)>(ref_arg);
                    }
                }(dest, std::get<Is>(src)),
                0)... };
    }

    template<typename... Args>
    RPC_HPP_INLINE constexpr void tuple_bind(
        const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src, Args&&... dest)
    {
        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }

    template<typename R, typename... Args>
    using fptr_t = R (*)(Args...);

    template<bool IsCallback>
    struct rpc_base
    {
        static constexpr bool is_callback = IsCallback;
        std::string func_name{};
    };

    template<bool IsCallback, typename... Args>
    struct rpc_request : rpc_base<IsCallback>
    {
        using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;

        rpc_request() noexcept = default;

        rpc_request(std::string t_func_name, args_t t_args, bool t_bind_args = false)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              bind_args(t_bind_args),
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
        R result{};
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
    struct rpc_result_w_bind : rpc_result<IsCallback, R>
    {
        using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;

        rpc_result_w_bind() noexcept = default;
        rpc_result_w_bind(std::string t_func_name, R t_result, args_t t_args)
            : rpc_result<IsCallback, R>{ std::move(t_func_name), std::move(t_result) },
              args(std::move(t_args))
        {
        }

        args_t args{};
    };

    template<bool IsCallback, typename... Args>
    struct rpc_result_w_bind<IsCallback, void, Args...> : rpc_request<IsCallback, Args...>
    {
        using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;
        using rpc_request<IsCallback, Args...>::rpc_request;
    };

    template<typename R, typename... Args>
    using func_result_w_bind = rpc_result_w_bind<false, R, Args...>;

    template<typename R, typename... Args>
    using callback_result_w_bind = rpc_result_w_bind<true, R, Args...>;

    template<bool IsCallback>
    struct rpc_error : rpc_base<IsCallback>
    {
        rpc_error() noexcept = default;
        rpc_error(std::string t_func_name, const rpc_exception& except)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              except_type(except.get_type()),
              err_mesg(except.what())
        {
        }

        rpc_error(std::string t_func_name, const exception_type t_ex_type, std::string t_err_mesg)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              except_type(t_ex_type),
              err_mesg(std::move(t_err_mesg))
        {
        }

        exception_type except_type{ exception_type::none };
        std::string err_mesg{};
    };

    template<bool IsCallback>
    [[noreturn]] void rpc_throw(const rpc_error<IsCallback>& err) noexcept(false)
    {
        switch (err.except_type)
        {
            case exception_type::func_not_found:
                throw function_not_found{ err.err_mesg };

            case exception_type::remote_exec:
                throw remote_exec_error{ err.err_mesg };

            case exception_type::serialization:
                throw serialization_error{ err.err_mesg };

            case exception_type::deserialization:
                throw deserialization_error{ err.err_mesg };

            case exception_type::signature_mismatch:
                throw function_mismatch{ err.err_mesg };

            case exception_type::client_send:
                throw client_send_error{ err.err_mesg };

            case exception_type::client_receive:
                throw client_receive_error{ err.err_mesg };

            case exception_type::server_send:
                throw server_send_error{ err.err_mesg };

            case exception_type::server_receive:
                throw server_receive_error{ err.err_mesg };

            case exception_type::rpc_object_mismatch:
                throw rpc_object_mismatch{ err.err_mesg };

            case exception_type::callback_install:
                throw callback_install_error{ err.err_mesg };

            case exception_type::callback_missing:
                throw callback_missing_error{ err.err_mesg };

            case exception_type::none:
            default:
                throw rpc_exception{ err.err_mesg, exception_type::none };
        }
    }

    using func_error = rpc_error<false>;
    using callback_error = rpc_error<true>;
} //namespace detail

struct callback_install_request : detail::rpc_base<true>
{
    callback_install_request() noexcept = default;
    explicit callback_install_request(std::string t_func_name) noexcept
        : rpc_base<true>{ std::move(t_func_name) }
    {
    }

    bool is_uninstall{ false };
};

enum class rpc_type : int
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

    RPC_HPP_NODISCARD("converting to bytes may be expensive")
    bytes_t to_bytes() const& { return Serial::to_bytes(m_obj); }

    RPC_HPP_NODISCARD("converting to bytes consumes object")
    bytes_t to_bytes() && { return Serial::to_bytes(std::move(m_obj)); }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    std::string get_func_name() const { return Serial::get_func_name(m_obj); }

    template<typename R>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    R get_result() const
    {
        switch (type())
        {
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
                if constexpr (std::is_void_v<R>)
                {
                    return;
                }
                else
                {
                    return Serial::template get_result<false, R>(m_obj).result;
                }

            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
                if constexpr (std::is_void_v<R>)
                {
                    return;
                }
                else
                {
                    return Serial::template get_result<true, R>(m_obj).result;
                }

            case rpc_type::func_error:
                detail::rpc_throw(Serial::template get_error<false>(m_obj));

            case rpc_type::callback_error:
                detail::rpc_throw(Serial::template get_error<true>(m_obj));

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::func_request:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    template<typename... Args>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_args() const
    {
        switch (type())
        {
            case rpc_type::func_request:
            case rpc_type::func_result_w_bind:
                return Serial::template get_request<false, Args...>(m_obj).args;

            case rpc_type::callback_request:
            case rpc_type::callback_result_w_bind:
                return Serial::template get_request<true, Args...>(m_obj).args;

            case rpc_type::callback_error:
            case rpc_type::callback_install_request:
            case rpc_type::callback_result:
            case rpc_type::func_error:
            case rpc_type::func_result:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool is_callback_uninstall() const
    {
        return type() == rpc_type::callback_install_request
            ? Serial::get_callback_install(m_obj).is_uninstall
            : false;
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    exception_type get_error_type() const
    {
        switch (type())
        {
            case rpc_type::callback_error:
                return Serial::template get_error<true>(m_obj).except_type;

            case rpc_type::func_error:
                return Serial::template get_error<false>(m_obj).except_type;

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
            case rpc_type::func_request:
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    template<bool IsCallback = false>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    std::string get_error_mesg() const
    {
        switch (type())
        {
            case rpc_type::callback_error:
                return Serial::template get_error<true>(m_obj).err_mesg;

            case rpc_type::func_error:
                return Serial::template get_error<false>(m_obj).err_mesg;

            case rpc_type::callback_install_request:
            case rpc_type::callback_request:
            case rpc_type::callback_result:
            case rpc_type::callback_result_w_bind:
            case rpc_type::func_request:
            case rpc_type::func_result:
            case rpc_type::func_result_w_bind:
            default:
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
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
                throw rpc_object_mismatch{ "Invalid rpc_object type detected" };
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    bool is_error() const
    {
        const auto rtype = type();
        return (rtype == rpc_type::func_error) || (rtype == rpc_type::callback_error);
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    rpc_type type() const { return Serial::get_type(m_obj); }

private:
    explicit rpc_object(const serial_t& serial) : m_obj(serial) {}
    explicit rpc_object(serial_t&& serial) noexcept : m_obj(std::move(serial)) {}

    serial_t m_obj;
};

namespace adapters
{
    template<typename Adapter>
    struct serial_adapter_base
    {
        using bytes_t = typename Adapter::bytes_t;
        using serial_t = typename Adapter::serial_t;
        using serializer_t = typename Adapter::serializer_t;
        using deserializer_t = typename Adapter::deserializer_t;
        using config = typename Adapter::config;

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

    template<typename Derived>
    class generic_serializer
    {
    public:
        generic_serializer() noexcept = default;
        generic_serializer& operator=(const generic_serializer&) = delete;
        generic_serializer& operator=(generic_serializer&&) = delete;

        template<typename T>
        RPC_HPP_INLINE void as_bool(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_bool(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_float(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_float(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_int(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_int(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_string(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_string(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_array(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_array(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_map(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_map(key, val);
        }

        template<typename T1, typename T2>
        RPC_HPP_INLINE void as_tuple(const std::string_view key, std::pair<T1, T2>& val)
        {
            (static_cast<Derived*>(this))->as_tuple(key, val);
        }

        template<typename... Args>
        RPC_HPP_INLINE void as_tuple(const std::string_view key, std::tuple<Args...>& val)
        {
            (static_cast<Derived*>(this))->as_tuple(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_optional(const std::string_view key, std::optional<T>& val)
        {
            (static_cast<Derived*>(this))->as_optional(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_object(const std::string_view key, T& val)
        {
            (static_cast<Derived*>(this))->as_object(key, val);
        }

    protected:
        ~generic_serializer() noexcept = default;
        generic_serializer(const generic_serializer&) = default;
        generic_serializer(generic_serializer&&) noexcept = default;
    };

    template<typename Adapter, bool Deserialize>
    class serializer_base : public generic_serializer<serializer_base<Adapter, Deserialize>>
    {
    public:
        using serializer_t = std::conditional_t<Deserialize, typename Adapter::deserializer_t,
            typename Adapter::serializer_t>;

        template<typename T>
        void serialize_object(const T& val)
        {
            static_assert(!Deserialize, "Cannot call serialize_object() on a deserializer");

            // Necessary for bi-directional serialization
            serialize(*this, const_cast<T&>(val)); // NOLINT(cppcoreguidelines-pro-type-const-cast)
        }

        template<typename T>
        void deserialize_object(T&& val)
        {
            static_assert(Deserialize, "Cannot call deserialize_object() on a serializer");
            serialize(*this, std::forward<T>(val));
        }

        template<typename T>
        RPC_HPP_INLINE void as_bool(const std::string_view key, T& val)
        {
            static_assert(detail::is_boolean_testable_v<T>, "T must be convertible to bool");
            (static_cast<serializer_t*>(this))->as_bool(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_float(const std::string_view key, T& val)
        {
            static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");
            (static_cast<serializer_t*>(this))->as_float(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_int(const std::string_view key, T& val)
        {
            static_assert(std::is_integral_v<T> || std::is_enum_v<T>, "T must be an integral type");
            (static_cast<serializer_t*>(this))->as_int(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_string(const std::string_view key, T& val)
        {
            static_assert(detail::is_stringlike_v<T>, "T must be convertible to std::string_view");
            (static_cast<serializer_t*>(this))->as_string(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_array(const std::string_view key, T& val)
        {
            static_assert(detail::is_container_v<T>, "T must have begin() and end()");
            (static_cast<serializer_t*>(this))->as_array(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_map(const std::string_view key, T& val)
        {
            static_assert(detail::is_map_v<T>, "T must be a map type");

            if constexpr (detail::is_multimap_v<T>)
            {
                (static_cast<serializer_t*>(this))->as_multimap(key, val);
            }
            else
            {
                (static_cast<serializer_t*>(this))->as_map(key, val);
            }
        }

        template<typename T1, typename T2>
        RPC_HPP_INLINE void as_tuple(const std::string_view key, std::pair<T1, T2>& val)
        {
            (static_cast<serializer_t*>(this))->as_tuple(key, val);
        }

        template<typename... Args>
        RPC_HPP_INLINE void as_tuple(const std::string_view key, std::tuple<Args...>& val)
        {
            (static_cast<serializer_t*>(this))->as_tuple(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_optional(const std::string_view key, std::optional<T>& val)
        {
            (static_cast<serializer_t*>(this))->as_optional(key, val);
        }

        template<typename T>
        RPC_HPP_INLINE void as_object(const std::string_view key, T& val)
        {
            (static_cast<serializer_t*>(this))->as_object(key, val);
        }
    };

    // Overloads for common types
    template<typename Adapter>
    void serialize(serializer_base<Adapter, false>& ser, const bool val)
    {
        ser.as_bool("", val);
    }

    template<typename Adapter>
    void serialize(serializer_base<Adapter, true>& ser, bool& val)
    {
        ser.as_bool("", val);
    }

    template<typename Adapter, typename T,
        std::enable_if_t<(std::is_integral_v<T> && (!std::is_same_v<T, bool>)), bool> = true>
    void serialize(serializer_base<Adapter, false>& ser, const T val)
    {
        ser.as_int("", val);
    }

    template<typename Adapter, typename T,
        std::enable_if_t<(std::is_integral_v<T> && (!std::is_same_v<T, bool>)), bool> = true>
    void serialize(serializer_base<Adapter, true>& ser, T& val)
    {
        ser.as_int("", val);
    }

    template<typename Adapter, typename T,
        std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    void serialize(serializer_base<Adapter, false>& ser, const T val)
    {
        ser.as_float("", val);
    }

    template<typename Adapter, typename T,
        std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    void serialize(serializer_base<Adapter, true>& ser, T& val)
    {
        ser.as_float("", val);
    }

    template<typename Adapter>
    void serialize(serializer_base<Adapter, false>& ser, const std::string_view val)
    {
        ser.as_string("", val);
    }

    template<typename Adapter, typename T,
        std::enable_if_t<detail::is_stringlike_v<T>, bool> = true>
    void serialize(serializer_base<Adapter, true>& ser, T& val)
    {
        ser.as_string("", val);
    }

    template<typename Adapter, bool Deserialize, typename T,
        std::enable_if_t<
            (detail::is_container_v<T> && (!detail::is_stringlike_v<T>)&&(!detail::is_map_v<T>)),
            bool> = true>
    void serialize(serializer_base<Adapter, Deserialize>& ser, T& val)
    {
        ser.as_array("", val);
    }

    template<typename Adapter, bool Deserialize, typename T,
        std::enable_if_t<detail::is_map_v<T>, bool> = true>
    void serialize(serializer_base<Adapter, Deserialize>& ser, T& val)
    {
        ser.as_map("", val);
    }

    template<typename Adapter, bool Deserialize, typename T1, typename T2>
    void serialize(serializer_base<Adapter, Deserialize>& ser, std::pair<T1, T2>& val)
    {
        ser.as_tuple("", val);
    }

    template<typename Adapter, bool Deserialize, typename... Args>
    void serialize(serializer_base<Adapter, Deserialize>& ser, std::tuple<Args...>& val)
    {
        ser.as_tuple("", val);
    }

    template<typename Adapter, bool Deserialize, typename T>
    void serialize(serializer_base<Adapter, Deserialize>& ser, std::optional<T>& val)
    {
        ser.as_optional("", val);
    }
} //namespace adapters

namespace detail
{
    template<bool IsCallback, typename Serial, typename R, typename... Args, typename F>
    void exec_func(F&& func, rpc_object<Serial>& rpc_obj)
    {
        auto args = rpc_obj.template get_args<Args...>();
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
            throw remote_exec_error{ ex.what() };
        }
    }

    // serialize functions for library types
    template<typename Adapter, bool Deserialize, bool IsCallback, typename... Args>
    void serialize(adapters::serializer_base<Adapter, Deserialize>& ser,
        rpc_request<IsCallback, Args...>& rpc_obj)
    {
        auto type = []
        {
            if constexpr (IsCallback)
            {
                return static_cast<int>(rpc_type::callback_request);
            }
            else
            {
                return static_cast<int>(rpc_type::func_request);
            }
        }();

        ser.as_int("type", type);
        ser.as_string("func_name", rpc_obj.func_name);
        ser.as_bool("bind_args", rpc_obj.bind_args);
        ser.as_tuple("args", rpc_obj.args);
    }

    template<typename Adapter, bool Deserialize, bool IsCallback, typename R>
    void serialize(
        adapters::serializer_base<Adapter, Deserialize>& ser, rpc_result<IsCallback, R>& rpc_obj)
    {
        auto type = []
        {
            if constexpr (IsCallback)
            {
                return static_cast<int>(rpc_type::callback_result);
            }
            else
            {
                return static_cast<int>(rpc_type::func_result);
            }
        }();

        ser.as_int("type", type);
        ser.as_string("func_name", rpc_obj.func_name);

        if constexpr (!std::is_void_v<R>)
        {
            ser.as_object("result", rpc_obj.result);
        }
    }

    template<typename Adapter, bool Deserialize, bool IsCallback, typename R, typename... Args>
    void serialize(adapters::serializer_base<Adapter, Deserialize>& ser,
        rpc_result_w_bind<IsCallback, R, Args...>& rpc_obj)
    {
        auto type = []
        {
            if constexpr (IsCallback)
            {
                return static_cast<int>(rpc_type::callback_result_w_bind);
            }
            else
            {
                return static_cast<int>(rpc_type::func_result_w_bind);
            }
        }();

        bool bind_args = true;

        ser.as_int("type", type);
        ser.as_string("func_name", rpc_obj.func_name);
        ser.as_bool("bind_args", bind_args);
        ser.as_tuple("args", rpc_obj.args);

        if constexpr (!std::is_void_v<R>)
        {
            ser.as_object("result", rpc_obj.result);
        }
    }

    template<typename Adapter, bool Deserialize, bool IsCallback>
    void serialize(
        adapters::serializer_base<Adapter, Deserialize>& ser, rpc_error<IsCallback>& rpc_obj)
    {
        auto type = []
        {
            if constexpr (IsCallback)
            {
                return static_cast<int>(rpc_type::callback_error);
            }
            else
            {
                return static_cast<int>(rpc_type::func_error);
            }
        }();

        ser.as_int("type", type);
        ser.as_string("func_name", rpc_obj.func_name);
        ser.as_int("except_type", rpc_obj.except_type);
        ser.as_string("err_mesg", rpc_obj.err_mesg);
    }
} //namespace detail

template<typename Adapter, bool Deserialize>
void serialize(
    adapters::serializer_base<Adapter, Deserialize>& ser, callback_install_request& rpc_obj)
{
    auto type = static_cast<int>(rpc_type::callback_install_request);
    ser.as_int("type", type);
    ser.as_string("func_name", rpc_obj.func_name);
    ser.as_bool("is_uninstall", rpc_obj.is_uninstall);
}
} //namespace rpc_hpp
#endif
