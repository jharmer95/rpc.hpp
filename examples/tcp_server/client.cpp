#define RPC_HPP_CLIENT_IMPL

#include "client.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "USAGE: rpc_client <server_ipv4> <port_num>\n";
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
            std::cout << "Sum(1, 2) == " << result << '\n';
        }

        // Example of calling w/ references
        {
            currentFuncName = "AddOneToEach";
            std::vector<int> vec{ 1, 2, 3, 4, 5 };

            client.template call_func<void>("AddOneToEach", vec);
            std::cout << "AddOneToEach({ 1, 2, 3, 4, 5 }) == {";

            for (size_t i = 0; i < vec.size() - 1; ++i)
            {
                std::cout << ' ' << vec[i] << ", ";
            }

            std::cout << ' ' << vec.back() << " }\n";
        }

        // Template function example
        {
            currentFuncName = "GetTypeName<int>";
            auto result = client.template call_func<std::string>("GetTypeName<int>");

            std::cout << "GetTypeName<int>() == \"" << result << "\"\n";

            currentFuncName = "GetTypeName<double>";
            result = client.template call_func<std::string>("GetTypeName<double>");

            std::cout << "GetTypeName<double>() == \"" << result << "\"\n";

            currentFuncName = "GetTypeName<std::string>";
            result = client.template call_func<std::string>("GetTypeName<std::string>");

            std::cout << "GetTypeName<std::string>() == \"" << result << "\"\n";
        }

        // Now shutdown the server
        {
            currentFuncName = "KillServer";
            client.call_func("KillServer");
            std::cout << "Server shutdown remotely...\n";
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Call to '" << currentFuncName << "' failed, reason: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
