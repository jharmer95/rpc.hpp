#ifndef RPC_HPP_DETAIL_MACROS_HPP
#define RPC_HPP_DETAIL_MACROS_HPP

#include <cassert>

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

#endif
