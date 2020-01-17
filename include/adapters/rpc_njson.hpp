///@file adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
///@version 0.1.0.0
///@date 01-17-2020
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

namespace njson = nlohmann;

using JSONAdapter = rpc::serial_adapter<njson::json>;

template<>
JSONAdapter::serial_adapter(std::string_view obj_str) : m_serial_object(njson::json::parse(obj_str)) {}

template<>
template<typename Value>
Value JSONAdapter::get_value() const
{
    return m_serial_object.get<Value>();
}

template<>
template<typename Value>
Value JSONAdapter::get_value(const std::string& name) const
{
    return m_serial_object[name].get<Value>();
}

template<>
template<typename Value>
Value& JSONAdapter::get_value_ref()
{
    return m_serial_object;
}

template<>
template<typename Value>
Value& JSONAdapter::get_value_ref(const std::string& name)
{
    return m_serial_object[name];
}

template<>
template<typename Value>
Value& JSONAdapter::get_value_ref() const
{
    return m_serial_object;
}

template<>
template<typename Value>
Value& JSONAdapter::get_value_ref(const std::string& name) const
{
    return m_serial_object[name];
}

template<>
template<typename Value>
void JSONAdapter::set_value(Value value)
{
    m_serial_object = value;
}

template<>
template<typename Value>
void JSONAdapter::set_value(const std::string& name, Value value)
{
    m_serial_object[name] = value;
}

template<>
template<typename Value>
void JSONAdapter::push_back(Value value)
{
    if (!m_serial_object.is_array())
    {
        if (m_serial_object.is_null())
        {
            m_serial_object = njson::json::array();
        }
        else
        {
            const auto tmp = m_serial_object.get<Value>();
            m_serial_object = njson::json::array();
            m_serial_object.push_back(tmp);
        }
    }

    m_serial_object.push_back(value);
}

template<>
template<typename Value>
void JSONAdapter::append_value(const std::string& name, Value value)
{
    if (!m_serial_object[name].is_array())
    {
        if (m_serial_object[name].is_null())
        {
            m_serial_object[name] = njson::json::array();
        }
        else
        {
            const auto tmp = m_serial_object[name].get<Value>();
            m_serial_object[name] = njson::json::array();
            m_serial_object[name].push_back(tmp);
        }
    }

    m_serial_object[name].push_back(value);
}

template<>
inline std::string JSONAdapter::to_string() const
{
    return m_serial_object.dump();
}

template<>
inline bool JSONAdapter::is_array() const noexcept
{
    return m_serial_object.is_array();
}

template<>
inline bool JSONAdapter::is_empty() const noexcept
{
    return m_serial_object.is_null() || (m_serial_object.is_array() && m_serial_object.empty());
}

template<>
template<>
inline njson::json::iterator JSONAdapter::begin() noexcept
{
    return m_serial_object.begin();
}

template<>
template<>
inline njson::json::iterator JSONAdapter::end() noexcept
{
    return m_serial_object.end();
}

template<>
template<>
inline njson::json::const_iterator JSONAdapter::begin() const noexcept
{
    return m_serial_object.begin();
}

template<>
template<>
inline njson::json::const_iterator JSONAdapter::end() const noexcept
{
    return m_serial_object.end();
}

template<>
template<>
inline njson::json::reverse_iterator JSONAdapter::rbegin() noexcept
{
    return m_serial_object.rbegin();
}

template<>
template<>
inline njson::json::reverse_iterator JSONAdapter::rend() noexcept
{
    return m_serial_object.rend();
}

template<>
template<>
inline njson::json::const_reverse_iterator JSONAdapter::rbegin() const noexcept
{
    return m_serial_object.rbegin();
}

template<>
template<>
inline njson::json::const_reverse_iterator JSONAdapter::rend() const noexcept
{
    return m_serial_object.rend();
}

template<>
inline size_t JSONAdapter::size() const noexcept
{
    return m_serial_object.size();
}

template<>
inline njson::json JSONAdapter::operator[](size_t n) const
{
    return m_serial_object[n];
}

template<>
inline njson::json JSONAdapter::make_array() noexcept
{
    return njson::json::array();
}
