///@file main.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Source file for testing rpc.hpp
///@version 0.1.0.0
///@date 11-18-2019
///
///@copyright Copyright Jackson Harmer (c) 2019
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
    argList.push_back("Hello world!");

    const auto retMsg = rpc::RunFromJSON(send_j);

    std::cout << "\nReturn message:\n" << retMsg << '\n';

    return 0;
}
