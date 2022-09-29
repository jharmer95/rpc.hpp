#include "funcs.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <random>
#include <sstream>

std::unique_ptr<rpc::server> P_SERVER;

double AverageContainer(const std::vector<uint64_t>& vec);
double AverageContainer(const std::vector<double>& vec);

std::vector<uint64_t> GenRandInts(const uint64_t min, const uint64_t max, const size_t num_ints)
{
    static std::mt19937_64 mt_gen{ static_cast<uint_fast64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count()) };

    std::uniform_int_distribution<uint64_t> distribution{ min, max };
    std::vector<uint64_t> vec(num_ints);
    std::generate(begin(vec), end(vec), [&distribution] { return distribution(mt_gen); });
    return vec;
}

std::string HashComplex(const ComplexObject& cx_obj)
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

constexpr double Average(const double n1, const double n2, const double n3, const double n4,
    const double n5, const double n6, const double n7, const double n8, const double n9,
    const double n10)
{
    return (n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + n10) / 10.00;
}

double StdDev(const double n1, const double n2, const double n3, const double n4, const double n5,
    const double n6, const double n7, const double n8, const double n9, const double n10)
{
    const auto avg = Average(
        n1 * n1, n2 * n2, n3 * n3, n4 * n4, n5 * n5, n6 * n6, n7 * n7, n8 * n8, n9 * n9, n10 * n10);

    return std::sqrt(avg);
}

void KillServer()
{
    P_SERVER->stop();
}

int main()
{
    P_SERVER = std::make_unique<rpc::server>(5100);
    P_SERVER->bind("StdDev", &StdDev);
    P_SERVER->bind("GenRandInts", &GenRandInts);
    P_SERVER->bind("Fibonacci", &Fibonacci);
    P_SERVER->bind("HashComplex", &HashComplex);
    P_SERVER->bind("AverageContainer<double>", &AverageContainer<double>);
    P_SERVER->bind("AverageContainer<uint64_t>", &AverageContainer<uint64_t>);
    P_SERVER->bind("KillServer", &KillServer);
    P_SERVER->run();

    return EXIT_SUCCESS;
}
