///@file main.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Example source file to show rpc.hpp
///@version 0.1.0.0
///@date 11-18-2019
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2019, Jackson Harmer
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

#include "rpc.hpp"

#include <algorithm>

std::string Dispatcher::Run(const std::string& funcName, const nlohmann::json& obj_j)
{
    if (funcName == "PrintMyArgs")
    {
        return rpc::RunCallBack(obj_j, m_printArgs);
    }
    else if (funcName == "TestMyArgs")
    {
        return rpc::RunCallBack(obj_j, m_testArgs);
    }
}

int main()
{
    nlohmann::json send_j;

    send_j["args"] = nlohmann::json::array();
    send_j["function"] = "PrintMyArgs";
    auto& argList = send_j["args"];

    TestStruct ts;

    ts.age = 5;
    std::string nmStr = "Frank Tank";
    std::copy(nmStr.begin(), nmStr.end(), ts.name);
    ts.sector = 5545;
    ts.userID = 12345678UL;

    argList.push_back(TestStruct::Serialize(ts));
    argList.push_back(45);

    std::vector<std::vector<std::string>> vec = { { "Hello world!", "Goodbye everybody!" }, { "Testing1", "Testing2" } };
    auto subArgList = nlohmann::json::array();

    for (const auto& svec : vec)
    {
        auto subSubArgList = nlohmann::json::array();

        for (const auto& str : svec)
        {
            subSubArgList.push_back(str);
        }

        subArgList.push_back(subSubArgList);
    }

    argList.push_back(subArgList);

    const auto retMsg = rpc::RunFromJSON(send_j);

    std::cout << "\nReturn message:\n" << retMsg << '\n';

    return 0;
}
