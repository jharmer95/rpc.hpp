///@file rpc.test.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Unit test source file for rpc.hpp
///@version 0.1.0.0
///@date 06-05-2020
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
#include "adapters/rpc_njson.hpp"
#include "rpc_dispatch_helper.hpp"

#include <fstream>
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

    [[nodiscard]] bool operator==(const TestMessage& other) const noexcept
    {
        if (Flag1 != other.Flag1 || Flag2 != other.Flag2 || ID != other.ID
            || DataSize != other.DataSize)
        {
            return false;
        }

        return memcmp(Data, other.Data, DataSize) == 0;
    }
};

template<>
inline njson rpc::serialize(const TestMessage& mesg)
{
    njson obj_j;
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
        obj_j["Data"] = njson::array();
    }

    return obj_j;
}

template<>
inline TestMessage rpc::deserialize(const njson& obj_j)
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
                mesgBuf[i++] = rpc::deserialize<njson, TestMessage>(njson::parse(s));
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
            file_out << rpc::serialize<njson, TestMessage>(mesgBuf[i]).dump() << '\n';
        }
        catch (...)
        {
            *numMesgs = i;
            return 1;
        }
    }

    return 0;
}

int ReadMessageRef(TestMessage& mesg)
{
    std::ifstream file_in("bus.txt");
    std::stringstream ss;

    std::string s;

    try
    {
        if (file_in >> s)
        {
            mesg = rpc::deserialize<njson, TestMessage>(njson::parse(s));
        }
        while (file_in >> s)
        {
            ss << s << '\n';
        }
    }
    catch (...)
    {
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessageRef(const TestMessage& mesg)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    try
    {
        file_out << rpc::serialize<njson, TestMessage>(mesg).dump() << '\n';
    }
    catch (...)
    {
        return 1;
    }

    return 0;
}

int ReadMessageVec(std::vector<TestMessage>& vec, int& numMesgs)
{
    std::ifstream file_in("bus.txt");
    std::stringstream ss;

    std::string s;
    int i = 0;

    try
    {
        while (file_in >> s)
        {
            if (i < numMesgs)
            {
                vec.push_back(rpc::deserialize<njson, TestMessage>(njson::parse(s)));
            }
            else
            {
                ss << s << '\n';
            }
        }
    }
    catch (...)
    {
        numMesgs = i;
        return 1;
    }

    file_in.close();

    std::ofstream file_out("bus.txt");

    file_out << ss.str();
    return 0;
}

int WriteMessageVec(const std::vector<TestMessage>& vec)
{
    std::ofstream file_out("bus.txt", std::fstream::out | std::fstream::app);

    for (const auto& mesg : vec)
    {
        try
        {
            file_out << rpc::serialize<njson, TestMessage>(mesg).dump() << '\n';
        }
        catch (...)
        {
            return 1;
        }
    }

    return 0;
}

int SimpleSum(const int n1, const int n2)
{
    return n1 + n2;
}

RPC_DEFAULT_DISPATCH(WriteMessages, WriteMessageRef, WriteMessageVec, ReadMessages, ReadMessageRef,
    ReadMessageVec, SimpleSum)

void ClearBus()
{
    std::ofstream("bus.txt");
}

TEST_CASE("References", "[]")
{
    ClearBus();
    TestMessage mesg;
    mesg.ID = 16;
    mesg.Flag1 = true;
    mesg.Flag2 = false;
    const std::vector<int> md = { 10, 20, 30 };
    std::copy(md.begin(), md.end(), mesg.Data);
    mesg.DataSize = static_cast<uint8_t>(md.size());

    const auto ret_obj1 = rpc::run<njson>("WriteMessageRef", mesg);

    REQUIRE(ret_obj1);

    TestMessage rmesg;

    const auto ret_obj2 = rpc::run<njson>("ReadMessageRef", rmesg);

    REQUIRE(ret_obj2);

    rmesg = rpc::deserialize<njson, TestMessage>(ret_obj2.get_arg(0));
    REQUIRE((mesg == rmesg));
}

TEST_CASE("Vectors", "[]")
{
    ClearBus();
    TestMessage mesg1;
    mesg1.ID = 16;
    mesg1.Flag1 = true;
    mesg1.Flag2 = false;
    const std::vector<int> md1 = { 10, 20, 30 };
    std::copy(md1.begin(), md1.end(), mesg1.Data);
    mesg1.DataSize = static_cast<uint8_t>(md1.size());

    TestMessage mesg2;
    mesg2.ID = 16;
    mesg2.Flag1 = true;
    mesg2.Flag2 = false;
    const std::vector<int> md2 = { 10, 20, 30 };
    std::copy(md2.begin(), md2.end(), mesg2.Data);
    mesg2.DataSize = static_cast<uint8_t>(md2.size());

    TestMessage mesg3;
    mesg3.ID = 16;
    mesg3.Flag1 = true;
    mesg3.Flag2 = false;
    const std::vector<int> md3 = { 10, 20, 30 };
    std::copy(md3.begin(), md3.end(), mesg3.Data);
    mesg3.DataSize = static_cast<uint8_t>(md3.size());

    std::vector<TestMessage> mesgVec{ mesg1, mesg2, mesg3 };

    const auto rec_obj1 = rpc::run<njson>("WriteMessageVec", mesgVec);

    REQUIRE(rec_obj1);

    std::vector<TestMessage> rmesgVec;

    const auto rec_obj2 = rpc::run<njson>("ReadMessageVec", rmesgVec, 3);

    REQUIRE(rec_obj2);

    for (const auto& val : rec_obj2.get_arg(0))
    {
        rmesgVec.push_back(rpc::deserialize<njson, TestMessage>(val));
    }

    REQUIRE(rmesgVec.size() == mesgVec.size());
    REQUIRE((rmesgVec.front() == mesgVec.front()));
}

TEST_CASE("Pointers", "[]")
{
    ClearBus();
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

    TestMessage mesgs[2]{ mesg1, mesg2 };
    int count = 2;

    const auto rec_obj1 = rpc::run<njson>("WriteMessages", mesgs, &count);

    REQUIRE(rec_obj1);

    TestMessage rdMsg[2];

    const auto rec_obj2 = rpc::run<njson>("ReadMessages", rdMsg, 2);

    REQUIRE(rec_obj2);

    for (size_t i = 0; i < rec_obj2.get_arg_count() - 1; ++i)
    {
        rdMsg[i] = rpc::deserialize<njson, TestMessage>(rec_obj2.get_arg(i));
    }

    REQUIRE((rdMsg[0] == mesg1));
    REQUIRE((rdMsg[1] == mesg2));
}

TEST_CASE("From String")
{
    const auto rec_obj = rpc::run_string<njson>(R"({ "function": "SimpleSum", "args": [3, 4] })");

    REQUIRE(rec_obj);

    const auto result = rec_obj.template get_result<int>();

    REQUIRE(result == 7);
}

TEST_CASE("async")
{
    auto rec_obj1 = rpc::async_run<njson>("SimpleSum", 5, 9);
    auto rec_obj2 = rpc::async_run<njson>("SimpleSum", 7, 19);
    auto rec_obj3 = rpc::async_run<njson>("SimpleSum", 1, 21);
    auto rec_obj4 = rpc::async_run<njson>("SimpleSum", 2, 2);

    const int val1 = rec_obj1.get().template get_result<int>();
    const int val2 = rec_obj2.get().template get_result<int>();
    const int val3 = rec_obj3.get().template get_result<int>();
    const int val4 = rec_obj4.get().template get_result<int>();

    REQUIRE(val1 == 14);
    REQUIRE(val2 == 26);
    REQUIRE(val3 == 22);
    REQUIRE(val4 == 4);
}
