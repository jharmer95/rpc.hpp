///@file main.cpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Source file for testing rpc.hpp
///@version 0.1.0.0
///@date 11-15-2019
///
///@copyright Copyright Jackson Harmer (c) 2019
///

#include "rpc.hpp"

#include <nlohmann/json/json.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct TestStruct
{
	int age;
	char name[255]{};
	int sector;
	unsigned long userID;

	// TODO: Get templated Serialize/Deserialize working
	static nlohmann::json Serialize(const TestStruct& st)
	{
		nlohmann::json obj_j;
		obj_j["age"] = st.age;
		obj_j["name"] = std::string(st.name);
		obj_j["sector"] = st.sector;
		obj_j["userID"] = st.userID;

		return obj_j;
	}

	// TODO: move array stuff to the library side so that return type is just TestStruct
	static std::vector<TestStruct> DeSerialize(const nlohmann::json& obj_j)
	{
		std::vector<TestStruct> vec;
		const size_t sz = obj_j.is_array() ? obj_j.size() : 1;

		for (size_t i = 0; i < sz; ++i)
		{
			TestStruct ts;
			ts.age = obj_j["age"].get<int>();
			const auto nmStr = obj_j["name"].get<std::string>();
			std::copy(nmStr.begin(), nmStr.end(), ts.name);
			ts.sector = obj_j["sector"].get<int>();
			ts.userID = obj_j["userID"].get<unsigned long>();
			vec.push_back(ts);
		}

		return vec;
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

class Dispatcher
{
public:
	auto get(const std::string& sv) const
	{
		if (sv == "PrintMyArgs")
		{
			return m_printArgs;
		}

		throw std::runtime_error("Could not find function!");
	}

private:
	std::function<int(TestStruct*, int n, std::string)> m_printArgs = PrintMyArgs;
} myDispatch;

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

	const auto retMsg = rpc::RunFromJSON(send_j, myDispatch);

	std::cout << "\nReturn message:\n" << retMsg << '\n';

	return 0;
}
