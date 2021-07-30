#define RPC_HPP_CLIENT_IMPL

#include "module_client.hpp"

#include <rpc_adapters/rpc_njson.hpp>

#include <cstdio>
#include <stdexcept>
#include <string>

using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: rpc_module_client <module_path>\n");
        return EXIT_FAILURE;
    }

    try
    {
        RpcClient client{ argv[1] };
        std::string funcName;

        try
        {
            // Trivial function example
            {
                funcName = "Sum";
                const auto pack = client.call_func<int>("Sum", 1, 2);
                printf("Sum(1, 2) == %d\n", pack.get_result());
            }

            // Example of calling w/ references
            {
                funcName = "AddOneToEach";
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

            // Local storage return
            {
                funcName = "GetName";
                std::string mod_name;

                const auto pack = client.call_func<std::string>("GetName", mod_name);

                printf("GetName() == \"%s\"\n", pack.get_arg<0>().c_str());
            }

            return EXIT_SUCCESS;
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "Call to '%s' failed, reason: %s\n", funcName.c_str(), ex.what());
            return EXIT_FAILURE;
        }
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Loading module '%s' failed, reason: %s\n", argv[1], ex.what());
        return EXIT_FAILURE;
    }
}
