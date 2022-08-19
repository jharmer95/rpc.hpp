///@file rpc_adapters/rpc_bitsery.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting Bitsery serialization (https://github.com/fraillt/bitsery)
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
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
#include <numeric>
#include <vector>

template<>
struct std::hash<std::vector<uint8_t>>
{
    [[nodiscard]] size_t operator()(const std::vector<uint8_t>& vec) const noexcept
    {
        static constexpr auto seed_hash = [](size_t seed, size_t val) noexcept
        {
            static constexpr size_t magic_hash_val = 0x9E3779B9UL;
            return seed ^ (val + magic_hash_val + (seed << 6) + (seed >> 2));
        };

        return std::accumulate(vec.begin(), vec.end(), vec.size(), seed_hash);
    }
};

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

    class bitsery_adapter : public serial_adapter_base<bitsery_adapter>
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

        [[nodiscard]] static std::vector<uint8_t> from_bytes(const std::vector<uint8_t>& bytes)
        {
            // TODO: Verify bitsery data somehow
            return bytes;
        }

        [[nodiscard]] static std::vector<uint8_t> from_bytes(std::vector<uint8_t>&& bytes)
        {
            // TODO: Verify bitsery data somehow
            return std::move(bytes);
        }

        [[nodiscard]] static std::vector<uint8_t> to_bytes(const std::vector<uint8_t>& serial_obj)
        {
            return serial_obj;
        }

        [[nodiscard]] static std::string get_func_name(const std::vector<uint8_t>& serial_obj)
        {
            // First 4 bytes represent the type
            size_t index = 4;
            const auto len = extract_length(serial_obj, index);

            assert(index < serial_obj.size());
            assert(index <= std::numeric_limits<ptrdiff_t>::max());

            return { std::next(serial_obj.begin(), index),
                std::next(serial_obj.begin(), index + len) };
        }

        static rpc_type get_type(const std::vector<uint8_t>& serial_obj)
        {
            RPC_HPP_PRECONDITION(serial_obj.size() > sizeof(int));

            // First 4 bytes represent the type
            int n_type;
            std::memcpy(&n_type, serial_obj.data(), sizeof(n_type));
            return static_cast<rpc_type>(n_type);
        }

        template<typename R>
        [[nodiscard]] static detail::func_result<R> get_result(
            const std::vector<uint8_t>& serial_obj)
        {
            return deserialize_rpc_object<detail::func_result<R>>(serial_obj);
        }

        template<typename R>
        [[nodiscard]] static std::vector<uint8_t> serialize_result(
            const detail::func_result<R>& result)
        {
            return serialize_rpc_object<detail::func_result<R>>(result);
        }

        template<typename R, typename... Args>
        [[nodiscard]] static detail::func_result_w_bind<R, Args...> get_result_w_bind(
            const std::vector<uint8_t>& serial_obj)
        {
            return deserialize_rpc_object<detail::func_result_w_bind<R, Args...>>(serial_obj);
        }

        template<typename R, typename... Args>
        [[nodiscard]] static std::vector<uint8_t> serialize_result_w_bind(
            const detail::func_result_w_bind<R, Args...>& result)
        {
            return serialize_rpc_object<detail::func_result_w_bind<R, Args...>>(result);
        }

        template<typename... Args>
        [[nodiscard]] static detail::func_request<Args...> get_request(
            const std::vector<uint8_t>& serial_obj)
        {
            if (const auto type = get_type(serial_obj);
                type == rpc_type::callback_result_w_bind || type == rpc_type::func_result_w_bind)
            {
                // First 4 bytes represent the type
                size_t index = 4;

                // Get length of func_name
                const auto fname_len = extract_length(serial_obj, index);

                assert(index < serial_obj.size());
                assert(index <= std::numeric_limits<ptrdiff_t>::max());

                index += fname_len;
                const auto pre_sz_index = index;

                const uint64_t r_sz =
                    *reinterpret_cast<const uint64_t*>(serial_obj.data() + pre_sz_index);

                if (r_sz == static_cast<uint64_t>(-1))
                {
                    index += 8;
                    const auto r_len = extract_length(serial_obj, index);

                    assert(index < serial_obj.size());
                    assert(index <= std::numeric_limits<ptrdiff_t>::max());
                    index += r_len;
                }
                else
                {
                    index += 8 + r_sz;
                }

                std::vector<uint8_t> copy_obj;
                copy_obj.reserve(serial_obj.size() - 7 - r_sz);

                std::copy_n(serial_obj.begin(), pre_sz_index, std::back_inserter(copy_obj));

                // bind_args = true
                copy_obj.push_back(1);

                std::copy(std::next(serial_obj.begin(), index), serial_obj.end(),
                    std::back_inserter(copy_obj));

                return deserialize_rpc_object<detail::func_request<Args...>>(copy_obj);
            }

            return deserialize_rpc_object<detail::func_request<Args...>>(serial_obj);
        }

        template<typename... Args>
        [[nodiscard]] static std::vector<uint8_t> serialize_request(
            const detail::func_request<Args...>& request)
        {
            return serialize_rpc_object<detail::func_request<Args...>>(request);
        }

        [[nodiscard]] static detail::func_error get_error(const std::vector<uint8_t>& serial_obj)
        {
            return deserialize_rpc_object<detail::func_error>(serial_obj);
        }

        [[nodiscard]] static std::vector<uint8_t> serialize_error(const detail::func_error& error)
        {
            return serialize_rpc_object<detail::func_error>(error);
        }

        [[nodiscard]] static callback_install_request get_callback_install(
            const std::vector<uint8_t>& serial_obj)
        {
            return deserialize_rpc_object<callback_install_request>(serial_obj);
        }

        [[nodiscard]] static std::vector<uint8_t> serialize_callback_install(
            const callback_install_request& callback_req)
        {
            return serialize_rpc_object<callback_install_request>(callback_req);
        }

        [[nodiscard]] static bool has_bound_args(const std::vector<uint8_t>& serial_obj)
        {
            switch (get_type(serial_obj))
            {
                case rpc_type::callback_request:
                case rpc_type::func_request:
                    return deserialize_rpc_object<detail::func_request<>>(serial_obj).bind_args;

                case rpc_type::callback_result_w_bind:
                case rpc_type::func_result_w_bind:
                    return true;

                case rpc_type::callback_error:
                case rpc_type::callback_install_request:
                case rpc_type::callback_result:
                case rpc_type::func_error:
                case rpc_type::func_result:
                default:
                    return false;
            }
        }

    private:
        using bit_buffer = std::vector<uint8_t>;
        using output_adapter = bitsery::OutputBufferAdapter<bit_buffer>;
        using input_adapter = bitsery::InputBufferAdapter<bit_buffer>;

        template<typename T>
        static T deserialize_rpc_object(const std::vector<uint8_t>& buffer)
        {
            T ret_obj{};

            if (const auto [error, _] = bitsery::quickDeserialization(
                    input_adapter{ buffer.begin(), buffer.size() }, ret_obj);
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

                    case bitsery::ReaderError::NoError:
                    default:
                        throw deserialization_error(
                            "Bitsery deserialization failed due to extra data on the end");
                }
            }

            RPC_HPP_POSTCONDITION(!ret_obj.func_name.empty());
            return ret_obj;
        }

        template<typename T>
        static std::vector<uint8_t> serialize_rpc_object(const T& rpc_obj)
        {
            std::vector<uint8_t> buffer;
            buffer.reserve(64);

            const auto bytes_written = bitsery::quickSerialization<output_adapter>(buffer, rpc_obj);

            // Trim excess bytes off the end
            buffer.resize(bytes_written);
            return buffer;
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
    };
} // namespace adapters

namespace detail
{
    template<typename S>
    void serialize(S& s, func_error& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::func_error;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);
        s.value4b(o.except_type);
        s.text1b(o.err_mesg, config::max_string_size);
    }

    template<typename S>
    void serialize(S& s, callback_error& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::callback_error;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);
        s.value4b(o.except_type);
        s.text1b(o.err_mesg, config::max_string_size);
        s.value8b(o.callback_id);
    }

    template<typename S, typename... Args>
    void serialize(S& s, func_request<Args...>& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::func_request;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);
        s.value1b(o.bind_args);

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
                    else if constexpr (detail::is_container_v<T>)
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

    template<typename S, typename... Args>
    void serialize(S& s, callback_request<Args...>& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::callback_result;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);
        s.value1b(o.bind_args);

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
                    else if constexpr (detail::is_container_v<T>)
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

        s.value8b(o.callback_id);
    }

    template<typename S, typename R>
    void serialize(S& s, func_result<R>& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::func_result;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);

        uint64_t r_sz = [&o]
        {
            if constexpr (std::is_void_v<R>)
            {
                return 0;
            }
            else if constexpr (std::is_arithmetic_v<R>)
            {
                return sizeof(R);
            }
            else if constexpr (rpc_hpp::detail::is_container_v<R>)
            {
                return static_cast<uint64_t>(-1);
            }
            else
            {
                std::vector<uint8_t> buf{};
                return bitsery::quickSerialization<
                    bitsery::OutputBufferAdapter<std::vector<uint8_t>>>(buf, o.result);
            }
        }();

        s.value8b(r_sz);

        if constexpr (std::is_arithmetic_v<R>)
        {
            s.template value<sizeof(R)>(o.result);
        }
        else if constexpr (rpc_hpp::detail::is_container_v<R>)
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
    }

    template<typename S, typename R>
    void serialize(S& s, callback_result<R>& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::callback_result;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);

        uint64_t r_sz = [&o]
        {
            if constexpr (std::is_void_v<R>)
            {
                return 0;
            }
            else if constexpr (std::is_arithmetic_v<R>)
            {
                return sizeof(R);
            }
            else if constexpr (rpc_hpp::detail::is_container_v<R>)
            {
                return static_cast<uint64_t>(-1);
            }
            else
            {
                std::vector<uint8_t> buf{};
                return bitsery::quickSerialization<
                    bitsery::OutputBufferAdapter<std::vector<uint8_t>>>(buf, o.result);
            }
        }();

        s.value8b(r_sz);

        if constexpr (std::is_arithmetic_v<R>)
        {
            s.template value<sizeof(R)>(o.result);
        }
        else if constexpr (rpc_hpp::detail::is_container_v<R>)
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

        s.value8b(o.callback_id);
    }

    template<typename S, typename R, typename... Args>
    void serialize(S& s, func_result_w_bind<R, Args...>& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::func_result_w_bind;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);

        uint64_t r_sz = [&o]
        {
            if constexpr (std::is_void_v<R>)
            {
                return 0;
            }
            else if constexpr (std::is_arithmetic_v<R>)
            {
                return sizeof(R);
            }
            else if constexpr (rpc_hpp::detail::is_container_v<R>)
            {
                return static_cast<uint64_t>(-1);
            }
            else
            {
                std::vector<uint8_t> buf{};
                return bitsery::quickSerialization<
                    bitsery::OutputBufferAdapter<std::vector<uint8_t>>>(buf, o.result);
            }
        }();

        s.value8b(r_sz);

        if constexpr (std::is_arithmetic_v<R>)
        {
            s.template value<sizeof(R)>(o.result);
        }
        else if constexpr (rpc_hpp::detail::is_container_v<R>)
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
                    else if constexpr (detail::is_container_v<T>)
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

    template<typename S, typename R, typename... Args>
    void serialize(S& s, callback_result_w_bind<R, Args...>& o)
    {
        using config = adapters::bitsery_adapter::config;

        auto type = rpc_type::callback_result_w_bind;

        s.value4b(type);
        s.text1b(o.func_name, config::max_func_name_size);

        uint64_t r_sz = [&o]
        {
            if constexpr (std::is_void_v<R>)
            {
                return 0;
            }
            else if constexpr (std::is_arithmetic_v<R>)
            {
                return sizeof(R);
            }
            else if constexpr (rpc_hpp::detail::is_container_v<R>)
            {
                return static_cast<uint64_t>(-1);
            }
            else
            {
                std::vector<uint8_t> buf{};
                return bitsery::quickSerialization<
                    bitsery::OutputBufferAdapter<std::vector<uint8_t>>>(buf, o.result);
            }
        }();

        s.value8b(r_sz);

        if constexpr (std::is_arithmetic_v<R>)
        {
            s.template value<sizeof(R)>(o.result);
        }
        else if constexpr (rpc_hpp::detail::is_container_v<R>)
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
                    else if constexpr (detail::is_container_v<T>)
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

        s.value8b(o.callback_id);
    }
} //namespace detail

template<typename S>
void serialize(S& s, callback_install_request& o)
{
    using config = adapters::bitsery_adapter::config;

    auto type = rpc_type::callback_install_request;

    s.value4b(type);
    s.text1b(o.func_name, config::max_func_name_size);
    s.value1b(o.is_uninstall);
    s.value8b(o.callback_id);
}
} // namespace rpc_hpp
