#include "funcs.hpp"

#include <algorithm>
#include <sstream>

static rpc::server& get_server()
{
    static rpc::server srv(5100);
    return srv;
}

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
    get_server().stop();
}

int main()
{
    auto& srv = get_server();
    srv.bind("StdDev", &StdDev);
    srv.bind("GenRandInts", &GenRandInts);
    srv.bind("Fibonacci", &Fibonacci);
    srv.bind("HashComplex", &HashComplex);
    srv.bind("AverageContainer<double>", &AverageContainer<double>);
    srv.bind("AverageContainer<uint64_t>", &AverageContainer<uint64_t>);
    srv.bind("KillServer", &KillServer);
    srv.run();

    return EXIT_SUCCESS;
}
