#define RPC_HPP_MODULE_IMPL

#include "module.hpp"

#include <rpc_adapters/rpc_njson.hpp>
#include <rpc_dispatch_helper.hpp>

using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

constexpr auto MODULE_NAME{ "rpc_module" };

int Sum(int n1, int n2)
{
    return n1 + n2;
}

void AddOneToEach(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

void GetName(std::string& name_out)
{
    name_out.assign(MODULE_NAME);
}

RPC_DEFAULT_DISPATCH(Sum, AddOneToEach, GetName)

std::string RunRemoteFunc(const std::string& json_str)
{
    std::string input = json_str;

    rpc::server::dispatch<njson_adapter>(input);

    return input;
}
