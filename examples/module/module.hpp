#pragma once

#include <rpc_adapters/rpc_njson.hpp>

using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

#include <string>
#include <vector>

#if defined(_WIN32)
#    if defined(RPC_HPP_EXPORT)
#        define DLL_PUBLIC __declspec(dllexport)
#    else
#        define DLL_PUBLIC __declspec(dllimport)
#    endif

#    define DLL_PRIVATE

#elif defined(__unix__)
#    define DLL_PUBLIC __attribute__((visibility("default")))
#    define DLL_PRIVATE __attribute__((visibility("hidden")))
#    define strcpy_s(X, Y) strcpy(X, Y)
#endif

// C++ private functions
DLL_PRIVATE int Sum(int n1, int n2);
DLL_PRIVATE void AddOneToEach(std::vector<int>& vec);
DLL_PRIVATE void GetName(std::string& name_out);

extern "C"
{
    // C-Compatible way to run plugin functions dynamically
    DLL_PUBLIC int RunRemoteFunc(char* json_str, size_t json_buf_len);
}

class RpcModule : public rpc::server_interface<njson_adapter>
{
private:
    void dispatch_impl(njson& serial_obj) override;
};
