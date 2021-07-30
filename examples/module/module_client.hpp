#pragma once

#if defined(_WIN32)
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <Windows.h>

using module_t = HMODULE;

#elif defined(__unix__)
#    include <dlfcn.h>

using module_t = void*;
#endif

#include <rpc_adapters/rpc_njson.hpp>

#include <stdexcept>

using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

class RpcClient : public rpc::client::client_interface<njson_adapter>
{
public:
    using remote_func_type = std::string (*)(const std::string&);

    RpcClient(const std::string& module_path)
    {
#if defined(_WIN32)
        m_module = LoadLibrary(module_path.c_str());
#elif defined(__unix__)
        m_module = dlopen(module_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif

        if (m_module == nullptr)
        {
            throw std::runtime_error("Could not load module!");
        }

#if defined(_WIN32)
        m_func = reinterpret_cast<remote_func_type>(GetProcAddress(m_module, "RunRemoteFunc"));
#elif defined(__unix__)
        m_func = reinterpret_cast<remote_func_type>(dlsym(m_module, "RunRemoteFunc"));
#endif

        if (m_func == nullptr)
        {
            throw std::runtime_error("Could not load function 'RunRemoteFunc'!");
        }
    }

private:
    void send(const std::string& mesg) override { m_result = m_func(mesg); }
    std::string receive() override { return m_result; }

    module_t m_module;
    remote_func_type m_func;
    std::string m_result;
};
