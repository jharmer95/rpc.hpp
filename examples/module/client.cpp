#define RPC_HPP_CLIENT_IMPL

#include "client.hpp"

#include <rpc_adapters/rpc_njson.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using rpc_hpp::adapters::njson_adapter;

RpcClient::RpcClient(const std::string& module_path) : m_module{ LoadLibrary(module_path.c_str()) }
{
    // Load the module into the application's memory
    if (m_module == nullptr)
    {
        throw std::runtime_error("Could not load module!");
    }

    // Locate the rpc.hpp handler function within the module
    m_func = reinterpret_cast<remote_func_type>(GetProcAddress(m_module, "RunRemoteFunc"));

    if (m_func == nullptr)
    {
        throw std::runtime_error("Could not load function 'RunRemoteFunc'!");
    }
}

void RpcClient::send(const std::string& mesg)
{
    // Interoperable with C-compatible code, so need to create a character buffer (keeping it small for this example)
    constexpr auto BUF_SZ = 128;

    if (mesg.size() >= BUF_SZ)
    {
        throw std::runtime_error("String buffer was not big enough for request!");
    }

    char buf[BUF_SZ];

#if defined(_WIN32)
    strcpy_s(buf, mesg.c_str());

#elif defined(__unix__)
    strcpy(buf, mesg.c_str());
#endif

    const auto result = m_func(buf, BUF_SZ);

    if (result == 1)
    {
        throw std::runtime_error("String buffer was not big enough for response!");
    }

    m_result = std::string{ buf };
}

void RpcClient::send(std::string&& mesg)
{
    // Interoperable with C-compatible code, so need to create a character buffer (keeping it small for this example)
    constexpr auto BUF_SZ = 128;

    if (mesg.size() >= BUF_SZ)
    {
        throw std::runtime_error("String buffer was not big enough for request!");
    }

    char buf[BUF_SZ];

#if defined(_WIN32)
    strcpy_s(buf, std::move(mesg).c_str());

#elif defined(__unix__)
    strcpy(buf, std::move(mesg).c_str());
#endif

    const auto result = m_func(buf, BUF_SZ);

    if (result == 1)
    {
        throw std::runtime_error("String buffer was not big enough for response!");
    }

    m_result = std::string{ buf };
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "USAGE: rpc_module_client <module_path>\n";
        return EXIT_FAILURE;
    }

    try
    {
        std::string funcName;
        RpcClient client{ argv[1] };

        try
        {
            // Trivial function example
            {
                funcName = "Sum";

                const auto result = client.template call_func<int>("Sum", 1, 2);
                std::cout << "Sum(1, 2) == " << result << '\n';
            }

            // Example of calling w/ references
            {
                funcName = "AddOneToEach";
                std::vector<int> vec{ 1, 2, 3, 4, 5 };

                client.call_func("AddOneToEach", vec);
                std::cout << "AddOneToEach({ 1, 2, 3, 4, 5 }) == {";

                for (size_t i = 0; i < vec.size() - 1; ++i)
                {
                    std::cout << ' ' << vec[i] << ", ";
                }

                std::cout << ' ' << vec.back() << " }\n";
            }

            // Local storage return
            {
                funcName = "GetName";
                std::string mod_name;

                client.call_func("GetName", mod_name);
                std::cout << "GetName() == \"" << mod_name << "\"\n";
            }

            return EXIT_SUCCESS;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Call to '" << funcName << "' failed, reason: " << ex.what() << '\n';
            return EXIT_FAILURE;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Loading module '" << argv[1] << "' failed, reason: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
