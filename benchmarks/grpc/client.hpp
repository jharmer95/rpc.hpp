#pragma once

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "benchmark.grpc.pb.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using grpc_benchmark::RpcBenchmark;

struct ComplexObject;

class gRPC_Client
{
public:
    gRPC_Client() { Start(); }

    void Start()
    {
        if (!m_stub)
        {
            m_stub = RpcBenchmark::NewStub(
                grpc::CreateChannel("127.0.0.1:5200", grpc::InsecureChannelCredentials()));
        }
    }

    void Stop() { m_stub.reset(); }

    double StdDev(double d1, double d2, double d3, double d4, double d5, double d6, double d7,
        double d8, double d9, double d10) const;

    std::vector<uint64_t> GenRandInts(uint64_t min, uint64_t max, size_t sz) const;
    uint64_t Fibonacci(uint64_t n) const;
    std::string HashComplex(const ::ComplexObject& cx) const;
    double AverageContainer_double(const std::vector<double>& vec) const;
    double AverageContainer_uint64(const std::vector<uint64_t>& vec) const;
    void KillServer();

private:
    std::unique_ptr<RpcBenchmark::Stub> m_stub{ nullptr };
};
