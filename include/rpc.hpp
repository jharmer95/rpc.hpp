#pragma once

#include "function_traits.hpp"
#include "serializable.hpp"
#include "safe_strcpy.hpp"

#include <nlohmann/json/json.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

namespace njson = nlohmann;

namespace rpc
{
template<typename T>
T DecodeArgArray(const njson::json& obj_j, uint8_t* buf, size_t* count)
{
	*count = 1UL;

	if (obj_j.is_array())
	{
		// Multi-value pointer (array)

		// TODO: Also support containers

		using P = std::remove_pointer_t<T>;
		static_assert(!std::is_void_v<P>,
			"Void pointers are not supported, either cast to a different type or do the conversion "
			"manually!");
		T bufPtr = reinterpret_cast<T>(buf);

		if constexpr (rpc::is_serializable<P, njson::json(const P&)>::value)
		{
			const auto values = P::DeSerialize(obj_j);

			if (values.size() != obj_j.size())
			{
				throw std::runtime_error(
					"JSON object was not deserialized correctly: number of arguments changed");
			}

			std::copy(values.begin(), values.end(), bufPtr);
		}
		else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
		{
			for (size_t i = 0; i < obj_j.size(); ++i)
			{
				bufPtr[i] = obj_j[i].get<P>();
			}
		}

		*count = obj_j.size();
		return bufPtr;
	}

	// Single value pointer
	using P = std::remove_pointer_t<T>;
	static_assert(!std::is_void_v<P>,
		"Void pointers are not supported, either cast to a different type or do the conversion "
		"manually!");

	const T bufPtr = reinterpret_cast<T>(buf);

	if constexpr (rpc::is_serializable<P, njson::json(const P&)>::value)
	{
		const auto value = P::DeSerialize(obj_j)[0];
		*bufPtr = value;
	}
	else if constexpr (std::is_same_v<P, char>)
	{
		const auto str = obj_j.get<std::string>();
		std::copy(str.begin(), str.end(), bufPtr);
	}
	else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
	{
		const auto value = obj_j.get<P>();
		*bufPtr = value;
	}

	return bufPtr;
}

template<typename T>
T DecodeArg(const njson::json& obj_j, uint8_t* buf, size_t* count, unsigned* paramNum)
{
	*paramNum += 1;
	*count = 1UL;

	const std::string dbg = obj_j.dump();

	constexpr bool isSerial = rpc::is_serializable<T, njson::json(const T&)>::value;

	if constexpr (std::is_pointer_v<T>)
	{
		return DecodeArgArray<T>(obj_j, buf, count);
	}
	else if constexpr (rpc::is_serializable<T, njson::json(const T&)>::value)
	{
		return T::DeSerialize(obj_j)[0];
	}
	else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
	{
		T retVal = obj_j.get<T>();
		return retVal;
	}
	else
	{
		throw std::runtime_error("Invalid type!");
	}
}

template<typename T>
void EncodeArgs(njson::json& args_j, const size_t count, const T& val)
{
	if constexpr (std::is_pointer_v<T>)
	{
		using P = std::remove_pointer_t<T>;

		for (size_t i = 0; i < count; ++i)
		{
			if constexpr (rpc::is_serializable<P, njson::json(const P&)>::value)
			{
				args_j.push_back(P::Serialize(val[i]));
			}
			else if constexpr (std::is_same_v<P, char>)
			{
				args_j.push_back(std::string(val, count));
			}
			else if constexpr (std::is_arithmetic_v<P> || std::is_same_v<P, std::string>)
			{
				args_j.push_back(val[i]);
			}
		}
	}
	else
	{
		if constexpr (rpc::is_serializable<T, njson::json(const T&)>::value)
		{
			args_j.push_back(T::Serialize(val));
		}
		else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
		{
			args_j.push_back(val);
		}
	}
}

template<typename R, typename... Args>
std::string RunCallBack(const njson::json& obj_j, std::function<R(Args...)> func)
{
	unsigned count = 0;

	std::array<std::pair<size_t, std::unique_ptr<unsigned char[]>>,
		function_param_count_v<R, Args...>>
		buffers;

	for (auto& buf : buffers)
	{
		buf.first = 0UL;
		buf.second = std::make_unique<unsigned char[]>(4096);
	}

	const std::string dbg = obj_j[count + 1].dump();

	std::tuple<Args...> args {
		DecodeArg<Args>(obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)...};

	const auto result = std::apply(func, args);

	njson::json retObj_j;

	retObj_j["result"] = result;
	retObj_j["args"] = njson::json::array();
	auto& argList = retObj_j["args"];

	// Ugly, I know, but at this time, there is really no other way to "iterate" through a tuple as it is compile-time only
	// At least we have 'if constexpr' now :D
	for (size_t i = 0; i < function_param_count_v<R, Args...>; ++i)
	{
		if constexpr (function_param_count_v<R, Args...>> 0)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;
			}
		}
		else if constexpr (function_param_count_v<R, Args...>> 1)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;

				case 1:
					EncodeArgs(argList, buffers[1].first, std::get<1>(args));
					break;
			}
		}
		else if constexpr (function_param_count_v<R, Args...>> 2)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;

				case 1:
					EncodeArgs(argList, buffers[1].first, std::get<1>(args));
					break;

				case 2:
					EncodeArgs(argList, buffers[2].first, std::get<2>(args));
					break;
			}
		}
		else if constexpr (function_param_count_v<R, Args...>> 3)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;

				case 1:
					EncodeArgs(argList, buffers[1].first, std::get<1>(args));
					break;

				case 2:
					EncodeArgs(argList, buffers[2].first, std::get<2>(args));
					break;

				case 3:
					EncodeArgs(argList, buffers[3].first, std::get<3>(args));
					break;
			}
		}
		else if constexpr (function_param_count_v<R, Args...>> 4)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;

				case 1:
					EncodeArgs(argList, buffers[1].first, std::get<1>(args));
					break;

				case 2:
					EncodeArgs(argList, buffers[2].first, std::get<2>(args));
					break;

				case 3:
					EncodeArgs(argList, buffers[3].first, std::get<3>(args));
					break;

				case 4:
					EncodeArgs(argList, buffers[4].first, std::get<4>(args));
					break;
			}
		}
		else if constexpr (function_param_count_v<R, Args...>> 5)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;

				case 1:
					EncodeArgs(argList, buffers[1].first, std::get<1>(args));
					break;

				case 2:
					EncodeArgs(argList, buffers[2].first, std::get<2>(args));
					break;

				case 3:
					EncodeArgs(argList, buffers[3].first, std::get<3>(args));
					break;

				case 4:
					EncodeArgs(argList, buffers[4].first, std::get<4>(args));
					break;

				case 5:
					EncodeArgs(argList, buffers[5].first, std::get<5>(args));
					break;
			}
		}
		else if constexpr (function_param_count_v<R, Args...>> 6)
		{
			switch (i)
			{
				case 0:
					EncodeArgs(argList, buffers[0].first, std::get<0>(args));
					break;

				case 1:
					EncodeArgs(argList, buffers[1].first, std::get<1>(args));
					break;

				case 2:
					EncodeArgs(argList, buffers[2].first, std::get<2>(args));
					break;

				case 3:
					EncodeArgs(argList, buffers[3].first, std::get<3>(args));
					break;

				case 4:
					EncodeArgs(argList, buffers[4].first, std::get<4>(args));
					break;

				case 5:
					EncodeArgs(argList, buffers[5].first, std::get<5>(args));
					break;

				case 6:
					EncodeArgs(argList, buffers[6].first, std::get<6>(args));
					break;
			}
		}
	}

	return retObj_j.dump();
}

// TODO: Need to get dispatch working
template<typename TDispatcher>
inline std::string RunFromJSON(const njson::json& obj_j, const TDispatcher& dispatch)
{
	const auto funcName = obj_j["function"].get<std::string>();
	const auto& argList = obj_j["args"];

	try
	{
		return RunCallBack(argList, dispatch.get(funcName));
	}
	catch (std::exception& ex)
	{
		std::cerr << ex.what() << '\n';
	}
}
} // namespace rpc
