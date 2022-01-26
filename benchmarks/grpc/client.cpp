#include "client.hpp"

struct ComplexObject
{
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};
};

double gRPC_Client::StdDev(double d1, double d2, double d3, double d4, double d5, double d6,
    double d7, double d8, double d9, double d10) const
{
    grpc::ClientContext context;
    grpc_benchmark::TenDoubles request;
    request.set_d1(d1);
    request.set_d2(d2);
    request.set_d3(d3);
    request.set_d4(d4);
    request.set_d5(d5);
    request.set_d6(d6);
    request.set_d7(d7);
    request.set_d8(d8);
    request.set_d9(d9);
    request.set_d10(d10);

    grpc_benchmark::Double response;
    m_stub->StdDev(&context, request, &response);
    return response.val();
}

std::vector<uint64_t> gRPC_Client::GenRandInts(uint64_t min, uint64_t max, size_t sz) const
{
    grpc::ClientContext context;
    grpc_benchmark::GenRandInts_Msg request;
    request.set_min(min);
    request.set_max(max);
    request.set_sz(sz);

    grpc_benchmark::Vec_Uint64 response;
    m_stub->GenRandInts(&context, request, &response);

    std::vector<uint64_t> vec{};
    vec.reserve(response.val().size());

    std::copy(std::begin(response.val()), std::end(response.val()), std::back_inserter(vec));
    return vec;
}

uint64_t gRPC_Client::Fibonacci(uint64_t n) const
{
    grpc::ClientContext context;
    grpc_benchmark::Uint64 request;
    request.set_val(n);

    grpc_benchmark::Uint64 response;
    m_stub->Fibonacci(&context, request, &response);
    return response.val();
}

std::string gRPC_Client::HashComplex(const ::ComplexObject& cx) const
{
    grpc::ClientContext context;
    grpc_benchmark::ComplexObject request;
    request.set_id(cx.id);
    request.set_name(cx.name);
    request.set_flag1(cx.flag1);
    request.set_flag2(cx.flag2);
    request.set_vals(cx.vals.data(), 12);

    grpc_benchmark::String response;
    m_stub->HashComplex(&context, request, &response);
    return response.val();
}

double gRPC_Client::AverageContainer_double(const std::vector<double>& vec) const
{
    grpc::ClientContext context;
    grpc_benchmark::Vec_Double request;

    for (const auto val : vec)
    {
        request.add_val(val);
    }

    grpc_benchmark::Double response;
    m_stub->AverageContainer_double(&context, request, &response);
    return response.val();
}

double gRPC_Client::AverageContainer_uint64(const std::vector<uint64_t>& vec) const
{
    grpc::ClientContext context;
    grpc_benchmark::Vec_Uint64 request;

    for (const auto val : vec)
    {
        request.add_val(val);
    }

    grpc_benchmark::Double response;
    m_stub->AverageContainer_uint64(&context, request, &response);
    return response.val();
}

void gRPC_Client::KillServer()
{
    grpc::ClientContext context;
    grpc_benchmark::Empty request;
    grpc_benchmark::Empty response;
    m_stub->KillServer(&context, request, &response);
    Stop();
}
