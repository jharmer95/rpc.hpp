///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.1.0.0
///@date 01-08-2020
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020, Jackson Harmer
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

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

namespace rpc
{
template<typename T_Serial>
class SerialAdapter
{
public:
    SerialAdapter() : m_serialObj() {}
    SerialAdapter(T_Serial obj) : m_serialObj(std::move(obj)) {}

    // TODO: Get SerialAdapter to handle serialization/deserialization functions
    //template<typename T_Value>
    //[[nodiscard]] static T_Serial Serialize(const T_Value&);

    //template<typename T_Value>
    //[[nodiscard]] static T_Value DeSerialize(const T_Serial&);

    template<typename T_Value>
    [[nodiscard]] T_Value GetValue() const;

    template<typename T_Value>
    [[nodiscard]] T_Value GetValue(const std::string& name) const;

    template<typename T_Value>
    [[nodiscard]] T_Value& GetValueRef();

    template<typename T_Value>
    [[nodiscard]] T_Value& GetValueRef(const std::string& name);

    template<typename T_Value>
    void SetValue(T_Value value);

    template<typename T_Value>
    void SetValue(const std::string& name, T_Value value);

    template<typename T_Value>
    void push_back(T_Value value);

    template<typename T_Value>
    void AppendValue(T_Value value);

    template<typename T_Value>
    void AppendValue(const std::string& name, T_Value value);

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] bool IsArray() const;
    [[nodiscard]] bool IsEmpty() const;

    template<typename T_SerialIter>
    T_SerialIter begin() const;

    template<typename T_SerialIter>
    T_SerialIter end() const;

    [[nodiscard]] T_Serial get() const { return m_serialObj; }
    [[nodiscard]] size_t size() const;
    T_Serial operator[](size_t n) const;
    static T_Serial EmptyArray();

protected:
    T_Serial m_serialObj;
};

template<typename T_Serial>
template<typename T_Value>
T_Value SerialAdapter<T_Serial>::GetValue() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
T_Value SerialAdapter<T_Serial>::GetValue(const std::string& name) const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
T_Value& SerialAdapter<T_Serial>::GetValueRef()
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
T_Value& SerialAdapter<T_Serial>::GetValueRef(const std::string& name)
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
void SerialAdapter<T_Serial>::SetValue(T_Value value)
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
void SerialAdapter<T_Serial>::SetValue(const std::string& name, T_Value value)
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
void SerialAdapter<T_Serial>::push_back(T_Value value)
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_Value>
void SerialAdapter<T_Serial>::AppendValue(T_Value value)
{
    push_back(value);
}

template<typename T_Serial>
template<typename T_Value>
void SerialAdapter<T_Serial>::AppendValue(const std::string& name, T_Value value)
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
std::string SerialAdapter<T_Serial>::ToString() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
bool SerialAdapter<T_Serial>::IsArray() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
bool SerialAdapter<T_Serial>::IsEmpty() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_SerialIter>
T_SerialIter SerialAdapter<T_Serial>::begin() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
template<typename T_SerialIter>
T_SerialIter SerialAdapter<T_Serial>::end() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
size_t SerialAdapter<T_Serial>::size() const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
T_Serial SerialAdapter<T_Serial>::operator[](size_t n) const
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
T_Serial SerialAdapter<T_Serial>::EmptyArray()
{
    const std::string errMesg =
        "Serialization type: \"" + std::string(typeid(T_Serial).name()) + "\" is not supported!";

    throw std::logic_error(errMesg);
}

template<typename T_Serial>
extern std::string dispatch(const std::string& funcName, const T_Serial& obj);

template<typename T_Obj, typename T_Serial>
T_Serial Serialize(const T_Obj&)
{
    throw std::logic_error("Type has not been provided with a Serialize method!");
}

template<typename T_Obj, typename T_Serial>
T_Obj DeSerialize(const T_Serial&)
{
    throw std::logic_error("Type has not been provided with a DeSerialize method!");
}

template<typename, typename T>
struct is_serializable_base
{
    static_assert(std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type");
};

template<typename C, typename R, typename... Args>
struct is_serializable_base<C, R(Args...)>
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

template<typename, typename T>
struct is_deserializable_base
{
    static_assert(std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type");
};

template<typename C, typename R, typename... Args>
struct is_deserializable_base<C, R(Args...)>
{
private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().DeSerialize(std::declval<Args>()...)),
            R>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename T_Obj, typename T_Serial>
struct is_serializable : std::integral_constant<bool,
                             is_serializable_base<T_Obj, T_Serial(const T_Obj&)>::value
                                 && is_deserializable_base<T_Obj, T_Obj(const T_Serial&)>::value>
{
};

template<typename T_Obj, typename T_Serial>
inline constexpr bool is_serializable_v = is_serializable<T_Obj, T_Serial>::value;

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

template<typename C>
inline constexpr bool is_container_v = is_container<C>::value;

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

template<typename T_Serial, typename T_Value>
T_Value DecodeArgContainer(const T_Serial& obj, uint8_t* buf, size_t* count)
{
#ifdef _DEBUG
    const auto t_name = typeid(T_Value).name();
#endif

    T_Value container;
    *count = 0UL;
    SerialAdapter adapter(obj);

    if (adapter.IsArray())
    {
        // Multi-value container (array)
        using P = typename T_Value::value_type;
        static_assert(!std::is_void_v<P>,
            "Void containers are not supported, either cast to a different type or do the "
            "conversion manually!");

        if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            std::transform(adapter.begin(), adapter.end(), std::back_inserter(container),
                [](const T_Serial& ser) -> P {
                    const SerialAdapter adp(ser);
                    return adp.template GetValue<P>();
                });
        }
        else if constexpr (is_container_v<P>)
        {
            for (const T_Serial& ser : adapter)
            {
                size_t ncount = 0Ul;
                container.push_back(DecodeArgContainer<P>(ser, buf, &ncount));
                *count += ncount;
            }
        }
        else if constexpr (is_serializable_v<P, T_Serial>)
        {
            std::transform(adapter.begin(), adapter.end(), std::back_inserter(container),
                [](const T_Serial& ser) -> P { return P::DeSerialize(ser); });
        }
        else
        {
            std::transform(adapter.begin(), adapter.end(), std::back_inserter(container),
                [](const T_Serial& ser) -> P { return DeSerialize<P>(ser); });
        }

        if (*count == 0UL)
        {
            *count = container.size();
        }

        return container;
    }

    // Single value container
    using P = typename T_Value::value_type;
    static_assert(!std::is_void_v<P>,
        "Void containers are not supported, either cast to a different type or do the conversion "
        "manually!");

    if constexpr (is_container_v<P>)
    {
        size_t ncount = 0UL;
        container.push_back(DecodeArgContainer<P>(obj, buf, &ncount));
        *count += ncount;
    }
    else if constexpr (is_serializable_v<P, T_Serial>)
    {
        container.push_back(P::DeSerialize(obj));
    }
    else if constexpr (std::is_same_v<P, char>)
    {
        const auto str = adapter.template GetValue<std::string>();
        std::copy(str.begin(), str.end(), std::back_inserter(container));
    }
    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
    {
        container.push_back(adapter.template GetValue<P>());
    }
    else
    {
        container.push_back(DeSerialize<P>(obj));
    }

    if (*count == 0UL)
    {
        *count = 1UL;
    }

    return container;
}

template<typename T_Serial, typename T_Value>
T_Value DecodeArgPtr(const T_Serial& obj, uint8_t* buf, size_t* count)
{
#ifdef _DEBUG
    const auto t_name = typeid(T_Value).name();
#endif

    *count = 1UL;
    SerialAdapter adapter(obj);

    if (adapter.IsEmpty())
    {
        return nullptr;
    }

    if (adapter.IsArray())
    {
        // Multi-value pointer (array)
        using P = std::remove_pointer_t<T_Value>;
        static_assert(!std::is_void_v<P>,
            "Void pointers are not supported, either cast to a different type or do the conversion "
            "manually!");

        if constexpr (is_serializable_v<P, T_Serial>)
        {
            for (size_t i = 0; i < adapter.size(); ++i)
            {
                const auto value = P::DeSerialize(adapter[i]);
                memcpy(&buf[i * sizeof(value)], &value, sizeof(value));
            }
        }
        else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
        {
            for (size_t i = 0; i < adapter.size(); ++i)
            {
                const auto sub = SerialAdapter(adapter[i]);
                const auto value = sub.template GetValue<P>();
                memcpy(&buf[i * sizeof(value)], &value, sizeof(value));
            }
        }
        else
        {
            for (size_t i = 0; i < adapter.size(); ++i)
            {
                const auto value = DeSerialize<P>(adapter[i]);
                memcpy(&buf[i * sizeof(value)], &value, sizeof(value));
            }
        }

        *count = adapter.size();
        return reinterpret_cast<T_Value>(buf);
    }

    // Single value pointer
    using P = std::remove_pointer_t<T_Value>;
    static_assert(!std::is_void_v<P>,
        "Void pointers are not supported, either cast to a different type or do the conversion "
        "manually!");

    if constexpr (is_serializable_v<P, T_Serial>)
    {
        const auto value = P::DeSerialize(obj);
        memcpy(buf, &value, sizeof(value));
    }
    else if constexpr (std::is_same_v<P, char>)
    {
        const auto str = adapter.template GetValue<std::string>();
        std::copy(str.begin(), str.end(), reinterpret_cast<T_Value>(buf));
    }
    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
    {
        const auto value = adapter.template GetValue<P>();
        memcpy(buf, &value, sizeof(value));
    }
    else
    {
        const auto value = DeSerialize<P>(obj);
        memcpy(buf, &value, sizeof(value));
    }

    return reinterpret_cast<T_Value>(buf);
}

template<typename T_Serial, typename T_Value>
T_Value DecodeArg(const T_Serial& obj, uint8_t* buf, size_t* count, unsigned* paramNum)
{
#ifdef _DEBUG
    const auto t_name = typeid(T_Value).name();
#endif

    *paramNum += 1;
    *count = 1UL;

    if constexpr (std::is_pointer_v<T_Value>)
    {
        return DecodeArgPtr<T_Serial, T_Value>(obj, buf, count);
    }
    else if constexpr (is_serializable_v<T_Value, T_Serial>)
    {
        return T_Value::DeSerialize(obj);
    }
    else if constexpr (std::is_arithmetic_v<T_Value> || std::is_same_v<T_Value, std::string>)
    {
        const SerialAdapter adapter(obj);
        return adapter.template GetValue<T_Value>();
    }
    else if constexpr (is_container_v<T_Value>)
    {
        return DecodeArgContainer<T_Value>(obj, buf, count);
    }
    else
    {
        return DeSerialize<T_Value>(obj);
    }
}

template<typename T_Serial, typename T_Value>
void EncodeArgs(T_Serial& obj, const size_t count, const T_Value& val)
{
#ifdef _DEBUG
    const auto t_name = typeid(T_Value).name();
#endif

    SerialAdapter adapter(obj);

    if constexpr (std::is_pointer_v<T_Value>)
    {
        if (val == nullptr)
        {
            adapter.push_back(T_Serial{});
        }
        else
        {
            using P = std::remove_pointer_t<T_Value>;

            for (size_t i = 0; i < count; ++i)
            {
                if constexpr (is_serializable_v<P, T_Serial>)
                {
                    adapter.push_back(P::Serialize(val[i]));
                }
                else if constexpr (std::is_same_v<P, char>)
                {
                    if (val[0] == '\0')
                    {
                        adapter.push_back("");
                    }
                    else
                    {
                        adapter.push_back(std::string(val, count));
                    }
                }
                else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
                {
                    adapter.push_back(val[i]);
                }
                else
                {
                    adapter.push_back(Serialize<P, T_Serial>(val[i]));
                }
            }
        }
    }
    else if constexpr (is_container_v<T_Value>)
    {
        using P = typename T_Value::value_type;

        if constexpr (std::is_same_v<P, std::string>)
        {
            std::copy(val.begin(), val.end(), std::back_inserter(adapter));
        }
        else if constexpr (is_container_v<P>)
        {
            for (const auto& c : val)
            {
                auto args = SerialAdapter<T_Serial>::EmptyArray();
                EncodeArgs<P>(args, c.size(), c);
                adapter.push_back(args);
            }
        }
        else
        {
            std::transform(
                val.begin(), val.end(), std::back_inserter(adapter), [&val, &count](const auto& v) {
                    if constexpr (is_serializable_v<P, T_Serial>)
                    {
                        return P::Serialize(v);
                    }
                    else if constexpr (std::is_same_v<P, char>)
                    {
                        return std::string(val, count);
                    }
                    else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
                    {
                        return v;
                    }
                    else
                    {
                        return Serialize<P>(v);
                    }
                });
        }
    }
    else
    {
        if constexpr (is_serializable_v<T_Value, T_Serial>)
        {
            adapter.push_back(T_Value::Serialize(val));
        }
        else if constexpr (std::is_arithmetic_v<T_Value> || std::is_same_v<T_Value, std::string>)
        {
            adapter.push_back(val);
        }
        else
        {
            adapter.push_back(Serialize<T_Value>(val));
        }
    }

    obj = adapter.get();
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

// Support for other Windows (x86) calling conventions
#if defined(_WIN32) && !defined(_WIN64)
template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, std::function<R __stdcall(Args...)> func)
{
    unsigned count = 0;
    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
            obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)...
    };

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

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, R(__stdcall* func)(Args...))
{
    unsigned count = 0;
    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
            obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)...
    };

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

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, std::function<R __fastcall(Args...)> func)
{
    unsigned count = 0;
    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
            obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)...
    };

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

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, R(__fastcall* func)(Args...))
{
    unsigned count = 0;
    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
            obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)...
    };

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

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, std::function<R __vectorcall(Args...)> func)
{
    unsigned count = 0;
    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
            obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)...
    };

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

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, R(__vectorcall* func)(Args...))
{
    unsigned count = 0;

    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<std::remove_cv_t<std::remove_reference_t<Args>>>(
            SerialAdapter(adapter[count]), buffers[count].second.get(),
            &(buffers[count].first), &count)...
    };

    const auto result = std::apply(func, args);

    SerialAdapter<T_Serial> retSer;

    retSer.SetValue("result", result);
    retSer.SetValue("args", retSer.EmptyArray());
    auto& argList = retSer.GetValueRef("args");

    unsigned count2 = 0;

    for_each_tuple(args, [&argList, &buffers, &count2](const auto& x) {
        EncodeArgs(argList, buffers[count2++].first, x);
    });

    return retSer.ToString();
}
#endif

template<typename T_Serial, typename R, typename... Args>
std::string RunCallBack(const T_Serial& obj, std::function<R(Args...)> func)
{
    unsigned count = 0;

    SerialAdapter adapter(obj);

    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<T_Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter[count], buffers[count].second.get(),
            &(buffers[count].first), &count)...
    };

    const auto result = std::apply(func, args);

    SerialAdapter<T_Serial> retSer;

    retSer.SetValue("result", result);
    retSer.SetValue("args", retSer.EmptyArray());
    auto& argList = retSer.template GetValueRef<T_Serial>("args");

    unsigned count2 = 0;

    for_each_tuple(args, [&argList, &buffers, &count2](const auto& x) {
        EncodeArgs(argList, buffers[count2++].first, x);
    });

    return retSer.ToString();
}

template<typename T_Serial, typename R, typename... Args>
std::string RunCallBack(const T_Serial& obj, R (*func)(Args...))
{
    unsigned count = 0;

    SerialAdapter adapter(obj);

    std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
        function_param_count_v<R, Args...>>
        buffers;

    for (auto& buf : buffers)
    {
        buf.first = 0UL;
        buf.second = std::make_unique<unsigned char[]>(64U * 1024U);
    }

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args{
        DecodeArg<T_Serial, std::remove_cv_t<std::remove_reference_t<Args>>>(
            adapter[count], buffers[count].second.get(), &(buffers[count].first), &count)...
    };

    const auto result = std::apply(func, args);

    SerialAdapter<T_Serial> retSer;

    retSer.SetValue("result", result);
    retSer.SetValue("args", retSer.EmptyArray());
    auto& argList = retSer.template GetValueRef<T_Serial>("args");

    unsigned count2 = 0;

    for_each_tuple(args, [&argList, &buffers, &count2](const auto& x) {
        EncodeArgs(argList, buffers[count2++].first, x);
    });

    return retSer.ToString();
}

template<typename T_Serial>
std::string Run(const T_Serial& obj)
{
    const auto adapter = SerialAdapter(obj);
    const auto funcName = adapter.template GetValue<std::string>("function");
    const auto argList = adapter.template GetValue<T_Serial>("args");

    try
    {
        return dispatch<T_Serial>(funcName, argList);
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
        SerialAdapter<T_Serial> result;
        result.SetValue("result", -1);
        return result.ToString();
    }
}
} // namespace rpc
