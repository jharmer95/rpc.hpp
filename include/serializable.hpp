#pragma once

#include <type_traits>

namespace rpc
{
template <typename, typename T>
struct is_serializable
{
    static_assert(std::integral_constant<T, false>::value, "Second template parameter needs to be of function type");
};

template <typename C, typename R, typename... Args>
struct is_serializable<C, R(Args...)>
{
private:
    template <typename T>
    static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().Serialize(std::declval<Args>()...)), R>::type;

    template <typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

// template <typename T, typename FMT>
// struct is_serializable
// {
//     template <typename U, FMT (U::*)() const>
//     struct SFINAE {};

//     template <typename U>
//     static constexpr char Test(SFINAE<U, &U::Serialize>*);

//     template <typename U>
//     static constexpr long long Test(...);

//     static constexpr bool value = sizeof(Test<T>(0)) == sizeof(char);
// };

// template <typename T, typename FMT>
// struct is_serializable
// {
// private:
//     template <typename C>
//     static std::true_type test(decltype(&C::template Serialize<FMT>));

//     template <typename C>
//     static std::false_type test(...);

// public:
//     static constexpr bool value = decltype(test<T>(FMT{}))::value;
// };

// template <typename FMT>
// struct is_serializable_test
// {
//     template <typename U>
//     static auto test(U* p) -> decltype(p->Serialize(FMT{}), std::true_type());

//     template <typename U>
//     static auto test(U& p) -> decltype(p->Serialize(FMT{}), std::true_type());

//     template <typename>
//     static auto test(...) -> std::false_type;
// };

// template <typename T, typename FMT>
// struct is_serializable : decltype(is_serializable_test::test<T, FMT>(0)) {};

// template <typename T, typename FMT>
// struct is_serializable
// {
// public:
//     static constexpr bool value = std::is_same_v<decltype(testSerialize<T, FMT>(0)), yes> && std::is_same_v<decltype(testDeserialize<T, FMT>(0)), yes>;

// private:
//     using yes = std::true_type;
//     using no = std::false_type;

//     template <typename U> static auto testSerialize(int) -> decltpye(std::declval<U>().Serialize<FMT>() == 1, yes());
//     template <typename U> static auto testDeSerialize(int) -> decltpye(std::declval<U>().DeSerialize<FMT>() == 1, yes());

//     template<typename> static no testSerialize(...);
//     template<typename> static no testDeSerialize(...);
// };

// template <typename T, typename R, typename... Args>
// inline constexpr bool is_serializable_v = is_serializable<T, R(Args...)>::value;
} // namespace rpc
