#pragma once

#include <type_traits>

namespace rpc
{
template <typename T, typename FMT>
struct is_serializable
{
public:
    static constexpr bool value = decltype(test<T>(FMT{}));

private:
    template <typename C>
    static std::true_type test(decltype(&C::template Serialize<FMT>));

    template <typename C>
    static std::false_type test(...);
};

// template <typename FMT>
// struct is_serializable_test
// {
//     template <typename U>
//     static auto test(U* p) -> decltype(p->Serialize(FMT{}), std::true_type());

//     template <typename U>
//     static auto test(U* p) -> decltype(p->Serialize(FMT{}), std::true_type());

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

template <typename T, typename FMT>
inline constexpr bool is_serializable_v = is_serializable<T, FMT>::value;
} // namespace rpc
