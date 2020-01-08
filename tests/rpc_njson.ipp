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
