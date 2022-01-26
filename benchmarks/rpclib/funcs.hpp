#pragma once

#include <rpc/server.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

struct ComplexObject
{
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};
    MSGPACK_DEFINE_ARRAY(id, name, flag1, flag2, vals);
};

double StdDev(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10);

constexpr uint64_t Fibonacci(const uint64_t number)
{
    return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}

std::vector<uint64_t> GenRandInts(uint64_t min, uint64_t max, size_t sz);
std::string HashComplex(const ComplexObject& cx);

template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}
