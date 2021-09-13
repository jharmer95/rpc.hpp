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

#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <vector>

namespace rpc
{
namespace adapters
{
    using bit_buffer = std::vector<uint8_t>;
    using output_adapter = bitsery::OutputBufferAdapter<bit_buffer>;
    using input_adapter = bitsery::InputBufferAdapter<bit_buffer>;

    using bitsery_adapter = details::serial_adapter<bit_buffer, bit_buffer>;

    template<typename R, typename... Args>
    struct pack_helper
    {
        pack_helper() = default;

        pack_helper(std::string name, std::string err, R res,
            std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args)
            : func_name(std::move(name)), err_mesg(std::move(err)), result(std::move(res)),
              args(std::move(args))
        {
        }

        std::string func_name;
        std::string err_mesg;
        R result;
        std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args;
    };

    template<typename... Args>
    struct pack_helper<void, Args...>
    {
        pack_helper() = default;

        pack_helper(std::string name, std::string err,
            std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args)
            : func_name(std::move(name)), err_mesg(std::move(err)), args(std::move(args))
        {
        }

        std::string func_name;
        std::string err_mesg;
        std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args;
    };

    template<typename S, typename R, typename... Args>
    void serialize(S& s, pack_helper<R, Args...>& o)
    {
        s.text1b(o.func_name, 50);
        s.text1b(o.err_mesg, 255);

        if constexpr (std::is_arithmetic_v<R>)
        {
            if constexpr (sizeof(R) == 1)
            {
                s.value1b(o.result);
            }
            else if constexpr (sizeof(R) == 2)
            {
                s.value2b(o.result);
            }
            else if constexpr (sizeof(R) == 4)
            {
                s.value4b(o.result);
            }
            else if constexpr (sizeof(R) == 8)
            {
                s.value8b(o.result);
            }
        }
        else if constexpr (!std::is_void_v<R>)
        {
            s.object(o.result);
        }

        s.ext(o.args,
            bitsery::ext::StdTuple{
                bitsery::ext::OverloadValue<uint8_t, 1>{},
                bitsery::ext::OverloadValue<int8_t, 1>{},
                bitsery::ext::OverloadValue<uint16_t, 2>{},
                bitsery::ext::OverloadValue<int16_t, 2>{},
                bitsery::ext::OverloadValue<uint32_t, 4>{},
                bitsery::ext::OverloadValue<int32_t, 4>{},
                bitsery::ext::OverloadValue<uint64_t, 8>{},
                bitsery::ext::OverloadValue<int64_t, 8>{},
            });
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
                pack.clear_result();
                return pack;
            }
        }
        else
        {
            if (helper.err_mesg.empty())
            {
                return packed_func<R, Args...>{ std::move(helper.func_name), helper.result,
                    std::move(helper.args) };
            }
            else
            {
                packed_func<R, Args...> pack{ std::move(helper.func_name), helper.result,
                    std::move(helper.args) };
                pack.set_err_mesg(helper.err_mesg);
                pack.clear_result();
                return pack;
            }
        }
    }
} // namespace adapters

template<>
inline adapters::bit_buffer adapters::bitsery_adapter::to_bytes(adapters::bit_buffer serial_obj)
{
    return std::move(serial_obj);
}

template<>
inline adapters::bit_buffer adapters::bitsery_adapter::from_bytes(adapters::bit_buffer bytes)
{
    return std::move(bytes);
}

template<>
template<typename R, typename... Args>
adapters::bit_buffer pack_adapter<adapters::bitsery_adapter>::serialize_pack(
    const packed_func<R, Args...>& pack)
{
    using namespace adapters;

    const auto helper = to_helper(pack);

    bit_buffer buffer;

    bitsery::quickSerialization<output_adapter>(buffer, helper);

    return buffer;
}

template<>
template<typename R, typename... Args>
packed_func<R, Args...> pack_adapter<adapters::bitsery_adapter>::deserialize_pack(
    const adapters::bit_buffer& serial_obj)
{
    using namespace adapters;

    pack_helper<R, Args...> helper;

    bitsery::quickDeserialization(input_adapter{ serial_obj.begin(), serial_obj.size() }, helper);
    return from_helper(std::move(helper));
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
