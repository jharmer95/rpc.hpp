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
            const auto pack = client.call_func<int>("Sum", 1, 2);
            printf("Sum(1, 2) == %d\n", pack.get_result());
        }

        // Example of calling w/ references
        {
            currentFuncName = "AddOneToEach";
            std::vector<int> vec{ 1, 2, 3, 4, 5 };

            const auto pack = client.call_func<void>("AddOneToEach", vec);
            const auto vec2 = pack.get_arg<0>();
            printf("AddOneToEach({ 1, 2, 3, 4, 5 }) == {");

            for (size_t i = 0; i < vec2.size() - 1; ++i)
            {
                printf(" %d,", vec2[i]);
            }

            printf("%d }\n", vec2.back());
        }

        // Template function example
        {
            currentFuncName = "GetTypeName<int>";
            auto pack1 = client.call_func<std::string>("GetTypeName<int>");

            printf("GetTypeName<int>() == \"%s\"\n", pack1.get_result().c_str());

            currentFuncName = "GetTypeName<double>";
            auto pack2 = client.call_func<std::string>("GetTypeName<double>");

            printf("GetTypeName<double>() == \"%s\"\n", pack2.get_result().c_str());

            currentFuncName = "GetTypeName<std::string>";
            auto pack3 = client.call_func<std::string>("GetTypeName<std::string>");

            printf("GetTypeName<std::string>() == \"%s\"\n", pack3.get_result().c_str());
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
