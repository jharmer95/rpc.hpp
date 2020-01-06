///@file dispatcher.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Example dispatcher.hpp file for use with the example main.cpp file
///@version 0.1.0.0
///@date 01-06-2020
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

#include <nlohmann/json/json.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct TestMessage
{
    bool Flag1{};
    bool Flag2{};
    int ID{};
    int Data[256]{};
    uint8_t DataSize{};

    bool operator==(const TestMessage& other)
    {
        if (Flag1 != other.Flag1 || Flag2 != other.Flag2 || ID != other.ID || DataSize != other.DataSize)
        {
            return false;
        }

        return memcmp(Data, other.Data, DataSize) == 0;
    }
};

int ReadMessages(TestMessage* mesgBuf, int* numMesgs);
int WriteMessages(TestMessage* mesgBuf, int* numMesgs);

class Dispatcher
{
public:
    std::string Run(const std::string& funcName, const nlohmann::json& obj_j);

    template<typename T>
    static nlohmann::json Serialize(const T& obj)
    {
        throw std::logic_error("Type has not been provided with a Serialize method!");
    }

    template<typename T>
    static T DeSerialize(const nlohmann::json& obj_j)
    {
        throw std::logic_error("Type has not been provided with a DeSerialize method!");
    }

    template<>
    static nlohmann::json Serialize<TestMessage>(const TestMessage& mesg)
    {
        nlohmann::json obj_j;
        obj_j["ID"] = mesg.ID;
        obj_j["Flag1"] = mesg.Flag1;
        obj_j["Flag2"] = mesg.Flag2;
        obj_j["DataSize"] = mesg.DataSize;

        if (mesg.DataSize > 0)
		{
			std::vector<int> tmpVec(mesg.Data, mesg.Data + mesg.DataSize);
			obj_j["Data"] = tmpVec;
		}
		else
		{
			obj_j["Data"] = nlohmann::json::array();
		}

        return obj_j;
    }

    template<>
    static TestMessage DeSerialize<TestMessage>(const nlohmann::json& obj_j)
    {
        TestMessage mesg;
        mesg.ID = obj_j["ID"].get<int>();
        mesg.Flag1 = obj_j["Flag1"].get<bool>();
        mesg.Flag2 = obj_j["Flag2"].get<bool>();
        mesg.DataSize = obj_j["DataSize"].get<uint8_t>();
        const auto sData = obj_j["Data"].get<std::vector<int>>();
		std::copy(sData.begin(), sData.begin() + mesg.DataSize, mesg.Data);
        return mesg;
    }

private:
    const std::function<int(TestMessage*, int*)> m_readMessages = ReadMessages;
    const std::function<int(TestMessage*, int*)> m_writeMessages = WriteMessages;
};

int ReadMessages(TestMessage* mesgBuf, int* numMesgs)
{
    // TODO: Remove read lines (capture non-used lines and write them to bus.txt)
    std::ifstream file_in("bus.txt");

    std::string s;

    for (int i = 0; i < *numMesgs; ++i)
    {
        try
        {
            if (file_in >> s)
            {
                mesgBuf[i] = Dispatcher::DeSerialize<TestMessage>(nlohmann::json::parse(s));
            }
            else
            {
                throw 1;
            }
        }
        catch (...)
        {
            *numMesgs = i;
            return 1;
        }
    }

    return 0;
}

int WriteMessages(TestMessage* mesgBuf, int* numMesgs)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    for (int i = 0; i < *numMesgs; ++i)
    {
        try
        {
            file_out << Dispatcher::Serialize<TestMessage>(mesgBuf[i]).dump() << '\n';
        }
        catch (...)
        {
            *numMesgs = i;
            return 1;
        }
    }

    return 0;
}
