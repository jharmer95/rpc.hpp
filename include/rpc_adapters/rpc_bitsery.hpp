///@file rpc_adapters/rpc_bitsery.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting Bitsery serialization (https://github.com/fraillt/bitsery)
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

#include "../rpc.hpp"

#include <limits>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <vector>

#if defined(RPC_HPP_ENABLE_SERVER_CACHE)
template<>
struct std::hash<std::vector<uint8_t>>
{
    [[nodiscard]] size_t operator()(const std::vector<uint8_t>& vec) const noexcept
    {
        size_t seed = vec.size();

        for (const auto i : vec)
        {
            seed ^= static_cast<size_t>(i) + size_t{ 0x9E3779B9UL } + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
#endif

namespace rpc
{
namespace adapters
{
    using bitsery_adapter =
        details::serial_adapter<std::vector<uint8_t>, std::vector<std::uint8_t>>;

    namespace bitsery
    {
        using bit_buffer = std::vector<uint8_t>;

        namespace config
        {
#if defined(RPC_HPP_BITSERY_EXACT_SZ)
            static constexpr bool use_exact_size = true;
#else
            static constexpr bool use_exact_size = false;
#endif

            extern const uint64_t max_func_name_size;
            extern const uint64_t max_string_size;
            extern const uint64_t max_container_size;
        } // namespace config

        namespace details
        {
            using output_adapter = ::bitsery::OutputBufferAdapter<bit_buffer>;
            using input_adapter = ::bitsery::InputBufferAdapter<bit_buffer>;

            // NOTE: This jank can be replaced once C++20 is adopted
            template<typename T,
                int = std::is_arithmetic_v<T> + std::is_integral_v<T> * 2 + std::is_signed_v<T> * 3>
            struct largest;

            template<typename T>
            struct largest<T, 0>
            {
                using type = T;
            };

            template<typename T>
            struct largest<T, 3>
            {
                using type = uint64_t;
            };

            template<typename T>
            struct largest<T, 4>
            {
                static_assert(!std::is_same_v<T, long double>,
                    "long double is not supported for RPC bitsery serialization!");
                using type = double;
            };

            template<typename T>
            struct largest<T, 6>
            {
                using type = int64_t;
            };

            template<typename T>
            using largest_t = typename largest<T>::type;

            template<typename R, typename... Args>
            struct pack_helper
            {
                static_assert(!std::is_same_v<R, long double>,
                    "long double is not supported for RPC bitsery serialization!");

#if defined(RPC_HPP_BITSERY_EXACT_SZ)
                using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
#else
                using args_t =
                    std::tuple<largest_t<std::remove_cv_t<std::remove_reference_t<Args>>>...>;
#endif

                pack_helper() = default;

                pack_helper(std::string name, std::string err, R res, args_t arg_tup) noexcept(
                    std::is_move_constructible_v<R>)
                    : func_name(std::move(name)), err_mesg(std::move(err)), result(std::move(res)),
                      args(std::move(arg_tup))
                {
                }

                std::string func_name{};
                std::string err_mesg{};
                R result{};
                args_t args{};
            };

            template<typename... Args>
            struct pack_helper<void, Args...>
            {
#if defined(RPC_HPP_BITSERY_EXACT_SZ)
                using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
#else
                using args_t =
                    std::tuple<largest_t<std::remove_cv_t<std::remove_reference_t<Args>>>...>;
#endif

                pack_helper() = default;

                pack_helper(std::string name, std::string err, args_t arg_tup) noexcept
                    : func_name(std::move(name)), err_mesg(std::move(err)), args(std::move(arg_tup))
                {
                }

                std::string func_name{};
                std::string err_mesg{};
                args_t args{};
            };

            template<typename S, typename R, typename... Args>
            void serialize(S& s, pack_helper<R, Args...>& o)
            {
                s.text1b(o.func_name, config::max_func_name_size);
                s.text1b(o.err_mesg, config::max_string_size);

                if constexpr (std::is_arithmetic_v<R>)
                {
                    s.template value<sizeof(R)>(o.result);
                }
                else if constexpr (rpc::details::is_container_v<R>)
                {
                    if constexpr (std::is_arithmetic_v<typename R::value_type>)
                    {
                        s.template container<sizeof(typename R::value_type)>(
                            o.result, config::max_container_size);
                    }
                    else
                    {
                        s.container(o.result, config::max_container_size);
                    }
                }
                else if constexpr (!std::is_void_v<R>)
                {
                    s.object(o.result);
                }

                s.ext(o.args,
                    ::bitsery::ext::StdTuple{ [](S& s2, std::string& str)
                        { s2.text1b(str, config::max_string_size); },
                        // Fallback serializer for integers, floats, and enums
                        [](auto& s2, auto& val)
                        {
                            using T = std::remove_cv_t<std::remove_reference_t<decltype(val)>>;

                            if constexpr (std::is_arithmetic_v<T>)
                            {
                                if constexpr (config::use_exact_size)
                                {
                                    s2.template value<sizeof(val)>(val);
                                }
                                else
                                {
                                    s2.value8b(val);
                                }
                            }
                            else if constexpr (rpc::details::is_container_v<T>)
                            {
                                if constexpr (std::is_arithmetic_v<typename T::value_type>)
                                {
                                    s2.template container<sizeof(typename T::value_type)>(
                                        val, config::max_container_size);
                                }
                                else
                                {
                                    s2.container(val, config::max_container_size);
                                }
                            }
                            else
                            {
                                s2.object(val);
                            }
                        } });
            }

            // nodiscard because a potentially expensive copy and allocation is being done
            template<typename R, typename... Args>
            [[nodiscard]] pack_helper<R, Args...> to_helper(const packed_func<R, Args...>& pack)
            {
                pack_helper<R, Args...> helper;

                if (!pack)
                {
                    helper.err_mesg = pack.get_err_mesg();
                }
                else
                {
                    if constexpr (!std::is_void_v<R>)
                    {
                        helper.result = pack.get_result();
                    }
                }

                helper.func_name = pack.get_func_name();
                helper.args = pack.get_args();

                return helper;
            }

#if !defined(RPC_HPP_BITSERY_EXACT_SZ)
#    if defined(_MSC_VER)
#        pragma warning(push)
#        pragma warning(disable : 4244)
#    elif defined(__GNUC__)
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wconversion"
#    elif defined(__clang__)
#        pragma clang diagnostic push
#        pragma clang diagnostic ignored "-Wconversion"
#        pragma clang diagnostic ignored "-Wshorten-64-to-32"
#    endif
#endif

            // nodiscard because a potentially expensive copy and allocation is being done
            template<typename R, typename... Args>
            [[nodiscard]] packed_func<R, Args...> from_helper(pack_helper<R, Args...> helper)
            {
                if constexpr (std::is_void_v<R>)
                {
                    if (helper.err_mesg.empty())
                    {
                        return packed_func<void, Args...>{ std::move(helper.func_name),
                            std::move(helper.args) };
                    }

                    packed_func<void, Args...> pack{ std::move(helper.func_name),
                        std::move(helper.args) };

                    pack.set_err_mesg(helper.err_mesg);
                    return pack;
                }
                else
                {
                    if (helper.err_mesg.empty())
                    {
                        return packed_func<R, Args...>{ std::move(helper.func_name),
                            std::move(helper.result), std::move(helper.args) };
                    }

                    packed_func<R, Args...> pack{ std::move(helper.func_name),
                        std::move(helper.result), std::move(helper.args) };

                    pack.set_err_mesg(helper.err_mesg);
                    pack.clear_result();
                    return pack;
                }
            }

#if !defined(RPC_HPP_BITSERY_EXACT_SZ)
#    if defined(_MSC_VER)
#        pragma warning(pop)
#    elif defined(__GNUC__)
#        pragma GCC diagnostic pop
#    elif defined(__clang__)
#        pragma clang diagnostic pop
#    endif
#endif

            // Borrowed from Bitsery library for compatibility
            inline unsigned extract_length(const bit_buffer& bytes, size_t& index)
            {
                assert(index < bytes.size());
                const uint8_t hb = bytes[index++];

                if (hb < 0x80U)
                {
                    return hb;
                }

                assert(index < bytes.size());
                const uint8_t lb = bytes[index++];

                if ((hb & 0x40U) != 0U)
                {
                    assert(index < bytes.size());
                    const uint16_t lw = *reinterpret_cast<const uint16_t*>(&bytes[index++]);

                    return ((((hb & 0x3FU) << 8) | lb) << 16) | lw;
                }

                return ((hb & 0x7FU) << 8) | lb;
            }

            // Borrowed from Bitsery library for compatibility
            inline void write_length(bit_buffer& bytes, size_t size, size_t& index)
            {
                if (size < 0x80U)
                {
                    assert(index < size);
                    assert(index <= std::numeric_limits<ptrdiff_t>::max());

                    bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                        static_cast<uint8_t>(size));
                }
                else
                {
                    if (size < 0x4000U)
                    {
                        bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                            static_cast<uint8_t>((size >> 8) | 0x80U));

                        bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                            static_cast<uint8_t>(size));
                    }
                    else
                    {
                        assert(size < 0x40000000U);

                        bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                            static_cast<uint8_t>((size >> 24) | 0xC0U));

                        bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                            static_cast<uint8_t>(size >> 16));

                        bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                            *reinterpret_cast<const uint8_t*>(&size));

                        bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                            *reinterpret_cast<const uint8_t*>(&size) + 1);
                    }
                }
            }
        } // namespace details
    }     // namespace bitsery
} // namespace adapters

template<>
[[nodiscard]] inline adapters::bitsery::bit_buffer adapters::bitsery_adapter::to_bytes(
    const adapters::bitsery::bit_buffer& serial_obj)
{
    return serial_obj;
}

template<>
[[nodiscard]] inline adapters::bitsery::bit_buffer adapters::bitsery_adapter::to_bytes(
    adapters::bitsery::bit_buffer&& serial_obj)
{
    return std::move(serial_obj);
}

template<>
[[nodiscard]] inline adapters::bitsery::bit_buffer adapters::bitsery_adapter::from_bytes(
    const adapters::bitsery::bit_buffer& bytes)
{
    return bytes;
}

template<>
[[nodiscard]] inline adapters::bitsery::bit_buffer adapters::bitsery_adapter::from_bytes(
    adapters::bitsery::bit_buffer&& bytes)
{
    return std::move(bytes);
}

template<>
template<typename R, typename... Args>
[[nodiscard]] adapters::bitsery::bit_buffer pack_adapter<adapters::bitsery_adapter>::serialize_pack(
    const packed_func<R, Args...>& pack)
{
    using namespace adapters::bitsery;

    const auto helper = adapters::bitsery::details::to_helper(pack);

    bit_buffer buffer;
    const auto bytes_written =
        bitsery::quickSerialization<adapters::bitsery::details::output_adapter>(buffer, helper);

    buffer.resize(bytes_written);
    return buffer;
}

template<>
template<typename R, typename... Args>
[[nodiscard]] packed_func<R, Args...> pack_adapter<adapters::bitsery_adapter>::deserialize_pack(
    const adapters::bitsery::bit_buffer& serial_obj)
{
    using namespace adapters::bitsery;

    adapters::bitsery::details::pack_helper<R, Args...> helper;

    const auto [error, success] = bitsery::quickDeserialization(
        adapters::bitsery::details::input_adapter{ serial_obj.begin(), serial_obj.size() }, helper);

    if (error != bitsery::ReaderError::NoError)
    {
        const std::string error_mesg = [error = error]()
        {
            switch (error)
            {
                case bitsery::ReaderError::ReadingError:
                    return "a reading error!";

                case bitsery::ReaderError::DataOverflow:
                    return "data overflow!";

                case bitsery::ReaderError::InvalidData:
                    return "invalid data!";

                case bitsery::ReaderError::InvalidPointer:
                    return "an invalid pointer!";

                case bitsery::ReaderError::NoError:
                default:
                    return "extra data on the end!";
            }
        }();

        throw std::runtime_error(
            std::string{ "Bitsery deserialization failed due to " } + error_mesg);
    }

    return adapters::bitsery::details::from_helper(helper);
}

template<>
[[nodiscard]] inline std::string pack_adapter<adapters::bitsery_adapter>::get_func_name(
    const adapters::bitsery::bit_buffer& serial_obj)
{
    size_t index = 0;
    const auto len = adapters::bitsery::details::extract_length(serial_obj, index);

    assert(index < serial_obj.size());
    assert(index <= std::numeric_limits<ptrdiff_t>::max());
    assert(len <= std::numeric_limits<ptrdiff_t>::max());
    return { serial_obj.begin() + static_cast<ptrdiff_t>(index),
        serial_obj.begin() + static_cast<ptrdiff_t>(index + len) };
}

template<>
inline void pack_adapter<adapters::bitsery_adapter>::set_err_mesg(
    adapters::bitsery::bit_buffer& serial_obj, const std::string& mesg)
{
    size_t index = 0;

    const auto name_len = adapters::bitsery::details::extract_length(serial_obj, index);

    const size_t name_sz_len = index;

    index += name_len;
    const auto err_len = adapters::bitsery::details::extract_length(serial_obj, index);

    const auto new_err_len = static_cast<unsigned>(mesg.size());

    if (new_err_len != err_len)
    {
        assert(index < serial_obj.size());
        assert(index <= std::numeric_limits<ptrdiff_t>::max());

        serial_obj.erase(serial_obj.begin() + static_cast<ptrdiff_t>(name_sz_len)
                + static_cast<ptrdiff_t>(name_len),
            serial_obj.begin() + static_cast<ptrdiff_t>(index + err_len));

        index = name_sz_len + name_len;
        adapters::bitsery::details::write_length(serial_obj, new_err_len, index);
    }

    for (unsigned i = 0; i < new_err_len; ++i)
    {
        assert(index < serial_obj.size());
        assert(index <= std::numeric_limits<ptrdiff_t>::max());

        serial_obj.insert(serial_obj.begin() + static_cast<ptrdiff_t>(index++),
            static_cast<unsigned char>(mesg[i]));
    }
}
} // namespace rpc
