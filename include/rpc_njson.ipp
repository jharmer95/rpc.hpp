#pragma once

template<>
template<typename T_Value>
T_Value rpc::SerialAdapter<njson::json>::GetValue(const std::string& name) const
{
    return this->m_serialObj[name].get<T_Value>();
}

template<>
template<typename T_Value>
void rpc::SerialAdapter<njson::json>::SetValue(const std::string& name, T_Value value)
{
    m_serialObj[name] = value;
}

template<>
template<typename T_Value>
void rpc::SerialAdapter<njson::json>::AppendValue(const std::string& name, T_Value value)
{
    if (!m_serialObj[name].is_array())
    {
        if (m_serialObj[name].is_null())
        {
            m_serialObj[name] = njson::json::array();
        }
        else
        {
            const auto tmp = m_serialObj[name];
            m_serialObj[name] = njson::json::array();
            m_serialObj[name].push_back(tmp);
        }
    }

    m_serialObj[name].push_back(value);
}

template<>
inline std::string rpc::SerialAdapter<njson::json>::ToString() const
{
    return m_serialObj.dump();
}
