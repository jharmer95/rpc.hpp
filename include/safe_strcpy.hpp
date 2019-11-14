#pragma once

#include <cstring>

namespace safe
{
///@brief Provides a safer, length-checked version of 'strcpy' from the C standard library
///
///@param dest Pointer to the destination char string
///@param src Pointer to the source char string
///@param destSize Maximum size of the destination char string
///@return size_t size of the string pointed to by src
///
inline size_t strcpy(char* dest, const char* src, size_t destSize)
{
    const size_t srcSize = strlen(src);

    if (srcSize + 1 < destSize)
    {
        memcpy(dest, src, srcSize + 1);
    }
    else if (destSize != 0)
    {
        memcpy(dest, src, destSize - 1);
        dest[destSize - 1] = '\0';
    }

    return srcSize;
}

///@brief Provides a safer, length-checked version of 'strcpy' from the C standard library.
/// Uses template deduction for fixed-sized arrays.
///
///@tparam size Deduced size of dest
///@param dest Fixed-size (stack) char array
///@param src Pointer to the source char string
///@return size_t size of the string pointed to by src
///
template <size_t size>
inline size_t strcpy(char (&dest)[size], const char* src)
{
    const size_t srcSize = strlen(src);

    if (srcSize + 1 < size)
    {
        memcpy(dest, src, srcSize + 1);
    }
    else if constexpr (size != 0)
    {
        memcpy(dest, src, size - 1);
        dest[size - 1] = '\0';
    }

    return srcSize;
}
} // namespace safe
