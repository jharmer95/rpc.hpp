#ifndef RPC_HPP_DETAIL_TRAITS_HPP
#define RPC_HPP_DETAIL_TRAITS_HPP

#include "macros.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace rpc_hpp::detail
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
        -> std::bool_constant<std::is_same_v<decltype(std::declval<typename T::iterator>()->second),
            typename T::mapped_type>>;

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
        -> std::bool_constant<std::is_same_v<decltype(std::declval<T>().at(typename T::key_type{})),
            typename T::mapped_type&>>;

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

template<typename R, typename... Args>
using fptr_t = R (*)(Args...);

template<typename T>
constexpr bool is_ref_arg()
{
    return std::is_reference_v<T>
        && (!std::is_const_v<std::remove_reference_t<T>>)&&(
            !std::is_pointer_v<std::remove_reference_t<T>>);
}

template<typename... Args>
constexpr bool has_ref_args()
{
    return (... || is_ref_arg<Args>());
}
} //namespace rpc_hpp::detail

#endif
