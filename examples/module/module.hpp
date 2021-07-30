#pragma once

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
#endif

extern "C"
{
    DLL_PRIVATE int Sum(int n1, int n2);
    DLL_PRIVATE void AddOneToEach(std::vector<int>& vec);
    DLL_PRIVATE void GetName(std::string& name_out);

    DLL_PUBLIC std::string RunRemoteFunc(const std::string& json_str);
}
