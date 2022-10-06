#include <rpc/server.h>

#include "../bench_funcs.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <random>
#include <sstream>

namespace
{
std::unique_ptr<rpc::server> P_SERVER;

void KillServer()
{
    P_SERVER->stop();
}
} //namespace

int main()
{
    P_SERVER = std::make_unique<rpc::server>(5100);
    P_SERVER->bind("StdDev", &::StdDev);
    P_SERVER->bind("GenRandInts", &::GenRandInts);
    P_SERVER->bind("Fibonacci", &::Fibonacci);
    P_SERVER->bind("HashComplex", &::HashComplex);
    P_SERVER->bind("AverageContainer<double>", &::AverageContainer<double>);
    P_SERVER->bind("AverageContainer<uint64_t>", &::AverageContainer<uint64_t>);
    P_SERVER->bind("KillServer", &KillServer);
    P_SERVER->run();

    return EXIT_SUCCESS;
}
