#ifndef RPC_HPP_DETAIL_FOR_EACH_HPP
#define RPC_HPP_DETAIL_FOR_EACH_HPP

#include "macros.hpp"
#include "traits.hpp"

#include <cstddef>
#include <tuple>
#include <utility>

namespace rpc_hpp::detail
{
template<typename F, typename... Ts, size_t... Is>
constexpr void for_each_tuple(
    const std::tuple<Ts...>& tuple, F&& func, RPC_HPP_UNUSED const std::index_sequence<Is...> iseq)
{
    using expander = int[];
    std::ignore = expander{ 0, ((void)std::forward<F>(func)(std::get<Is>(tuple)), 0)... };
}

template<typename F, typename... Ts>
RPC_HPP_INLINE constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, F&& func)
{
    for_each_tuple(tuple, std::forward<F>(func), std::make_index_sequence<sizeof...(Ts)>());
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
} // namespace rpc_hpp::detail

#endif
