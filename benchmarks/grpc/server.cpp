#include "server.hpp"

#include <algorithm>
#include <future>
#include <sstream>
#include <thread>

std::unique_ptr<Server> P_SERVER;

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
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //P_SERVER->Shutdown();
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
        std::ignore = std::async(::KillServer);
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
    P_SERVER = std::unique_ptr<Server>(builder.BuildAndStart());
    P_SERVER->Wait();
}

int main()
{
    RunServer();
    return EXIT_SUCCESS;
}
