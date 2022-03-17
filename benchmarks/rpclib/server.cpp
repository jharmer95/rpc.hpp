#include "funcs.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>

std::unique_ptr<rpc::server> P_SERVER;

double AverageContainer(const std::vector<uint64_t>& vec);
double AverageContainer(const std::vector<double>& vec);

std::vector<uint64_t> GenRandInts(const uint64_t min, const uint64_t max, const size_t sz)
{
    std::vector<uint64_t> vec;
    vec.reserve(sz);

    for (size_t i = 0; i < sz; ++i)
    {
        vec.push_back(static_cast<uint64_t>(std::rand()) % (max - min + 1) + min);
    }

    return vec;
}

std::string HashComplex(const ComplexObject& cx)
{
    std::stringstream hash;
    auto values = cx.vals;

    if (cx.flag1)
    {
        std::reverse(values.begin(), values.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        const int acc = cx.flag2 ? cx.name[i] + values[i % 12] : cx.name[i] - values[i % 12];
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
