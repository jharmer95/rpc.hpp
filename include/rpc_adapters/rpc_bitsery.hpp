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

#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <cassert>
#include <vector>

#if defined(RPC_HPP_ENABLE_SERVER_CACHE)
#    include <numeric>

template<>
struct std::hash<std::vector<uint8_t>>
{
    [[nodiscard]] inline size_t operator()(const std::vector<uint8_t>& vec) const noexcept
    {
        static constexpr auto seed_hash = [](size_t seed, size_t val) noexcept
        {
            static constexpr size_t magic_hash_val = 0x9E3779B9UL;
            return seed ^ (val + magic_hash_val + (seed << 6) + (seed >> 2));
        };

        return std::accumulate(vec.begin(), vec.end(), vec.size(), seed_hash);
    }
};
#endif

namespace rpc_hpp
{
namespace adapters
{
    class bitsery_adapter;

    template<>
    struct serial_traits<bitsery_adapter>
    {
        using serial_t = std::vector<uint8_t>;
        using bytes_t = std::vector<uint8_t>;
    };

    class bitsery_adapter : public detail::serial_adapter_base<bitsery_adapter>
    {
    public:
        struct config
        {
#if defined(RPC_HPP_BITSERY_EXACT_SZ)
            static constexpr bool use_exact_size = true;
#else
            static constexpr bool use_exact_size = false;
#endif

            static const uint64_t max_func_name_size;
            static const uint64_t max_string_size;
            static const uint64_t max_container_size;
        };

        [[nodiscard]] static std::vector<uint8_t> to_bytes(std::vector<uint8_t>&& serial_obj)
        {
            return serial_obj;
        }

        [[nodiscard]] static std::optional<std::vector<uint8_t>> from_bytes(
            std::vector<uint8_t>&& bytes)
        {
            // TODO: Verify bitsery data somehow
            return bytes;
        }

        static std::vector<uint8_t> empty_object()
        {
            std::vector<uint8_t> buffer(sizeof(int) + 2);
            bitsery::quickSerialization<output_adapter>(buffer, pack_helper<void>{});
            return buffer;
        }

        template<typename R, typename... Args>
        [[nodiscard]] static std::vector<uint8_t> serialize_pack(
            const detail::packed_func<R, Args...>& pack)
        {
            const auto helper = to_helper(pack);
            std::vector<uint8_t> buffer{};
            buffer.reserve(64);

            const auto bytes_written = bitsery::quickSerialization<output_adapter>(buffer, helper);
            buffer.resize(bytes_written);
            return buffer;
        }

        template<typename R, typename... Args>
        [[nodiscard]] static detail::packed_func<R, Args...> deserialize_pack(
            const std::vector<uint8_t>& serial_obj)
        {
            pack_helper<R, Args...> helper{};

            if (const auto [error, _] = bitsery::quickDeserialization(
                    input_adapter{ serial_obj.begin(), serial_obj.size() }, helper);
                error != bitsery::ReaderError::NoError)
            {
                switch (error)
                {
                    case bitsery::ReaderError::ReadingError:
                        throw deserialization_error(
                            "Bitsery deserialization failed due to a reading error");

                    case bitsery::ReaderError::DataOverflow:
                        throw function_mismatch("Bitsery deserialization failed due to data "
                                                "overflow (likely mismatched "
                                                "function signature)");

                    case bitsery::ReaderError::InvalidData:
                        throw deserialization_error(
                            "Bitsery deserialization failed due to a invalid data");

                    case bitsery::ReaderError::InvalidPointer:
                        throw deserialization_error(
                            "Bitsery deserialization failed due to an invalid pointer");

                    default:
                        throw deserialization_error(
                            "Bitsery deserialization failed due to extra data on the end");
                }
            }

            return from_helper(helper);
        }

        [[nodiscard]] static std::string get_func_name(const std::vector<uint8_t>& serial_obj)
        {
            size_t index = sizeof(int);
            const auto len = extract_length(serial_obj, index);

            assert(index < serial_obj.size());
            assert(index <= std::numeric_limits<ptrdiff_t>::max());

            return { serial_obj.begin() + static_cast<ptrdiff_t>(index),
                serial_obj.begin() + static_cast<ptrdiff_t>(index + len) };
        }

        [[nodiscard]] static rpc_exception extract_exception(const std::vector<uint8_t>& serial_obj)
        {
            const auto pack = deserialize_pack<void>(serial_obj);
            return rpc_exception{ pack.get_err_mesg(), pack.get_except_type() };
        }

        static void set_exception(std::vector<uint8_t>& serial_obj, const rpc_exception& ex)
        {
            // copy except_type into buffer
            const int ex_type = static_cast<int>(ex.get_type());
            memcpy(&serial_obj[0], &ex_type, sizeof(int));
            const std::string_view mesg = ex.what();
            const auto new_err_len = static_cast<unsigned>(mesg.size());

            size_t index = sizeof(int);
            const auto name_len = extract_length(serial_obj, index);
            const size_t name_sz_len = index;

            index += name_len;

            if (const auto err_len = extract_length(serial_obj, index); new_err_len != err_len)
            {
                if (err_len != 0)
                {
                    assert(index < serial_obj.size());
                    assert(index <= std::numeric_limits<ptrdiff_t>::max());

                    serial_obj.erase(serial_obj.begin() + static_cast<ptrdiff_t>(name_sz_len)
                            + static_cast<ptrdiff_t>(name_len),
                        serial_obj.begin() + static_cast<ptrdiff_t>(index + err_len));
                }

                index = name_sz_len + name_len;
                write_length(serial_obj, new_err_len, index);
            }

            for (unsigned i = 0; i < new_err_len; ++i)
            {
                assert(index < serial_obj.size());
                assert(index <= std::numeric_limits<ptrdiff_t>::max());

                serial_obj.insert(serial_obj.begin() + static_cast<ptrdiff_t>(index++),
                    static_cast<unsigned char>(mesg[i]));
            }
        }

    private:
        using bit_buffer = std::vector<uint8_t>;
        using output_adapter = bitsery::OutputBufferAdapter<bit_buffer>;
        using input_adapter = bitsery::InputBufferAdapter<bit_buffer>;

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

            int except_type{};
            std::string func_name{};
            std::string err_mesg{};
            R result{};
            args_t args{};

            template<typename S>
            void serialize(S& s)
            {
                s.template value<sizeof(int)>(except_type);
                s.text1b(func_name, config::max_func_name_size);
                s.text1b(err_mesg, config::max_string_size);

                if constexpr (std::is_arithmetic_v<R>)
                {
                    s.template value<sizeof(R)>(result);
                }
                else if constexpr (rpc_hpp::detail::is_container_v<R>)
                {
                    if constexpr (std::is_arithmetic_v<typename R::value_type>)
                    {
                        s.template container<sizeof(typename R::value_type)>(
                            result, config::max_container_size);
                    }
                    else
                    {
                        s.container(result, config::max_container_size);
                    }
                }
                else
                {
                    s.object(result);
                }

                s.ext(args,
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
                            else if constexpr (rpc_hpp::detail::is_container_v<T>)
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

            int except_type{};
            std::string func_name{};
            std::string err_mesg{};
            args_t args{};

            template<typename S>
            void serialize(S& s)
            {
                s.template value<sizeof(int)>(except_type);
                s.text1b(func_name, config::max_func_name_size);
                s.text1b(err_mesg, config::max_string_size);

                s.ext(args,
                    bitsery::ext::StdTuple{ [](S& s2, std::string& str)
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
                            else if constexpr (rpc_hpp::detail::is_container_v<T>)
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
        };

        // nodiscard because a potentially expensive copy and allocation is being done
        template<typename R, typename... Args>
        [[nodiscard]] static pack_helper<R, Args...> to_helper(
            const detail::packed_func<R, Args...>& pack)
        {
            pack_helper<R, Args...> helper{};
            helper.func_name = pack.get_func_name();
            helper.args = pack.get_args();

            if (!pack)
            {
                helper.except_type = static_cast<int>(pack.get_except_type());
                helper.err_mesg = pack.get_err_mesg();
                return helper;
            }

            if constexpr (!std::is_void_v<R>)
            {
                helper.result = pack.get_result();
            }

            return helper;
        }

        // nodiscard because a potentially expensive copy and allocation is being done
        template<typename R, typename... Args>
        [[nodiscard]] static detail::packed_func<R, Args...> from_helper(
            pack_helper<R, Args...> helper)
        {
            if constexpr (std::is_void_v<R>)
            {
                if (helper.except_type == 0)
                {
                    return detail::packed_func<void, Args...>{ std::move(helper.func_name),
                        std::move(helper.args) };
                }

                detail::packed_func<void, Args...> pack{ std::move(helper.func_name),
                    std::move(helper.args) };

                pack.set_exception(
                    std::move(helper.err_mesg), static_cast<exception_type>(helper.except_type));

                return pack;
            }
            else
            {
                if (helper.err_mesg.empty())
                {
                    return detail::packed_func<R, Args...>{ std::move(helper.func_name),
                        std::move(helper.result), std::move(helper.args) };
                }

                detail::packed_func<R, Args...> pack{ std::move(helper.func_name), std::nullopt,
                    std::move(helper.args) };

                pack.set_exception(
                    std::move(helper.err_mesg), static_cast<exception_type>(helper.except_type));

                return pack;
            }
        }

        // Borrowed from Bitsery library for compatibility
        static unsigned extract_length(const bit_buffer& bytes, size_t& index) noexcept
        {
            RPC_HPP_PRECONDITION(index < bytes.size());

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
        static void write_length(bit_buffer& bytes, size_t size, size_t& index)
        {
            RPC_HPP_PRECONDITION(size < 0x40000000U);

            if (size < 0x80U)
            {
                assert(index < size);
                assert(index <= std::numeric_limits<ptrdiff_t>::max());

                bytes.insert(
                    bytes.begin() + static_cast<ptrdiff_t>(index++), static_cast<uint8_t>(size));

                return;
            }

            if (size < 0x4000U)
            {
                bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                    static_cast<uint8_t>((size >> 8) | 0x80U));

                bytes.insert(
                    bytes.begin() + static_cast<ptrdiff_t>(index++), static_cast<uint8_t>(size));

                return;
            }

            bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                static_cast<uint8_t>((size >> 24) | 0xC0U));

            bytes.insert(
                bytes.begin() + static_cast<ptrdiff_t>(index++), static_cast<uint8_t>(size >> 16));

            bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                *reinterpret_cast<const uint8_t*>(&size));

            bytes.insert(bytes.begin() + static_cast<ptrdiff_t>(index++),
                *reinterpret_cast<const uint8_t*>(&size) + 1);
        }
    };
} // namespace adapters
} // namespace rpc_hpp
