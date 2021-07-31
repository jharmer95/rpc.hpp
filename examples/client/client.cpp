#define RPC_HPP_CLIENT_IMPL

#include "client.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "USAGE: rpc_client <server_ipv4> <port_num>\n");
        return EXIT_FAILURE;
    }

    RpcClient client{ argv[1], argv[2] };

    std::string currentFuncName;

    try
    {
        // Trivial function example
        {
            currentFuncName = "Sum";
            const auto result = client.template call_func<int>("Sum", 1, 2);
            printf("Sum(1, 2) == %d\n", result);
        }

        // Example of calling w/ references
        {
            currentFuncName = "AddOneToEach";
            std::vector<int> vec{ 1, 2, 3, 4, 5 };

            client.template call_func<void>("AddOneToEach", vec);
            printf("AddOneToEach({ 1, 2, 3, 4, 5 }) == {");

            for (size_t i = 0; i < vec.size() - 1; ++i)
            {
                printf(" %d,", vec[i]);
            }

            printf("%d }\n", vec.back());
        }

        // Template function example
        {
            currentFuncName = "GetTypeName<int>";
            auto result = client.template call_func<std::string>("GetTypeName<int>");

            printf("GetTypeName<int>() == \"%s\"\n", result.c_str());

            currentFuncName = "GetTypeName<double>";
            result = client.template call_func<std::string>("GetTypeName<double>");

            printf("GetTypeName<double>() == \"%s\"\n", result.c_str());

            currentFuncName = "GetTypeName<std::string>";
            result = client.template call_func<std::string>("GetTypeName<std::string>");

            printf("GetTypeName<std::string>() == \"%s\"\n", result.c_str());
        }

        // Now shutdown the server
        {
            currentFuncName = "KillServer";
            client.call_func("KillServer");
            printf("Server shutdown remotely...\n");
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Call to '%s' failed, reason: %s\n", currentFuncName.c_str(), ex.what());
        return EXIT_FAILURE;
    }
}
