#pragma once

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "benchmark.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using grpc_benchmark::RpcBenchmark;

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
};

double StdDev(double n1, double n2, double n3, double n4, double n5, double n6, double n7,
    double n8, double n9, double n10);

constexpr uint64_t Fibonacci(const uint64_t number)
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

std::vector<uint64_t> GenRandInts(uint64_t min, uint64_t max, size_t sz);
std::string HashComplex(const ComplexObject& cx);

template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}
