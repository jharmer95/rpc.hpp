#ifndef RPC_HPP
#define RPC_HPP

#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#if defined(_MSC_VER)
#  define RPC_HPP_ASSUME(EXPR) __assume(EXPR)
#elif defined(__clang__)
#  define RPC_HPP_ASSUME(EXPR) __builtin_assume(EXPR)
#elif defined(__GNUC__)
#  define RPC_HPP_ASSUME(EXPR) (EXPR) ? static_cast<void>(0) : __builtin_unreachable()
#else
#  define RPC_HPP_ASSUME(EXPR) static_cast<void>(0)
#endif

#if defined(RPC_HPP_ASSERT_NONE)
#  define RPC_HPP_ASSERTION(EXPR) static_cast<void>(0)
#elif defined(RPC_HPP_ASSERT_DEBUG)
#  define RPC_HPP_ASSERTION(EXPR) assert(EXPR)
#elif defined(RPC_HPP_ASSERT_STDERR)
#  define RPC_HPP_ASSERTION(EXPR)                                                             \
    if (!(EXPR))                                                                              \
    std::fprintf(stderr,                                                                      \
        "RPC_HPP_ASSERTION: \"%s\" failed!\n  func: %s,\n  file: %s,\n  line: %d\n\n", #EXPR, \
        __FUNCTION__, __FILE__, __LINE__)
#elif defined(RPC_HPP_ASSERT_THROW)
#  define RPC_HPP_ASSERTION(EXPR) \
    if (!(EXPR))                  \
    throw std::runtime_error("RPC_HPP_ASSERTION: \"" #EXPR "\" failed!")
#elif defined(RPC_HPP_ASSERT_ABORT)
#  define RPC_HPP_ASSERTION(EXPR) \
    if (!(EXPR))                  \
    std::abort()
#elif defined(RPC_HPP_ASSERT_ASSUME)
#  define RPC_HPP_ASSERTION(EXPR) RPC_HPP_ASSUME(EXPR)
#else
#  define RPC_HPP_ASSERTION(EXPR) assert(EXPR)
#endif

#define RPC_HPP_POSTCONDITION(EXPR) RPC_HPP_ASSERTION(EXPR)
#define RPC_HPP_PRECONDITION(EXPR) RPC_HPP_ASSERTION(EXPR)

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

namespace rpc_hpp
{
enum class exception_type : int
{
    none,
    function_missing,
    remote_exec,
    serialization,
    deserialization,
    function_bind,
    func_signature_mismatch,
    client_send,
    client_receive,
    server_send,
    server_receive,
    rpc_object_mismatch,
    callback_install,
    callback_missing,
};

[[nodiscard]] constexpr auto validate_exception_type(exception_type type) noexcept -> bool
{
    return static_cast<int>(type) >= static_cast<int>(exception_type::none)
        && static_cast<int>(type) <= static_cast<int>(exception_type::callback_missing);
}

// invariants:
//   1. m_type must be a valid exception_type after construction
class rpc_exception : public std::runtime_error
{
public:
    explicit rpc_exception(const std::string& mesg, const exception_type type)
        : std::runtime_error(mesg), m_type(type)
    {
        RPC_HPP_POSTCONDITION(validate_exception_type(m_type));
    }

    explicit rpc_exception(const char* const mesg, const exception_type type)
        : std::runtime_error(mesg), m_type(type)
    {
        RPC_HPP_POSTCONDITION(validate_exception_type(m_type));
    }

    [[nodiscard]] exception_type get_type() const noexcept { return m_type; }

private:
    exception_type m_type;
};

#define RPC_HPP_DECLARE_EXCEPTION(ENAME, ETYPE)                          \
  class ENAME : public rpc_exception                                     \
  {                                                                      \
public:                                                                  \
    explicit ENAME(const std::string& mesg) : rpc_exception(mesg, ETYPE) \
    {                                                                    \
    }                                                                    \
    explicit ENAME(const char* const mesg) : rpc_exception(mesg, ETYPE)  \
    {                                                                    \
    }                                                                    \
  }

RPC_HPP_DECLARE_EXCEPTION(function_missing_error, exception_type::function_missing);
RPC_HPP_DECLARE_EXCEPTION(remote_exec_error, exception_type::remote_exec);
RPC_HPP_DECLARE_EXCEPTION(serialization_error, exception_type::serialization);
RPC_HPP_DECLARE_EXCEPTION(deserialization_error, exception_type::deserialization);
RPC_HPP_DECLARE_EXCEPTION(function_bind_error, exception_type::function_bind);
RPC_HPP_DECLARE_EXCEPTION(function_mismatch_error, exception_type::func_signature_mismatch);
RPC_HPP_DECLARE_EXCEPTION(client_send_error, exception_type::client_send);
RPC_HPP_DECLARE_EXCEPTION(client_receive_error, exception_type::client_receive);
RPC_HPP_DECLARE_EXCEPTION(server_send_error, exception_type::server_send);
RPC_HPP_DECLARE_EXCEPTION(server_receive_error, exception_type::server_receive);
RPC_HPP_DECLARE_EXCEPTION(object_mismatch_error, exception_type::rpc_object_mismatch);
RPC_HPP_DECLARE_EXCEPTION(callback_install_error, exception_type::callback_install);
RPC_HPP_DECLARE_EXCEPTION(callback_missing_error, exception_type::callback_missing);
#undef RPC_HPP_DECLARE_EXCEPTION

namespace detail
{
#define RPC_HPP_NOPAREN(...) __VA_ARGS__
#define RPC_HPP_CHECKER(NAME, EXPR1, EXPR2)                       \
  template<typename C>                                            \
  struct NAME                                                     \
  {                                                               \
private:                                                          \
    template<typename T>                                          \
    static constexpr auto check(RPC_HPP_UNUSED T* ptr) noexcept   \
        -> std::is_same<decltype(EXPR1), EXPR2>;                  \
    template<typename>                                            \
    static constexpr auto check(...) noexcept -> std::false_type; \
    using type = decltype(check<C>(nullptr));                     \
                                                                  \
public:                                                           \
    static constexpr bool value = type::value;                    \
  }

    RPC_HPP_CHECKER(has_begin, std::declval<T>().begin(), typename T::iterator);
    RPC_HPP_CHECKER(has_end, std::declval<T>().end(), typename T::iterator);
    RPC_HPP_CHECKER(has_size, std::declval<T>().size(), typename T::iterator);
    RPC_HPP_CHECKER(has_set_key, typename T::value_type{}, typename T::key_type);
    RPC_HPP_CHECKER(
        has_map_iterator, std::declval<typename T::iterator>()->second, typename T::mapped_type);
    RPC_HPP_CHECKER(
        has_map_at, std::declval<T>().at(typename T::key_type{}), typename T::mapped_type);
#undef RPC_HPP_CHECKER

#define RPC_HPP_TYPE_TRAIT(NAME, COND)     \
  template<typename C>                     \
  struct NAME : std::bool_constant<(COND)> \
  {                                        \
  };                                       \
  template<typename C>                     \
  inline constexpr bool NAME##_v = NAME<C>::value

#define RPC_HPP_CONJ_TYPE_TRAIT(NAME, ...) \
  RPC_HPP_TYPE_TRAIT(NAME, std::conjunction<RPC_HPP_NOPAREN(__VA_ARGS__)>::value)

#define RPC_HPP_DISJ_TYPE_TRAIT(NAME, ...) \
  RPC_HPP_TYPE_TRAIT(NAME, std::disjunction<RPC_HPP_NOPAREN(__VA_ARGS__)>::value)

    RPC_HPP_TYPE_TRAIT(is_boolean_testable, (std::is_convertible_v<C, bool>));
    RPC_HPP_DISJ_TYPE_TRAIT(is_stringlike, std::is_convertible<C, std::string>,
        std::is_convertible<C, std::string_view>);

    RPC_HPP_CONJ_TYPE_TRAIT(
        is_container, has_begin<std::remove_cv_t<C>>, has_end<std::remove_cv_t<C>>);

    RPC_HPP_CONJ_TYPE_TRAIT(is_map, is_container<C>, has_map_iterator<std::remove_cv_t<C>>);
    RPC_HPP_CONJ_TYPE_TRAIT(is_multimap, is_map<C>, std::negation<has_map_at<C>>);
    RPC_HPP_CONJ_TYPE_TRAIT(is_set, is_container<C>, has_set_key<std::remove_cv_t<C>>);
    RPC_HPP_CONJ_TYPE_TRAIT(is_ref_arg, std::is_reference<C>,
        std::negation<std::is_const<std::remove_reference_t<C>>>,
        std::negation<std::is_pointer<std::remove_reference_t<C>>>);
#undef RPC_HPP_TYPE_TRAIT
#undef RPC_HPP_CONJ_TYPE_TRAIT
#undef RPC_HPP_DISJ_TYPE_TRAIT

#if defined(__cpp_lib_remove_cvref)
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
        static_assert(std::conjunction_v<std::is_invocable<F, Ts>...>,
            "applied function must be able to take given args");

        using expander = int[];
        std::ignore = expander{ 0, ((void)std::forward<F>(func)(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, F&& func)
    {
        static_assert(std::conjunction_v<std::is_invocable<F, Ts>...>,
            "applied function must be able to take given args");

        for_each_tuple(tuple, std::forward<F>(func), std::make_index_sequence<sizeof...(Ts)>());
    }

    template<typename... Args, size_t... Is>
    constexpr void tuple_bind(const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src,
        RPC_HPP_UNUSED const std::index_sequence<Is...> iseq, Args&&... dest)
    {
        static_assert(sizeof...(Args) == sizeof...(Is),
            "index sequence length must be the same as the number of arguments");

        static_assert(std::disjunction_v<is_ref_arg<Args>...>,
            "At least one argument must be a (non-const) reference for tuple_bind to have an "
            "effect");

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
    constexpr void tuple_bind(
        const std::tuple<remove_cvref_t<decay_str_t<Args>>...>& src, Args&&... dest)
    {
        static_assert(std::disjunction_v<is_ref_arg<Args>...>,
            "At least one argument must be a (non-const) reference for tuple_bind to have an "
            "effect");

        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }

    template<typename R, typename... Args>
    using fptr_t = R (*)(Args...);

    // invariants: none
    template<bool IsCallback>
    struct rpc_base
    {
        static constexpr bool is_callback = IsCallback;
        std::string func_name{};
    };

    // invariants:
    //   1. func_name cannot be empty after construction
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
            RPC_HPP_POSTCONDITION(!this->func_name.empty());
        }

        bool bind_args{ false };
        args_t args{};
    };

    template<typename... Args>
    using func_request = rpc_request<false, Args...>;

    template<typename... Args>
    using callback_request = rpc_request<true, Args...>;

    // invariants:
    //   1. func_name cannot be empty after construction
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

    // invariants:
    //   1. func_name cannot be empty after construction
    template<bool IsCallback, typename R, typename... Args>
    struct rpc_result_w_bind : rpc_result<IsCallback, R>
    {
        using args_t = std::tuple<remove_cvref_t<decay_str_t<Args>>...>;

        rpc_result_w_bind() noexcept = default;
        rpc_result_w_bind(std::string t_func_name, R t_result, args_t t_args)
            : rpc_result<IsCallback, R>{ std::move(t_func_name), std::move(t_result) },
              args(std::move(t_args))
        {
            RPC_HPP_POSTCONDITION(!this->func_name.empty());
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

    // invariants:
    //   1. except_type must hold a valid exception_type
    template<bool IsCallback>
    struct rpc_error : rpc_base<IsCallback>
    {
        rpc_error() noexcept = default;
        rpc_error(std::string t_func_name, const rpc_exception& except)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              except_type(except.get_type()),
              err_mesg(except.what())
        {
            RPC_HPP_POSTCONDITION(validate_exception_type(except_type));
        }

        rpc_error(std::string t_func_name, const exception_type t_ex_type, std::string t_err_mesg)
            : rpc_base<IsCallback>{ std::move(t_func_name) },
              except_type(t_ex_type),
              err_mesg(std::move(t_err_mesg))
        {
            RPC_HPP_POSTCONDITION(validate_exception_type(except_type));
        }

        exception_type except_type{ exception_type::none };
        std::string err_mesg{};
    };

    using func_error = rpc_error<false>;
    using callback_error = rpc_error<true>;

    template<bool IsCallback>
    [[noreturn]] void rpc_throw(const rpc_error<IsCallback>& err) noexcept(false)
    {
        RPC_HPP_PRECONDITION(validate_exception_type(err.except_type));

        switch (err.except_type)
        {
            case exception_type::function_missing:
                throw function_missing_error{ err.err_mesg };

            case exception_type::remote_exec:
                throw remote_exec_error{ err.err_mesg };

            case exception_type::serialization:
                throw serialization_error{ err.err_mesg };

            case exception_type::deserialization:
                throw deserialization_error{ err.err_mesg };

            case exception_type::function_bind:
                throw function_bind_error{ err.err_mesg };

            case exception_type::func_signature_mismatch:
                throw function_mismatch_error{ err.err_mesg };

            case exception_type::client_send:
                throw client_send_error{ err.err_mesg };

            case exception_type::client_receive:
                throw client_receive_error{ err.err_mesg };

            case exception_type::server_send:
                throw server_send_error{ err.err_mesg };

            case exception_type::server_receive:
                throw server_receive_error{ err.err_mesg };

            case exception_type::rpc_object_mismatch:
                throw object_mismatch_error{ err.err_mesg };

            case exception_type::callback_install:
                throw callback_install_error{ err.err_mesg };

            case exception_type::callback_missing:
                throw callback_missing_error{ err.err_mesg };

            case exception_type::none:
                throw rpc_exception{ err.err_mesg, exception_type::none };

            default:
                RPC_HPP_ASSUME(0);
        }
    }
} //namespace detail

// invariants:
//   1. func_name cannot be empty after construction
struct callback_install_request : detail::rpc_base<true>
{
    callback_install_request() noexcept = default;
    explicit callback_install_request(std::string t_func_name)
        : rpc_base<true>{ std::move(t_func_name) }
    {
        RPC_HPP_POSTCONDITION(!this->func_name.empty());
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

[[nodiscard]] constexpr auto validate_rpc_type(rpc_type type) noexcept -> bool
{
    return static_cast<int>(type) >= static_cast<int>(rpc_type::callback_install_request)
        && static_cast<int>(type) <= static_cast<int>(rpc_type::func_result_w_bind);
}

// invariants:
//   1. m_obj cannot be empty after construction
//   2. m_obj must always hold a valid 'type' field after construction
template<typename Serial>
class rpc_object
{
public:
    using serial_t = typename Serial::serial_t;
    using bytes_t = typename Serial::bytes_t;

    template<bool IsCallback, typename R>
    explicit rpc_object(detail::rpc_result<IsCallback, R> result)
        : m_obj(Serial::serialize_result(std::move(result)))
    {
        RPC_HPP_POSTCONDITION(!is_empty());
        RPC_HPP_POSTCONDITION(
            type() == (IsCallback ? rpc_type::callback_result : rpc_type::func_result));
    }

    template<bool IsCallback, typename... Args>
    explicit rpc_object(detail::rpc_request<IsCallback, Args...> request)
        : m_obj(Serial::serialize_request(std::move(request)))
    {
        RPC_HPP_POSTCONDITION(!is_empty());
        RPC_HPP_POSTCONDITION(
            type() == (IsCallback ? rpc_type::callback_request : rpc_type::func_request));
    }

    template<bool IsCallback>
    explicit rpc_object(detail::rpc_error<IsCallback> error)
        : m_obj(Serial::serialize_error(std::move(error)))
    {
        RPC_HPP_POSTCONDITION(!is_empty());
        RPC_HPP_POSTCONDITION(
            type() == (IsCallback ? rpc_type::callback_error : rpc_type::func_error));
    }

    template<bool IsCallback, typename R, typename... Args>
    explicit rpc_object(detail::rpc_result_w_bind<IsCallback, R, Args...> result)
        : m_obj(Serial::serialize_result_w_bind(std::move(result)))
    {
        RPC_HPP_POSTCONDITION(!is_empty());
        RPC_HPP_POSTCONDITION(type()
            == (IsCallback ? rpc_type::callback_result_w_bind : rpc_type::func_result_w_bind));
    }

    explicit rpc_object(callback_install_request callback_req)
        : m_obj(Serial::serialize_callback_install(std::move(callback_req)))
    {
        RPC_HPP_POSTCONDITION(!is_empty());
        RPC_HPP_POSTCONDITION(type() == rpc_type::callback_install_request);
    }

    RPC_HPP_NODISCARD("parsing consumes the original input")
    static auto parse_bytes(bytes_t&& bytes) noexcept -> std::optional<rpc_object>
    {
        try
        {
            auto result = rpc_object{ Serial::from_bytes(std::move(bytes)) };

            RPC_HPP_POSTCONDITION(!result.is_empty());
            RPC_HPP_POSTCONDITION(validate_rpc_type(result.type()));
            return result;
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }

    RPC_HPP_NODISCARD("converting to bytes may be expensive")
    auto to_bytes() const& -> bytes_t { return Serial::to_bytes(m_obj); }

    RPC_HPP_NODISCARD("converting to bytes consumes object")
    auto to_bytes() && -> bytes_t { return Serial::to_bytes(std::move(m_obj)); }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_func_name() const -> std::string
    {
        RPC_HPP_PRECONDITION(!is_empty());
        return Serial::get_func_name(m_obj);
    }

    template<typename R>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_result() const -> R
    {
        RPC_HPP_PRECONDITION(!is_empty());

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
                throw object_mismatch_error{ "RPC error: invalid rpc_object type detected" };

            default:
                RPC_HPP_ASSUME(0);
        }
    }

    template<typename... Args>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_args() const
    {
        RPC_HPP_PRECONDITION(!is_empty());

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
                throw object_mismatch_error{ "RPC error: invalid rpc_object type detected" };

            default:
                RPC_HPP_ASSUME(0);
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto is_callback_uninstall() const -> bool
    {
        RPC_HPP_PRECONDITION(!is_empty());

        return type() == rpc_type::callback_install_request
            && Serial::get_callback_install(m_obj).is_uninstall;
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_error_type() const -> exception_type
    {
        RPC_HPP_PRECONDITION(!is_empty());

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
                throw object_mismatch_error{ "RPC error: invalid rpc_object type detected" };

            default:
                RPC_HPP_ASSUME(0);
        }
    }

    template<bool IsCallback = false>
    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto get_error_mesg() const -> std::string
    {
        RPC_HPP_PRECONDITION(!is_empty());

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
                throw object_mismatch_error{ "RPC error: invalid rpc_object type detected" };

            default:
                RPC_HPP_ASSUME(0);
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto has_bound_args() const -> bool
    {
        RPC_HPP_PRECONDITION(!is_empty());

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
                throw object_mismatch_error{ "RPC error: invalid rpc_object type detected" };

            default:
                RPC_HPP_ASSUME(0);
        }
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto is_error() const -> bool
    {
        RPC_HPP_PRECONDITION(!is_empty());

        const auto rtype = type();
        return (rtype == rpc_type::func_error) || (rtype == rpc_type::callback_error);
    }

    RPC_HPP_NODISCARD("extracting data from serial object may be expensive")
    auto type() const -> rpc_type
    {
        RPC_HPP_PRECONDITION(!is_empty());

        auto type = Serial::get_type(m_obj);

        RPC_HPP_POSTCONDITION(validate_rpc_type(type));
        return type;
    }

private:
    explicit rpc_object(const serial_t& serial) : m_obj(serial)
    {
        RPC_HPP_POSTCONDITION(!is_empty());
        RPC_HPP_POSTCONDITION(validate_rpc_type(type()));
    }

    explicit rpc_object(serial_t&& serial) noexcept : m_obj(std::move(serial)) {}

    [[nodiscard]] auto is_empty() const& noexcept -> bool { return Serial::is_empty(m_obj); }

    serial_t m_obj;
};

namespace adapters
{
    // invariants: none
    template<typename Adapter>
    struct serial_adapter_base
    {
        using bytes_t = typename Adapter::bytes_t;
        using serial_t = typename Adapter::serial_t;
        using serializer_t = typename Adapter::serializer_t;
        using deserializer_t = typename Adapter::deserializer_t;
        using config = typename Adapter::config;

        static auto is_empty(const serial_t& serial_obj) noexcept -> bool = delete;
        static auto from_bytes(bytes_t&& bytes) -> serial_t = delete;
        static auto to_bytes(const serial_t& serial_obj) -> bytes_t = delete;
        static auto to_bytes(serial_t&& serial_obj) -> bytes_t = delete;
        static auto get_func_name(const serial_t& serial_obj) -> std::string = delete;
        static auto get_type(const serial_t& serial_obj) -> rpc_type = delete;

        template<bool IsCallback, typename R>
        static auto get_result(const serial_t& serial_obj)
            -> detail::rpc_result<IsCallback, R> = delete;

        template<bool IsCallback, typename R>
        static auto serialize_result(const detail::rpc_result<IsCallback, R>& result)
            -> serial_t = delete;

        template<bool IsCallback, typename R, typename... Args>
        static auto serialize_result_w_bind(
            const detail::rpc_result_w_bind<IsCallback, R, Args...>& result) -> serial_t = delete;

        template<bool IsCallback, typename... Args>
        static auto get_request(const serial_t& serial_obj)
            -> detail::rpc_request<IsCallback, Args...> = delete;

        template<bool IsCallback, typename... Args>
        static auto serialize_request(const detail::rpc_request<IsCallback, Args...>& request)
            -> serial_t = delete;

        template<bool IsCallback>
        static auto get_error(const serial_t& serial_obj) -> detail::rpc_error<IsCallback> = delete;

        template<bool IsCallback>
        static auto serialize_error(const detail::rpc_error<IsCallback>& error)
            -> serial_t = delete;

        static auto get_callback_install(const serial_t& serial_obj)
            -> callback_install_request = delete;

        static auto serialize_callback_install(const callback_install_request& callback_req)
            -> serial_t = delete;

        static auto has_bound_args(const serial_t& serial_obj) -> bool = delete;
    };

    // invariants: none
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

    // invariants: none
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
        std::enable_if_t<(detail::is_container_v<T>
                             && (!detail::is_stringlike_v<T>)&&(!detail::is_map_v<T>)),
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
        static_assert(std::is_invocable_v<F, Args...>, "F must be invocable with Args...");
        static_assert(
            std::is_convertible_v<std::invoke_result_t<F, Args...>, R>, "F must yield an R");

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
