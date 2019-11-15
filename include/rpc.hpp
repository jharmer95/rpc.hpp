#pragma once

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

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
	static constexpr size_t nargs = sizeof...(Args);

	using result_type = R;

	template<size_t i>
	struct arg
	{
		using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
	};
};

template<typename R, typename... Args>
inline constexpr size_t function_param_count_v = function_traits<std::function<R(Args...)>>::nargs;

template<typename R, typename... Args>
using function_result_t = typename function_traits<std::function<R(Args...)>>::type;

template<size_t i, typename R, typename... Args>
using function_args_t = typename function_traits<std::function<R(Args...)>>::template arg<i>::type;

namespace rpc
{
template<typename, typename T>
struct is_serializable
{
	static_assert(std::integral_constant<T, false>::value,
		"Second template parameter needs to be of function type");
};

template<typename C, typename R, typename... Args>
struct is_serializable<C, R(Args...)>
{
private:
	template<typename T>
	static constexpr auto check(T*) ->
		typename std::is_same<decltype(std::declval<T>().Serialize(std::declval<Args>()...)),
			R>::type;

	template<typename>
	static constexpr std::false_type check(...);

	typedef decltype(check<C>(0)) type;

public:
	static constexpr bool value = type::value;
};

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

		if constexpr (is_serializable<P, njson::json(const P&)>::value)
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

	if constexpr (is_serializable<P, njson::json(const P&)>::value)
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

	if constexpr (std::is_pointer_v<T>)
	{
		return DecodeArgArray<T>(obj_j, buf, count);
	}
	else if constexpr (is_serializable<T, njson::json(const T&)>::value)
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
			if constexpr (is_serializable<P, njson::json(const P&)>::value)
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
		if constexpr (is_serializable<T, njson::json(const T&)>::value)
		{
			args_j.push_back(T::Serialize(val));
		}
		else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
		{
			args_j.push_back(val);
		}
	}
}

template<typename F, typename... Ts, size_t... Is>
void for_each_tuple(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>)
{
	using expander = int[];
	(void)expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<typename F, typename... Ts>
void for_each_tuple(const std::tuple<Ts...>& tuple, F func)
{
	for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
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

	std::tuple<Args...> args{ DecodeArg<Args>(
		obj_j[count], buffers[count].second.get(), &(buffers[count].first), &count)... };

	const auto result = std::apply(func, args);

	njson::json retObj_j;

	retObj_j["result"] = result;
	retObj_j["args"] = njson::json::array();
	auto& argList = retObj_j["args"];

	unsigned count2 = 0;

	for_each_tuple(args, [&argList, &buffers, &count2](const auto& x) {
		EncodeArgs(argList, buffers[count2++].first, x);
	});

	return retObj_j.dump();
}

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
		return "{ \"result\": -1 }";
	}
}
} // namespace rpc
