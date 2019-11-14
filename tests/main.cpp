#include "rpc.hpp"

#include <nlohmann/json/json.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>

struct TestStruct
{
	int age;
	char name[255]{};
	int sector;
	unsigned long userID;

	template<typename FMT>
	static FMT Serialize(const TestStruct&)
	{
	}

	template<>
	static nlohmann::json Serialize(const TestStruct& st)
	{
		nlohmann::json obj_j;
		obj_j["age"] = st.age;
		obj_j["name"] = std::string(st.name, 255);
		obj_j["sector"] = st.sector;
		obj_j["userID"] = st.userID;

		return obj_j;
	}

	template<typename FMT>
	static TestStruct DeSerialize(const FMT&)
	{
	}

	template<>
	static TestStruct DeSerialize(const nlohmann::json& obj_j)
	{
		TestStruct ts;
		ts.age = obj_j["age"].get<int>();
		const auto nmStr = obj_j["name"].get<std::string>();
		std::copy(nmStr.begin(), nmStr.end(), ts.name);
		ts.sector = obj_j["sector"].get<int>();
		ts.userID = obj_j["userID"].get<unsigned long>();

		return ts;
	}
};

int PrintMyArgs(TestStruct* pts, int n, std::string msg)
{
	std::cout << "age: " << pts->age << '\n';
	std::cout << "name: " << pts->name << '\n';
	std::cout << "sector: " << pts->sector << '\n';
	std::cout << "userID: " << pts->userID << "\n\n";
	std::cout << "n: " << n << '\n';
	std::cout << "msg: " << msg << '\n';

	return 2;
}

template <typename... Args>
const std::function<int(Args...)>& DispatchFunction(std::string_view sv)
{
    if (sv == "PrintMyArgs")
    {
        return PrintMyArgs;
    }
    else
    {
        throw std::runtime_error("Could not find function!");
    }
}

int main()
{
	nlohmann::json send_j;

	send_j["function"] = "PrintMyArgs";
	send_j["args"] = nlohmann::json::array();
	auto& argList = send_j["args"];

	TestStruct ts;

	ts.age = 5;
	std::string nmStr = "Frank Tank";
	std::copy(nmStr.begin(), nmStr.end(), ts.name);
	ts.sector = 5545;
	ts.userID = 12345678UL;

	argList.push_back(TestStruct::Serialize<nlohmann::json>(ts));
	argList.push_back(45);
	argList.push_back("Hello world!");

	const auto retMsg = rpc::RunFromJSON(send_j);

	std::cout << "\nReturn message:\n" << retMsg << '\n';

	return 0;
}
