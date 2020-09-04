///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.1.0.0
///@date 08-25-2020
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

#include <nlohmann/json.hpp>

#include <utility>

// -------- SECTION: JSON --------
#if defined(RPC_HPP_NJSON_ENABLED)

using njson = nlohmann::json;
using njson_adapter = rpc::serial_adapter<njson>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> njson_adapter::to_packed_func(const njson& serial_obj)
{
    unsigned i = 0;

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<njson, Args>(
        serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (serial_obj.contains("result") && !serial_obj["result"].is_null())
        {
            return packed_func<R, Args...>(
                serial_obj["func_name"], serial_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(serial_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(serial_obj["func_name"], args);
    }
}

template<>
template<typename R, typename... Args>
njson njson_adapter::from_packed_func(const packed_func<R, Args...>& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { ret_j["args"].push_back(x); });

    return ret_j;
}

template<>
inline std::string njson_adapter::to_string(const njson& serial_obj)
{
    return serial_obj.dump();
}

template<>
inline njson njson_adapter::from_string(const std::string& str)
{
    return njson::parse(str);
}

template<>
inline std::string njson_adapter::extract_func_name(const njson& obj)
{
    return obj["func_name"].get<std::string>();
}

template<>
inline njson njson_adapter::make_sub_object(const njson& obj, unsigned index)
{
    return obj[index];
}

template<>
inline njson njson_adapter::make_sub_object(const njson& obj, const std::string& name)
{
    return obj[name];
}

template<>
template<typename T>
T njson_adapter::get_value(const njson& obj)
{
    return obj.get<T>();
}

template<>
template<typename Container>
void njson_adapter::populate_array(const njson& obj, Container& container)
{
    static_assert(is_container_v<Container>, "Type is not a container!");
    using value_t = typename Container::value_type;

    for (const auto& val : obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}
#endif

// -------- SECTION: CBOR --------
#if defined(RPC_HPP_NCBOR_ENABLED)
struct ncbor
{
    explicit ncbor(std::vector<uint8_t> vec) : value(std::move(vec)) {}

    explicit operator std::vector<uint8_t>() const { return value; }

    std::vector<uint8_t> value;
};

using ncbor_adapter = rpc::serial_adapter<ncbor>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> ncbor_adapter::to_packed_func(const ncbor& serial_obj)
{
    unsigned i = 0;
    const njson j_obj = njson::from_cbor(serial_obj.value);

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<Args>(serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (j_obj.contains("result") && !j_obj["result"].is_null())
        {
            return packed_func<R, Args...>(j_obj["func_name"], j_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(j_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(j_obj["func_name"], args);
    }
}

template<>
template<typename R, typename... Args>
ncbor ncbor_adapter::from_packed_func(const packed_func<R, Args...>& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { ret_j["args"].push_back(x); });

    return ncbor(njson::to_cbor(ret_j));
}

template<>
inline std::string ncbor_adapter::to_string(const ncbor& serial_obj)
{
    return njson::from_cbor(serial_obj.value).dump();
}

template<>
inline ncbor ncbor_adapter::from_string(const std::string& str)
{
    return ncbor(njson::to_cbor(njson::parse(str)));
}

template<>
inline std::string ncbor_adapter::extract_func_name(const ncbor& obj)
{
    return njson::from_cbor(obj.value)["func_name"].get<std::string>();
}

template<>
inline ncbor ncbor_adapter::make_sub_object(const ncbor& obj, unsigned index)
{
    return njson::to_cbor(njson::from_cbor(obj)[index]);
}

template<>
inline ncbor ncbor_adapter::make_sub_object(const ncbor& obj, const std::string& name)
{
    return njson::to_cbor(njson::from_cbor(obj)[name]);
}

template<>
template<typename T>
T ncbor_adapter::get_value(const ncbor& obj)
{
    return njson::from_cbor(obj).get<T>();
}

template<>
template<typename Container, typename = std::enable_if_t<rpc::is_container_v<Container>>>
void ncbor_adapter::populate_array(const ncbor& obj, Container& container)
{
    using value_t = typename Container::value_type;
    njson j_obj = json::from_cbor(obj);

    for (const auto& val : j_obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}
#endif

// -------- SECTION: BSON --------
#if defined(RPC_HPP_NBSON_ENABLED)
struct nbson
{
    explicit nbson(std::vector<uint8_t> vec) : value(std::move(vec)) {}

    explicit operator std::vector<uint8_t>() const { return value; }

    std::vector<uint8_t> value;
};

using nbson_adapter = rpc::serial_adapter<nbson>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> nbson_adapter::to_packed_func(const nbson& serial_obj)
{
    unsigned i = 0;
    const njson j_obj = njson::from_bson(serial_obj.value);

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<Args>(serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (j_obj.contains("result") && !j_obj["result"].is_null())
        {
            return packed_func<R, Args...>(j_obj["func_name"], j_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(j_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(j_obj["func_name"], args);
    }
}

template<>
template<typename R, typename... Args>
nbson nbson_adapter::from_packed_func(const packed_func<R, Args...>& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { ret_j["args"].push_back(x); });

    return nbson(njson::to_bson(ret_j));
}

template<>
inline std::string nbson_adapter::to_string(const nbson& serial_obj)
{
    return njson::from_bson(serial_obj.value).dump();
}

template<>
inline nbson nbson_adapter::from_string(const std::string& str)
{
    return nbson(njson::to_bson(njson::parse(str)));
}

template<>
inline std::string nbson_adapter::extract_func_name(const nbson& obj)
{
    return njson::from_bson(obj.value)["func_name"].get<std::string>();
}

template<>
inline nbson nbson_adapter::make_sub_object(const nbson& obj, unsigned index)
{
    return njson::to_bson(njson::from_bson(obj)[index]);
}

template<>
inline nbson nbson_adapter::make_sub_object(const nbson& obj, const std::string& name)
{
    return njson::to_bson(njson::from_bson(obj)[name]);
}

template<>
template<typename T>
T nbson_adapter::get_value(const nbson& obj)
{
    return njson::from_bson(obj).get<T>();
}

template<>
template<typename Container, typename = std::enable_if_t<rpc::is_container_v<Container>>>
void nbson_adapter::populate_array(const nbson& obj, Container& container)
{
    using value_t = typename Container::value_type;
    njson j_obj = json::from_bson(obj);

    for (const auto& val : j_obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}
#endif

// -------- SECTION: MSGPACK --------
#if defined(RPC_HPP_NMSGPACK_ENABLED)
struct nmsgpack
{
    explicit nmsgpack(std::vector<uint8_t> vec) : value(std::move(vec)) {}

    explicit operator std::vector<uint8_t>() const { return value; }

    std::vector<uint8_t> value;
};

using nmsgpack_adapter = rpc::serial_adapter<nmsgpack>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> nmsgpack_adapter::to_packed_func(const nmsgpack& serial_obj)
{
    unsigned i = 0;
    const njson j_obj = njson::from_msgpack(serial_obj.value);

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<Args>(serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (j_obj.contains("result") && !j_obj["result"].is_null())
        {
            return packed_func<R, Args...>(j_obj["func_name"], j_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(j_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(j_obj["func_name"], args);
    }
}

template<>
template<typename R, typename... Args>
nmsgpack nmsgpack_adapter::from_packed_func(const packed_func<R, Args...>& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { ret_j["args"].push_back(x); });

    return nmsgpack(njson::to_msgpack(ret_j));
}

template<>
inline std::string nmsgpack_adapter::to_string(const nmsgpack& serial_obj)
{
    return njson::from_msgpack(serial_obj.value).dump();
}

template<>
inline nmsgpack nmsgpack_adapter::from_string(const std::string& str)
{
    return nmsgpack(njson::to_msgpack(njson::parse(str)));
}

template<>
inline std::string nmsgpack_adapter::extract_func_name(const nmsgpack& obj)
{
    return njson::from_msgpack(obj.value)["func_name"].get<std::string>();
}

template<>
inline nmsgpack nmsgpack_adapter::make_sub_object(const nmsgpack& obj, unsigned index)
{
    return njson::to_msgpack(njson::from_msgpack(obj)[index]);
}

template<>
inline nmsgpack nmsgpack_adapter::make_sub_object(const nmsgpack& obj, const std::string& name)
{
    return njson::to_msgpack(njson::from_msgpack(obj)[name]);
}

template<>
template<typename T>
T nmsgpack_adapter::get_value(const nmsgpack& obj)
{
    return njson::from_msgpack(obj).get<T>();
}

template<>
template<typename Container, typename = std::enable_if_t<rpc::is_container_v<Container>>>
void nmsgpack_adapter::populate_array(const nmsgpack& obj, Container& container)
{
    using value_t = typename Container::value_type;
    njson j_obj = json::from_msgpack(obj);

    for (const auto& val : j_obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}
#endif

// -------- SECTION: UBJSON --------
#if defined(RPC_HPP_NUBJSON_ENABLED)
struct nubjson
{
    explicit nubjson(std::vector<uint8_t> vec) : value(std::move(vec)) {}

    explicit operator std::vector<uint8_t>() const { return value; }

    std::vector<uint8_t> value;
};

using nubjson_adapter = rpc::serial_adapter<nubjson>;

template<>
template<typename R, typename... Args>
rpc::packed_func<R, Args...> nubjson_adapter::to_packed_func(const nubjson& serial_obj)
{
    unsigned i = 0;
    const njson j_obj = njson::from_ubjson(serial_obj.value);

    std::array<std::any, sizeof...(Args)> args{ details::args_from_serial<Args>(serial_obj, i)... };

    if constexpr (!std::is_void_v<R>)
    {
        if (j_obj.contains("result") && !j_obj["result"].is_null())
        {
            return packed_func<R, Args...>(j_obj["func_name"], j_obj["result"].get<R>(), args);
        }

        return packed_func<R, Args...>(j_obj["func_name"], std::nullopt, args);
    }
    else
    {
        return packed_func<void, Args...>(j_obj["func_name"], args);
    }
}

template<>
template<typename R, typename... Args>
nubjson nubjson_adapter::from_packed_func(const packed_func<R, Args...>& pack)
{
    njson ret_j;

    ret_j["func_name"] = pack.get_func_name();
    ret_j["result"] = nullptr;

    if constexpr (!std::is_void_v<R>)
    {
        if (pack)
        {
            ret_j["result"] = *pack.get_result();
        }
    }

    ret_j["args"] = njson::array();
    unsigned i = 0;

    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argTup{
        details::args_from_packed<Args, R, Args...>(pack, i)...
    };

    rpc::for_each_tuple(argTup, [&ret_j](auto x) { ret_j["args"].push_back(x); });

    return nubjson(njson::to_ubjson(ret_j));
}

template<>
inline std::string nubjson_adapter::to_string(const nubjson& serial_obj)
{
    return njson::from_ubjson(serial_obj.value).dump();
}

template<>
inline nubjson nubjson_adapter::from_string(const std::string& str)
{
    return nubjson(njson::to_ubjson(njson::parse(str)));
}

template<>
inline std::string nubjson_adapter::extract_func_name(const nubjson& obj)
{
    return njson::from_ubjson(obj.value)["func_name"].get<std::string>();
}

template<>
inline nubjson nubjson_adapter::make_sub_object(const nubjson& obj, unsigned index)
{
    return njson::to_ubjson(njson::from_ubjson(obj)[index]);
}

template<>
inline nubjson nubjson_adapter::make_sub_object(const nubjson& obj, const std::string& name)
{
    return njson::to_ubjson(njson::from_ubjson(obj)[name]);
}

template<>
template<typename T>
T nubjson_adapter::get_value(const nubjson& obj)
{
    return njson::from_ubjson(obj).get<T>();
}

template<>
template<typename Container, typename = std::enable_if_t<rpc::is_container_v<Container>>>
void nubjson_adapter::populate_array(const nubjson& obj, Container& container)
{
    using value_t = typename Container::value_type;
    njson j_obj = json::from_ubjson(obj);

    for (const auto& val : j_obj)
    {
        container.push_back(details::arg_from_serial<njson, value_t>(val));
    }
}
#endif
