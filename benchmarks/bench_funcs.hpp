#pragma once

#include "bench_structs.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

constexpr double Average(const double n1, const double n2, const double n3, const double n4,
    const double n5, const double n6, const double n7, const double n8, const double n9,
    const double n10)
{
    return (n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10) / 10.00;
}

inline double StdDev(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10)
{
    const auto avg = ::Average(
        n1 * n1, n2 * n2, n3 * n3, n4 * n4, n5 * n5, n6 * n6, n7 * n7, n8 * n8, n9 * n9, n10 * n10);

    return std::sqrt(avg);
}

inline uint64_t Fibonacci(const uint64_t number)
{
    uint64_t num1{ 0 };
    uint64_t num2{ 1 };

    if (number == 0)
    {
        return 0;
    }

    for (uint64_t i = 2; i <= number; ++i)
    {
        const uint64_t next{ num1 + num2 };
        num1 = num2;
        num2 = next;
    }

    return num2;
}

inline std::vector<uint64_t> GenRandInts(
    const uint64_t min, const uint64_t max, const size_t num_ints)
{
    static std::mt19937_64 mt_gen{ static_cast<uint_fast64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count()) };

    std::uniform_int_distribution<uint64_t> distribution{ min, max };
    std::vector<uint64_t> vec(num_ints);
    std::generate(begin(vec), end(vec), [&distribution] { return distribution(mt_gen); });
    return vec;
}

inline std::string HashComplex(const ComplexObject& cx_obj)
{
    std::stringstream hash;
    auto rev_vals = cx_obj.vals;

    if (cx_obj.flag1)
    {
        std::reverse(rev_vals.begin(), rev_vals.end());
    }

    const auto name_sz = cx_obj.name.size();

    for (size_t i = 0; i < name_sz; ++i)
    {
        const size_t wrap_idx = i % rev_vals.size();
        const int acc = (cx_obj.flag2) ? (cx_obj.name[i] + rev_vals[wrap_idx])
                                       : (cx_obj.name[i] - rev_vals[wrap_idx]);

        hash << std::hex << acc;
    }

    return hash.str();
}

template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}

double AverageContainer(const std::vector<uint64_t>& vec);
double AverageContainer(const std::vector<double>& vec);
