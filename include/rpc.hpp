///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.1.0.0
///@date 11-18-2019
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2019, Jackson Harmer
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

#include "dispatcher.hpp"

#include <nlohmann/json/json.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

namespace njson = nlohmann;

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    static constexpr size_t nargs = sizeof...(Args);

    using result_type = R;

    template<size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
    };
};

template<typename R, typename... Args>
inline constexpr size_t function_param_count_v = function_traits<std::function<R(Args...)>>::nargs;

template<typename R, typename... Args>
using function_result_t = typename function_traits<std::function<R(Args...)>>::type;

template<size_t i, typename R, typename... Args>
using function_args_t = typename function_traits<std::function<R(Args...)>>::template arg<i>::type;

namespace rpc
{
std::shared_ptr<Dispatcher> DISPATCHER = std::make_shared<Dispatcher>();

template<typename, typename T>
struct is_serializable
{
    static_assert(std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type");
};

template<typename C, typename R, typename... Args>
struct is_serializable<C, R(Args...)>
{
private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().Serialize(std::declval<Args>()...)),
            R>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename C>
struct has_begin
{
private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().begin()), typename T::iterator>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename C>
struct has_end
{
private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().end()), typename T::iterator>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename C>
struct has_size
{
private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().size()), size_t>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename C>
struct is_container
    : std::integral_constant<bool, has_size<C>::value && has_begin<C>::value && has_end<C>::value>
{
};

template<typename T>
T DecodeArgContainer(const njson::json& obj_j, uint8_t* buf, size_t* count)
{
    T container;
    *count = 0UL;

    if (obj_j.is_array())
    {
        // Multi-value container (array)
        using P = typename T::value_type;
        static_assert(!std::is_void_v<P>,
            "Void containers are not supported, either cast to a different type or do the "
            "conversion "
            "manually!");

        if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                container.push_back(obj_j[i].get<P>());
            }
        }
        else if constexpr (is_container<P>::value)
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                size_t ncount = 0UL;
                container.push_back(DecodeArgContainer<P>(obj_j[i], buf, &ncount));
                *count += ncount;
            }
        }
        else if constexpr (is_serializable<P, njson::json(const P&)>::value)
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                container.push_back(P::DeSerialize(obj_j[i]));
            }
        }
        else
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                container.push_back(DISPATCHER->DeSerialize<P>(obj_j[i]));
            }
        }

        if (*count == 0UL)
        {
            *count = container.size();
        }

        return container;
    }

    // Single value container
    using P = typename T::value_type;
    static_assert(!std::is_void_v<P>,
        "Void containers are not supported, either cast to a different type or do the conversion "
        "manually!");

    if constexpr (is_container<P>::value)
    {
        size_t ncount = 0UL;
        container.push_back(DecodeArgContainer<P>(obj_j, buf, &ncount));
        *count += ncount;
    }
    else if constexpr (is_serializable<P, njson::json(const P&)>::value)
    {
        container.push_back(P::DeSerialize(obj_j));
    }
    else if constexpr (std::is_same_v<P, char>)
    {
        const auto str = obj_j.get<std::string>();
        std::copy(str.begin(), str.end(), std::back_inserter(container));
    }
    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
    {
        container.push_back(obj_j.get<P>());
    }
    else
    {
        container.push_back(DISPATCHER->DeSerialize<P>(obj_j));
    }

    if (*count == 0UL)
    {
        *count = 1UL;
    }

    return container;
}

template<typename T>
T DecodeArgPtr(const njson::json& obj_j, uint8_t* buf, size_t* count)
{
    *count = 1UL;

    if (obj_j.is_array())
    {
        // Multi-value pointer (array)
        using P = std::remove_pointer_t<T>;
        static_assert(!std::is_void_v<P>,
            "Void pointers are not supported, either cast to a different type or do the conversion "
            "manually!");

        const auto bufPtr = reinterpret_cast<T>(buf);

        if constexpr (is_serializable<P, njson::json(const P&)>::value)
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                const auto value = P::DeSerialize(obj_j[i]);
                bufPtr[i] = value;
            }
        }
        else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                bufPtr[i] = obj_j[i].get<P>();
            }
        }
        else
        {
            for (size_t i = 0; i < obj_j.size(); ++i)
            {
                const auto value = DISPATCHER->DeSerialize<P>(obj_j[i]);
                bufPtr[i] = value;
            }
        }

        *count = obj_j.size();
        return bufPtr;
    }

    // Single value pointer
    using P = std::remove_pointer_t<T>;
    static_assert(!std::is_void_v<P>,
        "Void pointers are not supported, either cast to a different type or do the conversion "
        "manually!");

    const auto bufPtr = reinterpret_cast<T>(buf);

    if constexpr (is_serializable<P, njson::json(const P&)>::value)
    {
        const auto value = P::DeSerialize(obj_j);
        *bufPtr = value;
    }
    else if constexpr (std::is_same_v<P, char>)
    {
        const auto str = obj_j.get<std::string>();
        std::copy(str.begin(), str.end(), bufPtr);
    }
    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
    {
        const auto value = obj_j.get<P>();
        *bufPtr = value;
    }
    else
    {
        const auto value = DISPATCHER->DeSerialize<P>(obj_j);
        *bufPtr = value;
    }

    return bufPtr;
}

template<typename T>
T DecodeArg(const njson::json& obj_j, uint8_t* buf, size_t* count, unsigned* paramNum)
{
    *paramNum += 1;
    *count = 1UL;

    if constexpr (std::is_pointer_v<T>)
    {
        return DecodeArgPtr<T>(obj_j, buf, count);
    }
    else if constexpr (is_serializable<T, njson::json(const T&)>::value)
    {
        return T::DeSerialize(obj_j);
    }
    else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
    {
        T retVal = obj_j.get<T>();
        return retVal;
    }
    else if constexpr (is_container<T>::value)
    {
        return DecodeArgContainer<T>(obj_j, buf, count);
    }
    else
    {
        return DISPATCHER->DeSerialize<T>(obj_j);
    }
}

template<typename T>
void EncodeArgs(njson::json& args_j, const size_t count, const T& val)
{
    if constexpr (std::is_pointer_v<T>)
    {
        using P = std::remove_pointer_t<T>;

        for (size_t i = 0; i < count; ++i)
        {
            if constexpr (is_serializable<P, njson::json(const P&)>::value)
            {
                args_j.push_back(P::Serialize(val[i]));
            }
            else if constexpr (std::is_same_v<P, char>)
            {
                args_j.push_back(std::string(val, count));
            }
            else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
            {
                args_j.push_back(val[i]);
            }
            else
            {
                args_j.push_back(DISPATCHER->Serialize<P>(val[i]));
            }
        }
    }
    else if constexpr (is_container<T>::value)
    {
        using P = typename T::value_type;

        if constexpr(std::is_same_v<P, std::string>)
        {
            for (const auto& v : val)
            {
                args_j.push_back(v);
            }
        }
        else if constexpr(is_container<P>::value)
        {
            for (const auto& c : val)
            {
                auto sargs_j = njson::json::array();
                EncodeArgs<P>(sargs_j, c.size(), c);
                args_j.push_back(sargs_j);
            }
        }
        else
        {
            for (const auto& v : val)
            {
                if constexpr (is_serializable<P, njson::json(const P&)>::value)
                {
                    args_j.push_back(P::Serialize(v));
                }
                else if constexpr (std::is_same_v<P, char>)
                {
                    args_j.push_back(std::string(val, count));
                }
                else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
                {
                    args_j.push_back(v);
                }
                else
                {
                    args_j.push_back(DISPATCHER->Serialize<P>(v));
                }
            }
        }
    }
    else
    {
        if constexpr (is_serializable<T, njson::json(const T&)>::value)
        {
            args_j.push_back(T::Serialize(val));
        }
        else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
        {
            args_j.push_back(val);
        }
        else
        {
            args_j.push_back(DISPATCHER->Serialize<T>(val));
        }
    }
}

template<typename F, typename... Ts, size_t... Is>
void for_each_tuple(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>)
{
    using expander = int[];
    (void)expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<typename F, typename... Ts>
void for_each_tuple(const std::tuple<Ts...>& tuple, F func)
{
    for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, std::function<R(Args...)> func)
{
    unsigned count = 0;

    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(4096);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{ DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
        obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)... };

    const auto result = std::apply(func, args);

    njson::json retObj_j;

    retObj_j["result"] = result;
    retObj_j["args"] = njson::json::array();
    auto& argList = retObj_j["args"];

    unsigned count2 = 0;

    for_each_tuple(args, [&argList, &buffers, &count2](const auto& x) {
        EncodeArgs(argList, buffers[count2++].first, x);
    });

    return retObj_j.dump();
}

std::string RunFromJSON(const njson::json& obj_j)
{
    const auto funcName = obj_j["function"].get<std::string>();
    const auto& argList = obj_j["args"];

    try
    {
        return DISPATCHER->Run(funcName, argList);
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
        return "{ \"result\": -1 }";
    }
}
} // namespace rpc
