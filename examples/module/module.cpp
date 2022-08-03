#include "module.hpp"

#include <cstring>

static constexpr auto MODULE_NAME{ "rpc_module" };
static RpcModule rpc_mod{};

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

RpcModule::RpcModule()
{
    bind("Sum", &Sum);
    bind("AddOneToEach", &AddOneToEach);
    bind("GetName", &GetName);
}

int RunRemoteFunc(char* const json_str, const size_t json_buf_len)
{
    const auto input = rpc_mod.dispatch(json_str);

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
