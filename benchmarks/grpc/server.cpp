#include "server.hpp"

#include <algorithm>
#include <chrono>
#include <future>
#include <random>
#include <sstream>
#include <thread>

namespace
{
std::unique_ptr<Server> P_SERVER;

void KillServer()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //P_SERVER->Shutdown();
    //P_SERVER->Wait();
    std::exit(0);
}

class RpcBenchmarkImpl final : public RpcBenchmark::Service
{
public:
    Status StdDev(ServerContext* context, const grpc_benchmark::TenDoubles* request,
        grpc_benchmark::Double* response) override
    {
        const auto result =
            ::StdDev(request->d1(), request->d2(), request->d3(), request->d4(), request->d5(),
                request->d6(), request->d7(), request->d8(), request->d9(), request->d10());

        response->set_val(result);
        return Status::OK;
    }

    Status GenRandInts(ServerContext* context, const grpc_benchmark::GenRandInts_Msg* request,
        grpc_benchmark::Vec_Uint64* response) override
    {
        const auto vec = ::GenRandInts(request->min(), request->max(), request->sz());
        response->mutable_val()->Add(std::begin(vec), std::end(vec));
        return Status::OK;
    }

    Status Fibonacci(ServerContext* context, const grpc_benchmark::Uint64* request,
        grpc_benchmark::Uint64* response) override
    {
        const auto result = ::Fibonacci(request->val());
        response->set_val(result);
        return Status::OK;
    }

    Status HashComplex(ServerContext* context, const grpc_benchmark::ComplexObject* request,
        grpc_benchmark::String* response) override
    {
        ::ComplexObject cx{ request->id(), request->name(), request->flag1(), request->flag2(),
            {} };

        std::copy_n(std::begin(request->vals()), 12, std::begin(cx.vals));
        const auto result = ::HashComplex(cx);
        response->set_val(result);
        return Status::OK;
    }

    Status AverageContainer_double(ServerContext* context,
        const grpc_benchmark::Vec_Double* request, grpc_benchmark::Double* response) override
    {
        const auto num_vals = request->val().size();
        std::vector<double> vec(std::begin(request->val()), std::end(request->val()));
        const auto result = ::AverageContainer<double>(vec);
        response->set_val(result);
        return Status::OK;
    }

    Status AverageContainer_uint64(ServerContext* context,
        const grpc_benchmark::Vec_Uint64* request, grpc_benchmark::Double* response) override
    {
        const auto num_vals = request->val().size();
        std::vector<uint64_t> vec(std::begin(request->val()), std::end(request->val()));
        const auto result = ::AverageContainer<uint64_t>(vec);
        response->set_val(result);
        return Status::OK;
    }

    Status KillServer(ServerContext* context, const grpc_benchmark::Empty* request,
        grpc_benchmark::Empty* response) override
    {
        //std::ignore = std::async(::KillServer);
        //std::exit(EXIT_SUCCESS);
        std::abort();
        return Status::OK;
    }
};

void RunServer()
{
    std::string server_address("127.0.0.1:5200");
    RpcBenchmarkImpl service{};

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    P_SERVER = std::unique_ptr<Server>{ builder.BuildAndStart() };
    P_SERVER->Wait();
}
} //namespace

int main()
{
    RunServer();
    return EXIT_SUCCESS;
}
