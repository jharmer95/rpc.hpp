///@file adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann/json (https://github.com/nlohmann/json)
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

#include <nlohmann/json.hpp>

namespace njson = nlohmann;

using JSONAdapter = rpc::SerialAdapter<njson::json>;

template<>
template<typename T_Value>
T_Value JSONAdapter::GetValue() const
{
    return m_serialObj.get<T_Value>();
}

template<>
template<typename T_Value>
T_Value JSONAdapter::GetValue(const std::string& name) const
{
    return m_serialObj[name].get<T_Value>();
}

template<>
template<typename T_Value>
T_Value& JSONAdapter::GetValueRef()
{
    return m_serialObj;
}

template<>
template<typename T_Value>
T_Value& JSONAdapter::GetValueRef(const std::string& name)
{
    return m_serialObj[name];
}

template<>
template<typename T_Value>
void JSONAdapter::SetValue(T_Value value)
{
    m_serialObj = value;
}

template<>
template<typename T_Value>
void JSONAdapter::SetValue(const std::string& name, T_Value value)
{
    m_serialObj[name] = value;
}

template<>
template<typename T_Value>
void JSONAdapter::push_back(T_Value value)
{
    if (!m_serialObj.is_array())
    {
        if (m_serialObj.is_null())
        {
            m_serialObj = njson::json::array();
        }
        else
        {
            const auto tmp = m_serialObj.get<T_Value>();
            m_serialObj = njson::json::array();
            m_serialObj.push_back(tmp);
        }
    }

    m_serialObj.push_back(value);
}

template<>
template<typename T_Value>
void JSONAdapter::AppendValue(const std::string& name, T_Value value)
{
    if (!m_serialObj[name].is_array())
    {
        if (m_serialObj[name].is_null())
        {
            m_serialObj[name] = njson::json::array();
        }
        else
        {
            const auto tmp = m_serialObj[name].get<T_Value>();
            m_serialObj[name] = njson::json::array();
            m_serialObj[name].push_back(tmp);
        }
    }

    m_serialObj[name].push_back(value);
}

template<>
inline std::string JSONAdapter::ToString() const
{
    return m_serialObj.dump();
}

template<>
inline bool JSONAdapter::IsArray() const
{
    return m_serialObj.is_array();
}

template<>
inline bool JSONAdapter::IsEmpty() const
{
    return m_serialObj.is_null() || (m_serialObj.is_array() && m_serialObj.empty());
}

template<>
template<>
inline njson::json::const_iterator JSONAdapter::begin() const
{
    return m_serialObj.begin();
}

template<>
template<>
inline njson::json::const_iterator JSONAdapter::end() const
{
    return m_serialObj.end();
}

template<>
inline size_t JSONAdapter::size() const
{
    return m_serialObj.size();
}

template<>
inline njson::json JSONAdapter::operator[](size_t n) const
{
    return m_serialObj[n];
}

template<>
inline njson::json JSONAdapter::EmptyArray()
{
    return njson::json::array();
}
