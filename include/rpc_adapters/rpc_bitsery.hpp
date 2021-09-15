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

#if !defined(RPC_HPP_ENABLE_BITSERY)
#    error 'rpc_bitsery' was included without defining 'RPC_HPP_ENABLE_BITSERY' Please define this macro or do not include this header!
#endif

#include "../rpc.hpp"

#include <limits>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <vector>

// TODO: Restructure adapter namespaces so these can sit in rpc::adapters::bitsery
extern const uint64_t RPC_HPP_BITSERY_MAX_FUNC_NAME_SZ;
extern const uint64_t RPC_HPP_BITSERY_MAX_STR_SZ;
extern const uint64_t RPC_HPP_BITSERY_MAX_CONTAINER_SZ;

#if defined(RPC_HPP_ENABLE_SERVER_CACHE)
namespace std
{
template<>
struct hash<std::vector<uint8_t>>
{
    size_t operator()(const std::vector<uint8_t>& vec) const
    {
        size_t seed = vec.size();

        for (const auto i : vec)
        {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
} // namespace std
#endif

namespace rpc
{
namespace adapters
{
    using bit_buffer = std::vector<uint8_t>;
    using output_adapter = bitsery::OutputBufferAdapter<bit_buffer>;
    using input_adapter = bitsery::InputBufferAdapter<bit_buffer>;

    using bitsery_adapter = details::serial_adapter<bit_buffer, bit_buffer>;

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
        using args_t = std::tuple<largest_t<std::remove_cv_t<std::remove_reference_t<Args>>>...>;
#endif

        pack_helper() = default;

        pack_helper(std::string name, std::string err, R res, args_t args)
            : func_name(std::move(name)), err_mesg(std::move(err)), result(std::move(res)),
              args(std::move(args))
        {
        }

        std::string func_name;
        std::string err_mesg;
        R result;
        args_t args;
    };

    template<typename... Args>
    struct pack_helper<void, Args...>
    {
#if defined(RPC_HPP_BITSERY_EXACT_SZ)
        using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
#else
        using args_t = std::tuple<largest_t<std::remove_cv_t<std::remove_reference_t<Args>>>...>;
#endif

        pack_helper() = default;

        pack_helper(std::string name, std::string err, args_t args)
            : func_name(std::move(name)), err_mesg(std::move(err)), args(std::move(args))
        {
        }

        std::string func_name;
        std::string err_mesg;
        args_t args;
    };

    template<typename S, typename R, typename... Args>
    void serialize(S& s, pack_helper<R, Args...>& o)
    {
        s.text1b(o.func_name, RPC_HPP_BITSERY_MAX_FUNC_NAME_SZ);
        s.text1b(o.err_mesg, RPC_HPP_BITSERY_MAX_STR_SZ);

        if constexpr (std::is_arithmetic_v<R>)
        {
            s.template value<sizeof(R)>(o.result);
        }
        else if constexpr (details::is_container_v<R>)
        {
            if constexpr (std::is_arithmetic_v<typename R::value_type>)
            {
                s.template container<sizeof(typename R::value_type)>(
                    o.result, RPC_HPP_BITSERY_MAX_CONTAINER_SZ);
            }
            else
            {
                s.container(o.result, RPC_HPP_BITSERY_MAX_CONTAINER_SZ);
            }
        }
        else if constexpr (!std::is_void_v<R>)
        {
            s.object(o.result);
        }

        s.ext(o.args,
            bitsery::ext::StdTuple{ [](S& s, std::string& str)
                { s.text1b(str, RPC_HPP_BITSERY_MAX_STR_SZ); },
                // Fallback serializer for integers, floats, and enums
                [](auto& s, auto& val)
                {
                    using T = std::remove_cv_t<std::remove_reference_t<decltype(val)>>;

                    if constexpr (std::is_arithmetic_v<T>)
                    {
#if defined(RPC_HPP_BITSERY_EXACT_SZ)
                        s.template value<sizeof(val)>(val);
#else
                        s.value8b(val);
#endif
                    }
                    else if constexpr (details::is_container_v<T>)
                    {
                        if constexpr (std::is_arithmetic_v<typename T::value_type>)
                        {
                            s.template container<sizeof(typename T::value_type)>(
                                val, RPC_HPP_BITSERY_MAX_CONTAINER_SZ);
                        }
                        else
                        {
                            s.container(val, RPC_HPP_BITSERY_MAX_CONTAINER_SZ);
                        }
                    }
                    else
                    {
                        s.object(val);
                    }
                } });
    }

    template<typename R, typename... Args>
    pack_helper<R, Args...> to_helper(const packed_func<R, Args...>& pack)
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

    template<typename R, typename... Args>
    packed_func<R, Args...> from_helper(pack_helper<R, Args...> helper)
    {
        if constexpr (std::is_void_v<R>)
        {
            if (helper.err_mesg.empty())
            {
                return packed_func<void, Args...>{ std::move(helper.func_name),
                    std::move(helper.args) };
            }
            else
            {
                packed_func<void, Args...> pack{ std::move(helper.func_name),
                    std::move(helper.args) };

                pack.set_err_mesg(helper.err_mesg);
                return pack;
            }
        }
        else
        {
            if (helper.err_mesg.empty())
            {
                return packed_func<R, Args...>{ std::move(helper.func_name),
                    std::move(helper.result), std::move(helper.args) };
            }
            else
            {
                packed_func<R, Args...> pack{ std::move(helper.func_name), std::move(helper.result),
                    std::move(helper.args) };

                pack.set_err_mesg(helper.err_mesg);
                pack.clear_result();
                return pack;
            }
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
} // namespace adapters

template<>
inline adapters::bit_buffer adapters::bitsery_adapter::to_bytes(adapters::bit_buffer serial_obj)
{
    return serial_obj;
}

template<>
inline adapters::bit_buffer adapters::bitsery_adapter::from_bytes(adapters::bit_buffer bytes)
{
    return bytes;
}

template<>
template<typename R, typename... Args>
adapters::bit_buffer pack_adapter<adapters::bitsery_adapter>::serialize_pack(
    const packed_func<R, Args...>& pack)
{
    using namespace adapters;

    const auto helper = to_helper(pack);

    bit_buffer buffer;
    const auto bytes_written = bitsery::quickSerialization<output_adapter>(buffer, helper);

    buffer.resize(bytes_written);
    return buffer;
}

template<>
template<typename R, typename... Args>
packed_func<R, Args...> pack_adapter<adapters::bitsery_adapter>::deserialize_pack(
    const adapters::bit_buffer& serial_obj)
{
    using namespace adapters;

    pack_helper<R, Args...> helper;

    const auto [error, success] = bitsery::quickDeserialization(
        input_adapter{ serial_obj.begin(), serial_obj.size() }, helper);

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

                default:
                    return "extra data on the end!";
            }
        }();

        throw std::runtime_error(
            std::string{ "Bitsery deserialization failed due to " } + error_mesg);
    }

    return from_helper(helper);
}

template<>
inline std::string pack_adapter<adapters::bitsery_adapter>::get_func_name(
    const adapters::bit_buffer& serial_obj)
{
    const uint8_t len = serial_obj.front();
    return { serial_obj.begin() + 1, serial_obj.begin() + 1 + len };
}

template<>
inline void pack_adapter<adapters::bitsery_adapter>::set_err_mesg(
    adapters::bit_buffer& serial_obj, std::string mesg)
{
    const uint8_t name_len = serial_obj.front();
    const uint8_t err_len = serial_obj.at(name_len + 1);
    const uint8_t new_err_len = mesg.size();

    if (new_err_len > err_len)
    {
        serial_obj.at(name_len + 1) = new_err_len;
        serial_obj.insert(serial_obj.begin() + name_len + 2, new_err_len - err_len, 0);
    }
    else if (new_err_len < err_len)
    {
        serial_obj.at(name_len + 1) = new_err_len;
        serial_obj.erase(serial_obj.begin() + name_len + 2,
            serial_obj.begin() + name_len + 2 + (err_len - new_err_len));
    }

    for (uint8_t i = 0; i < new_err_len; ++i)
    {
        serial_obj.at(name_len + 2 + i) = static_cast<uint8_t>(mesg.at(i));
    }
}
} // namespace rpc
