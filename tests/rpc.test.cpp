///@file rpc.test.cpp
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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "rpc.hpp"

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

template <>
nlohmann::json rpc::Serialize<TestMessage>(const TestMessage& mesg)
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

template <>
TestMessage rpc::DeSerialize<TestMessage>(const nlohmann::json& obj_j)
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

int ReadMessages(TestMessage* mesgBuf, int* numMesgs)
{
    std::ifstream file_in("bus.txt");
    std::stringstream ss;

    std::string s;
    int i = 0;

    try
    {
        while (file_in >> s)
        {
            if (i < *numMesgs)
            {
                mesgBuf[i++] = rpc::DeSerialize<TestMessage>(nlohmann::json::parse(s));
            }
            else
            {
                ss << s << '\n';
            }
        }
    }
    catch (...)
    {
        *numMesgs = i;
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessages(TestMessage* mesgBuf, int* numMesgs)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    for (int i = 0; i < *numMesgs; ++i)
    {
        try
        {
            file_out << rpc::Serialize<TestMessage>(mesgBuf[i]).dump() << '\n';
        }
        catch (...)
        {
            *numMesgs = i;
            return 1;
        }
    }

    return 0;
}

std::string rpc::dispatch(const std::string& funcName, const nlohmann::json& obj_j)
{
    if (funcName == "WriteMessages")
    {
        return rpc::RunCallBack(obj_j, WriteMessages);
    }
    else if (funcName == "ReadMessages")
    {
        return rpc::RunCallBack(obj_j, ReadMessages);
    }

    throw std::runtime_error("RPC error: Called function: \"" + funcName + "\" not found!");
}

TEST_CASE("ReadWriteMessages", "[]")
{
    TestMessage mesg1;
    mesg1.ID = 24;
    mesg1.Flag1 = true;
    mesg1.Flag2 = false;
    const std::vector<int> md1 = { 10, 20, 30, 40, 50 };
    std::copy(md1.begin(), md1.end(), mesg1.Data);
    mesg1.DataSize = static_cast<uint8_t>(md1.size());

    TestMessage mesg2;
    mesg2.ID = 61;
    mesg2.Flag1 = true;
    mesg2.Flag2 = true;
    const std::vector<int> md2 = { 33, 44, 55 };
    std::copy(md2.begin(), md2.end(), mesg2.Data);
    mesg2.DataSize = static_cast<uint8_t>(md2.size());

    nlohmann::json send_j;
    send_j["args"] = nlohmann::json::array();
    send_j["function"] = "WriteMessages";

    auto& argList = send_j["args"];

    nlohmann::json mesgList = nlohmann::json::array();
    mesgList.push_back(rpc::Serialize<TestMessage>(mesg1));
    mesgList.push_back(rpc::Serialize<TestMessage>(mesg2));
    argList.push_back(mesgList);
    argList.push_back(2);

    const auto retMsg = rpc::RunFromJSON(send_j);

    const size_t numMesg = 2;
    const auto rdMsg = std::make_unique<TestMessage[]>(numMesg);

    nlohmann::json recv_j;
    recv_j["function"] = "ReadMessages";
    recv_j["args"] = njson::json::array();
    auto& argList2 = recv_j["args"];

    nlohmann::json subArgList = nlohmann::json::array();

    for (size_t i = 0; i < numMesg; ++i)
    {
        subArgList.push_back(rpc::Serialize<TestMessage>(rdMsg[i]));
    }

    argList2.push_back(subArgList);
    argList2.push_back(numMesg);

    const auto retMsg2 = rpc::RunFromJSON(recv_j);
    const auto retData = nlohmann::json::parse(retMsg2)["args"];

    for (size_t i = 0; i < retData.back().get<size_t>(); ++i)
    {
        rdMsg[i] = rpc::DeSerialize<TestMessage>(retData.at(i));
    }

    for (size_t i = 0; i < numMesg; ++i)
    {
        std::cout << rdMsg[i].ID << '\n';
    }

    REQUIRE((rdMsg[0] == mesg1));
    REQUIRE((rdMsg[1] == mesg2));
}
