#pragma once

#include <functional>

template <typename T>
struct function_traits;

template <typename R, typename... Args>
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

template <typename R, typename... Args>
inline constexpr size_t function_param_count_v = function_traits<std::function<R(Args...)>>::nargs;

template <typename R, typename... Args>
using function_result_t = typename function_traits<std::function<R(Args...)>>::type;

template <size_t i, typename R, typename... Args>
using function_args_t = typename function_traits<std::function<R(Args...)>>::template arg<i>::type;
