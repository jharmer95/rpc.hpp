#define RPC_HPP_MODULE_IMPL

#include "module.hpp"

#include <rpc_dispatch_helper.hpp>

#include <cstring>

constexpr auto MODULE_NAME{ "rpc_module" };

static RpcModule rpc_mod;

inline int Sum(int n1, int n2)
{
    return n1 + n2;
}

inline void AddOneToEach(std::vector<int>& vec)
{
    for (auto& n : vec)
    {
        ++n;
    }
}

inline void GetName(std::string& name_out)
{
    name_out.assign(MODULE_NAME);
}

void RpcModule::dispatch_impl(njson& serial_obj)
{
    RPC_DEFAULT_DISPATCH(Sum, AddOneToEach, GetName)
}

int RunRemoteFunc(char* const json_str, const size_t json_buf_len)
{
    std::string input{ json_str };
    rpc_mod.dispatch(input);

    if (input.size() >= json_buf_len)
    {
        return 1;
    }

#if defined(_WIN32)
    strcpy_s(json_str, json_buf_len, input.c_str());

#elif defined(__unix__)
    strcpy(json_str, input.c_str());
#endif
    return 0;
}
