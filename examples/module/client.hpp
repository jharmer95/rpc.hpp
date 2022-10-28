#pragma once

#include <rpc_adapters/rpc_njson.hpp>
#include <rpc_client.hpp>

#include <string>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Windows.h>

using module_t = HMODULE;
#elif defined(__unix__)
#  include <dlfcn.h>

// For cleaner syntax, match macros w/ Windows (not recommended for production as behaviors/flags can be different)
#  define FreeLibrary(X) dlclose(X)
#  define GetProcAddress(X, Y) dlsym(X, Y)
#  define LoadLibrary(X) dlopen(X, RTLD_LOCAL | RTLD_LAZY)

using module_t = void*;
#endif

class RpcClient : public rpc_hpp::client_interface<rpc_hpp::adapters::njson_adapter>
{
public:
    using remote_func_type = int (*)(char*, size_t);

    ~RpcClient() override
    {
        // Don't forget to release the module when done
        if (m_module != nullptr)
        {
            FreeLibrary(m_module);
        }
    }

    RpcClient(std::string_view module_path);

    // Cannot copy a client, module could be unloaded by a copy
    RpcClient(const RpcClient&) = delete;

    // Moving is OK, module pointer will not be unloaded
    RpcClient(RpcClient&& other) noexcept
        : m_module(other.m_module), m_func(other.m_func), m_result(std::move(other.m_result))
    {
        other.m_module = nullptr;
        other.m_func = nullptr;
    }

    // Cannot copy a client, module could be unloaded by a copy
    RpcClient& operator=(const RpcClient&) = delete;

    RpcClient& operator=(RpcClient&& other) & noexcept
    {
        m_module = other.m_module;
        m_func = other.m_func;
        m_result = std::move(other.m_result);

        other.m_module = nullptr;
        other.m_func = nullptr;

        return *this;
    }

private:
    void send(std::string&& bytes) override;

    [[nodiscard]] std::string receive() override
    {
        // By moving, we indicate that the result is no longer valid after the function returns
        return std::move(m_result);
    }

    module_t m_module{ nullptr };
    remote_func_type m_func{ nullptr };
    std::string m_result{};
};
