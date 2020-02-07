///@file adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.1.0.0
///@date 02-07-2020
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

#include <string_view>

using njson = nlohmann::json;

/// @brief Adapter to utilize nlohmann/json with rpc.hpp
using njson_adapter = rpc::serial_adapter<njson>;

template<>
njson_adapter::serial_adapter(std::string_view obj_str) : m_serial_object(njson::parse(obj_str)) {}

template<>
template<typename Value>
Value njson_adapter::get_value() const
{
    return m_serial_object.get<Value>();
}

template<>
template<typename Value>
Value njson_adapter::get_value(const std::string& name) const
{
    return m_serial_object[name].get<Value>();
}

template<>
template<typename Value>
Value njson_adapter::get_value(const size_t index) const
{
    return m_serial_object[index];
}

template<>
template<typename Value>
Value& njson_adapter::get_value_ref()
{
    return m_serial_object;
}

template<>
template<typename Value>
Value& njson_adapter::get_value_ref(const std::string& name)
{
    return m_serial_object[name];
}

template<>
template<typename Value>
Value& njson_adapter::get_value_ref(const size_t index)
{
    return m_serial_object[index];
}

template<>
template<typename Value>
const Value& njson_adapter::get_value_ref() const
{
    return m_serial_object;
}

template<>
template<typename Value>
const Value& njson_adapter::get_value_ref(const std::string& name) const
{
    return m_serial_object[name];
}

template<>
template<typename Value>
const Value& njson_adapter::get_value_ref(const size_t index) const
{
    return m_serial_object[index];
}

template<>
template<typename Value>
void njson_adapter::set_value(Value value)
{
    m_serial_object = value;
}

template<>
template<typename Value>
void njson_adapter::set_value(const std::string& name, Value value)
{
    m_serial_object[name] = value;
}

template<>
template<typename Value>
void njson_adapter::set_value(const size_t index, Value value)
{
    m_serial_object[index] = value;
}

template<>
template<typename Value>
void njson_adapter::push_back(Value value)
{
    if (!m_serial_object.is_array())
    {
        if (m_serial_object.is_null())
        {
            m_serial_object = njson::array();
        }
        else
        {
            const auto tmp = m_serial_object.get<Value>();
            m_serial_object = njson::array();
            m_serial_object.push_back(tmp);
        }
    }

    m_serial_object.push_back(value);
}

template<>
template<typename Value>
void njson_adapter::append_value(const std::string& name, Value value)
{
    if (!m_serial_object[name].is_array())
    {
        if (m_serial_object[name].is_null())
        {
            m_serial_object[name] = njson::array();
        }
        else
        {
            const auto tmp = m_serial_object[name].get<Value>();
            m_serial_object[name] = njson::array();
            m_serial_object[name].push_back(tmp);
        }
    }

    m_serial_object[name].push_back(value);
}

template<>
inline std::string njson_adapter::to_string() const
{
    return m_serial_object.dump();
}

template<>
inline bool njson_adapter::is_array() const noexcept
{
    return m_serial_object.is_array();
}

template<>
inline bool njson_adapter::is_empty() const noexcept
{
    return m_serial_object.is_null() || (m_serial_object.is_array() && m_serial_object.empty());
}

template<>
template<>
inline njson::iterator njson_adapter::begin() noexcept
{
    return m_serial_object.begin();
}

template<>
template<>
inline njson::iterator njson_adapter::end() noexcept
{
    return m_serial_object.end();
}

template<>
template<>
inline njson::const_iterator njson_adapter::begin() const noexcept
{
    return m_serial_object.begin();
}

template<>
template<>
inline njson::const_iterator njson_adapter::end() const noexcept
{
    return m_serial_object.end();
}

template<>
template<>
inline njson::reverse_iterator njson_adapter::rbegin() noexcept
{
    return m_serial_object.rbegin();
}

template<>
template<>
inline njson::reverse_iterator njson_adapter::rend() noexcept
{
    return m_serial_object.rend();
}

template<>
template<>
inline njson::const_reverse_iterator njson_adapter::rbegin() const noexcept
{
    return m_serial_object.rbegin();
}

template<>
template<>
inline njson::const_reverse_iterator njson_adapter::rend() const noexcept
{
    return m_serial_object.rend();
}

template<>
inline size_t njson_adapter::size() const noexcept
{
    return m_serial_object.size();
}

template<>
inline njson& njson_adapter::operator[](size_t n)
{
    return m_serial_object[n];
}

template<>
inline const njson& njson_adapter::operator[](size_t n) const
{
    return m_serial_object[n];
}

template<>
inline njson njson_adapter::make_array(size_t sz) noexcept
{
    if (sz == 0)
    {
        return njson::array();
    }

    auto obj = njson::array();

    for (size_t i = 0; i < sz; ++i)
    {
        obj.push_back(nullptr);
    }

    return obj;
}
