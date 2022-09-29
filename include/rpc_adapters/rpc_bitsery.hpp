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

#ifndef RPC_ADAPTERS_BITSERY_HPP
#define RPC_ADAPTERS_BITSERY_HPP

#include "../rpc.hpp"

#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/deserializer.h>
#include <bitsery/ext/std_set.h>
#include <bitsery/ext/std_map.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/serializer.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/core/traits.h>
#include <bitsery/traits/deque.h>
#include <bitsery/traits/forward_list.h>
#include <bitsery/traits/list.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <cassert>
#include <numeric>
#include <type_traits>

template<>
struct std::hash<std::vector<uint8_t>>
{
    [[nodiscard]] size_t operator()(const std::vector<uint8_t>& vec) const noexcept
    {
        return std::accumulate(vec.begin(), vec.end(), vec.size(),
            [](size_t seed, size_t val) noexcept
            {
                static constexpr size_t magic_hash_val = 0x9E37'79B9UL;
                return seed ^ (val + magic_hash_val + (seed << 6UL) + (seed >> 2UL));
            });
    }
};

namespace rpc_hpp::adapters
{
class bitsery_adapter;
class bitsery_adapter2;

template<>
struct serial_traits<bitsery_adapter>
{
    using serial_t = std::vector<uint8_t>;
    using bytes_t = std::vector<uint8_t>;
};

template<>
struct config<bitsery_adapter>
{
#if defined(RPC_HPP_BITSERY_EXACT_SZ)
    static constexpr bool use_exact_size = true;
#else
    static constexpr bool use_exact_size = false;
#endif

    static const size_t max_func_name_size;
    static const size_t max_string_size;
    static const size_t max_container_size;
};

// TODO: Move this function
template<typename S, typename T, typename Adapter, bool Deserialize>
void parse_obj(S& ser, serializer<Adapter, Deserialize>& fallback, T& val)
{
    using config = config<bitsery_adapter>;

    if constexpr (std::is_arithmetic_v<T>)
    {
        if constexpr (config::use_exact_size)
        {
            ser.template value<sizeof(T)>(val);
        }
        else
        {
            ser.value8b(val);
        }
    }
    else if constexpr (detail::is_stringlike_v<T>)
    {
        ser.text1b(val, config::max_string_size);
    }
    else if constexpr (detail::is_map_v<T>)
    {
        using key_t = typename T::key_type;
        using val_t = typename T::mapped_type;

        ser.ext(val, ::bitsery::ext::StdMap{ config::max_container_size },
            [&fallback](S& s_ser, key_t& map_key, val_t& map_val)
            {
                parse_obj(s_ser, fallback, map_key);
                parse_obj(s_ser, fallback, map_val);
            });
    }
    else if constexpr (detail::is_set_v<T>)
    {
        using key_t = typename T::key_type;

        ser.ext(val, ::bitsery::ext::StdSet{ config::max_container_size },
            [&fallback](S& s_ser, key_t& key_val) { parse_obj(s_ser, fallback, key_val); });
    }
    else if constexpr (detail::is_container_v<T>)
    {
        using val_t = typename T::value_type;

        if constexpr (std::is_arithmetic_v<val_t>)
        {
            if constexpr (::bitsery::traits::ContainerTraits<std::remove_cv_t<T>>::isResizable)
            {
                ser.template container<sizeof(val_t), std::remove_cv_t<T>>(
                    val, config::max_container_size);
            }
            else
            {
                ser.template container<sizeof(val_t), std::remove_cv_t<T>>(val);
            }
        }
        else
        {
            if constexpr (::bitsery::traits::ContainerTraits<std::remove_cv_t<T>>::isResizable)
            {
                ser.container(val, config::max_container_size,
                    [](S& s_ser, std::string& substr)
                    { s_ser.text1b(substr, config::max_string_size); });
            }
            else
            {
                ser.container(val,
                    [](S& s_ser, std::string& substr)
                    { s_ser.text1b(substr, config::max_string_size); });
            }
        }
    }
    else
    {
        serialize(fallback, val);
    }
}

class bitsery_serializer : public serializer<bitsery_serializer, false>
{
public:
    bitsery_serializer() : m_ser(m_bytes) { m_bytes.reserve(64UL); }

    [[nodiscard]] const std::vector<uint8_t>& object() &
    {
        flush();
        return m_bytes;
    }

    [[nodiscard]] std::vector<uint8_t>&& object() &&
    {
        flush();
        return std::move(m_bytes);
    }

    template<typename T>
    void as_bool(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        m_ser.value1b(val);
    }

    template<typename T>
    void as_float(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        m_ser.value<sizeof(T)>(val);
    }

    template<typename T>
    void as_int(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        m_ser.value<sizeof(T)>(val);
    }

    template<typename T>
    void as_string(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        m_ser.text1b(val, config::max_string_size);
    }

    template<typename T>
    void as_array(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        using val_t = typename T::value_type;

        if constexpr (std::is_arithmetic_v<val_t>)
        {
            m_ser.container<sizeof(val_t)>(val, config::max_container_size);
        }
        else
        {
            m_ser.container(val, config::max_container_size);
        }
    }

    template<typename T, size_t N>
    void as_array(RPC_HPP_UNUSED const std::string_view key, const std::array<T, N>& val)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            m_ser.container<sizeof(T)>(val);
        }
        else
        {
            m_ser.container(val);
        }
    }

    template<typename T>
    void as_map(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        using S = ::bitsery::Serializer<output_adapter>;
        using key_t = typename T::key_type;
        using val_t = typename T::mapped_type;

        m_ser.ext(val, ::bitsery::ext::StdMap{ config::max_container_size },
            [this](S& ser, key_t& map_key, val_t& map_val)
            {
                parse_obj(ser, *this, map_key);
                parse_obj(ser, *this, map_val);
            });
    }

    template<typename T>
    void as_multimap(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        as_map(key, val);
    }

    template<typename... Args>
    void as_tuple(RPC_HPP_UNUSED const std::string_view key, const std::tuple<Args...>& val)
    {
        using S = ::bitsery::Serializer<output_adapter>;

        m_ser.ext(val,
            ::bitsery::ext::StdTuple{ [this](S& ser, auto& subval)
                {
                    parse_obj(ser, *this, subval);
                } });
    }

    template<typename T>
    void as_object(RPC_HPP_UNUSED const std::string_view key, const T& val)
    {
        parse_obj(m_ser, *this, val);
    }

private:
    using config = config<bitsery_adapter>;
    using output_adapter = ::bitsery::OutputBufferAdapter<std::vector<uint8_t>>;

    void flush()
    {
        m_ser.adapter().flush();
        m_bytes.resize(m_ser.adapter().writtenBytesCount());
    }

    std::vector<uint8_t> m_bytes{};
    ::bitsery::Serializer<output_adapter> m_ser;
};

class bitsery_deserializer : public serializer<bitsery_deserializer, true>
{
public:
    explicit bitsery_deserializer(const std::vector<uint8_t>& bytes)
        : m_bytes(bytes), m_ser(m_bytes.cbegin(), m_bytes.size())
    {
    }

    template<typename T>
    void as_bool(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        m_ser.value1b(val);
    }

    template<typename T>
    void as_float(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        m_ser.value<sizeof(T)>(val);
    }

    template<typename T>
    void as_int(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        m_ser.value<sizeof(T)>(val);
    }

    template<typename T>
    void as_string(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        m_ser.text1b(val, config::max_string_size);
    }

    template<typename T>
    void as_array(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        if constexpr (std::is_arithmetic_v<typename T::value_type>)
        {
            m_ser.container<sizeof(typename T::value_type)>(val, config::max_container_size);
        }
        else
        {
            m_ser.container(val, config::max_container_size);
        }
    }

    template<typename T, size_t N>
    void as_array(RPC_HPP_UNUSED const std::string_view key, std::array<T, N>& val)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            m_ser.container<sizeof(T)>(val);
        }
        else
        {
            m_ser.container(val);
        }
    }

    template<typename T>
    void as_map(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        using S = ::bitsery::Deserializer<input_adapter>;
        using key_t = typename T::key_type;
        using val_t = typename T::mapped_type;

        m_ser.ext(val, ::bitsery::ext::StdMap{ config::max_container_size },
            [this](S& ser, key_t& map_key, val_t& map_val)
            {
                parse_obj(ser, *this, map_key);
                parse_obj(ser, *this, map_val);
            });
    }

    template<typename T>
    void as_multimap(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        as_map(key, val);
    }

    template<typename... Args>
    void as_tuple(RPC_HPP_UNUSED const std::string_view key, std::tuple<Args...>& val)
    {
        using S = ::bitsery::Deserializer<input_adapter>;

        m_ser.ext(val,
            ::bitsery::ext::StdTuple{ [this](S& ser, auto& subval)
                {
                    parse_obj(ser, *this, subval);
                } });
    }

    template<typename T>
    void as_object(RPC_HPP_UNUSED const std::string_view key, T& val)
    {
        parse_obj(m_ser, *this, val);
    }

private:
    using config = config<bitsery_adapter>;
    using input_adapter = ::bitsery::InputBufferAdapter<std::vector<uint8_t>>;

    const std::vector<uint8_t>& m_bytes;
    ::bitsery::Deserializer<input_adapter> m_ser;
};

class bitsery_adapter : public serial_adapter_base<bitsery_adapter>
{
public:
    using config = config<bitsery_adapter>;

    [[nodiscard]] static std::vector<uint8_t> from_bytes(const std::vector<uint8_t>& bytes)
    {
        RPC_HPP_PRECONDITION(bytes.size() >= sizeof(int));

        // Check that getting the type does not throw
        RPC_HPP_UNUSED const auto type = get_type(bytes);

        if (get_func_name(bytes).empty())
        {
            throw deserialization_error("Bitsery: func_name could not be extracted from bytes");
        }

        return bytes;
    }

    [[nodiscard]] static std::vector<uint8_t> from_bytes(std::vector<uint8_t>&& bytes)
    {
        RPC_HPP_PRECONDITION(bytes.size() >= sizeof(int));

        // Check that getting the type does not throw
        RPC_HPP_UNUSED const auto type = get_type(bytes);

        if (get_func_name(bytes).empty())
        {
            throw deserialization_error("Bitsery: func_name could not be extracted from bytes");
        }

        return bytes;
    }

    [[nodiscard]] static std::vector<uint8_t> to_bytes(const std::vector<uint8_t>& serial_obj)
    {
        return serial_obj;
    }

    [[nodiscard]] static std::string get_func_name(const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(serial_obj.size() > sizeof(int));

        // First 4 bytes represent the type
        size_t index = 4;
        const auto len = extract_length(serial_obj, index);

        assert(index + len <= std::numeric_limits<ptrdiff_t>::max());
        assert(index + len < serial_obj.size());

        return { std::next(serial_obj.begin(), static_cast<ptrdiff_t>(index)),
            std::next(
                serial_obj.begin(), static_cast<ptrdiff_t>(index) + static_cast<ptrdiff_t>(len)) };
    }

    static rpc_type get_type(const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(serial_obj.size() >= sizeof(int));

        // First 4 bytes represent the type
        int n_type{};
        std::memcpy(&n_type, serial_obj.data(), sizeof(n_type));

        if ((n_type < static_cast<int>(rpc_type::callback_install_request))
            || (n_type > static_cast<int>(rpc_type::func_result_w_bind)))
        {
            throw deserialization_error("Bitsery: invalid type field detected");
        }

        return static_cast<rpc_type>(n_type);
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static detail::rpc_result<IsCallback, R> get_result(
        const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(verify_type(
            serial_obj, IsCallback ? rpc_type::callback_result : rpc_type::func_result));

        return deserialize_rpc_object<detail::rpc_result<IsCallback, R>>(serial_obj);
    }

    template<bool IsCallback, typename R>
    [[nodiscard]] static std::vector<uint8_t> serialize_result(
        const detail::rpc_result<IsCallback, R>& result)
    {
        return serialize_rpc_object<detail::rpc_result<IsCallback, R>>(result);
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static detail::rpc_result_w_bind<IsCallback, R, Args...> get_result_w_bind(
        const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(verify_type(serial_obj,
            IsCallback ? rpc_type::callback_result_w_bind : rpc_type::func_result_w_bind));

        return deserialize_rpc_object<detail::rpc_result_w_bind<IsCallback, R, Args...>>(
            serial_obj);
    }

    template<bool IsCallback, typename R, typename... Args>
    [[nodiscard]] static std::vector<uint8_t> serialize_result_w_bind(
        const detail::rpc_result_w_bind<IsCallback, R, Args...>& result)
    {
        return serialize_rpc_object<detail::rpc_result_w_bind<IsCallback, R, Args...>>(result);
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static detail::rpc_request<IsCallback, Args...> get_request(
        const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(verify_type(serial_obj,
                                 IsCallback ? rpc_type::callback_request : rpc_type::func_request)
            || verify_type(serial_obj,
                IsCallback ? rpc_type::callback_result_w_bind : rpc_type::func_result_w_bind));

        return deserialize_rpc_object<detail::rpc_request<IsCallback, Args...>>(serial_obj);
    }

    template<bool IsCallback, typename... Args>
    [[nodiscard]] static std::vector<uint8_t> serialize_request(
        const detail::rpc_request<IsCallback, Args...>& request)
    {
        return serialize_rpc_object<detail::rpc_request<IsCallback, Args...>>(request);
    }

    template<bool IsCallback>
    [[nodiscard]] static detail::rpc_error<IsCallback> get_error(
        const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(
            verify_type(serial_obj, IsCallback ? rpc_type::callback_error : rpc_type::func_error));

        return deserialize_rpc_object<detail::rpc_error<IsCallback>>(serial_obj);
    }

    template<bool IsCallback>
    [[nodiscard]] static std::vector<uint8_t> serialize_error(
        const detail::rpc_error<IsCallback>& error)
    {
        return serialize_rpc_object<detail::rpc_error<IsCallback>>(error);
    }

    [[nodiscard]] static callback_install_request get_callback_install(
        const std::vector<uint8_t>& serial_obj)
    {
        RPC_HPP_PRECONDITION(verify_type(serial_obj, rpc_type::callback_install_request));

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
        T ret_obj;
        bitsery_deserializer ser{ buffer };
        ser.deserialize_object(ret_obj);
        return ret_obj;
    }

    template<typename T>
    static std::vector<uint8_t> serialize_rpc_object(const T& rpc_obj)
    {
        bitsery_serializer ser{};
        ser.serialize_object(rpc_obj);
        return std::move(ser).object();
    }

    // Borrowed from Bitsery library for compatibility
    static unsigned extract_length(const bit_buffer& bytes, size_t& index) noexcept
    {
        RPC_HPP_PRECONDITION(index < bytes.size());

        const uint8_t high_byte = bytes[index];
        ++index;

        if (high_byte < 0x80U)
        {
            return high_byte;
        }

        assert(index < bytes.size());
        const uint8_t low_byte = bytes[index];
        ++index;

        if ((high_byte & 0x40U) != 0U)
        {
            assert(index < bytes.size());
            const uint16_t low_word = *reinterpret_cast<const uint16_t*>(&bytes[index]);
            ++index;
            return ((((high_byte & 0x3FU) << 8U) | low_byte) << 16U) | low_word;
        }

        return ((high_byte & 0x7FU) << 8U) | low_byte;
    }

    static bool verify_type(const bit_buffer& bytes, rpc_type type) noexcept
    {
        RPC_HPP_PRECONDITION(bytes.size() >= sizeof(int));

        return std::memcmp(&type, bytes.data(), sizeof(int)) == 0;
    }
};
} //namespace rpc_hpp::adapters
#endif
