#include "client.hpp"

#include <exception>
#include <iostream>
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

            const auto response = client.call_func("Sum", 1, 2);

            if (response.is_error())
            {
                throw std::runtime_error{ "Received an error: " + response.get_error_mesg() };
            }

            const auto result = response.template get_result<int>();
            std::cout << "Sum(1, 2) == " << result << '\n';
        }

        // Example of calling w/ references
        {
            currentFuncName = "AddOneToEach";
            std::vector<int> vec{ 1, 2, 3, 4, 5 };

            const auto response = client.template call_func_w_bind("AddOneToEach", vec);

            if (response.is_error())
            {
                throw std::runtime_error{ "Received an error: " + response.get_error_mesg() };
            }

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
            auto response = client.call_func("GetTypeName<int>");

            if (response.is_error())
            {
                throw std::runtime_error{ "Received an error: " + response.get_error_mesg() };
            }

            auto result = response.template get_result<std::string>();
            std::cout << "GetTypeName<int>() == \"" << result << "\"\n";

            currentFuncName = "GetTypeName<double>";
            response = client.call_func("GetTypeName<double>");

            if (response.is_error())
            {
                throw std::runtime_error{ "Received an error: " + response.get_error_mesg() };
            }

            result = response.template get_result<std::string>();
            std::cout << "GetTypeName<double>() == \"" << result << "\"\n";

            currentFuncName = "GetTypeName<std::string>";
            response = client.call_func("GetTypeName<std::string>");

            if (response.is_error())
            {
                throw std::runtime_error{ "Received an error: " + response.get_error_mesg() };
            }

            result = response.template get_result<std::string>();
            std::cout << "GetTypeName<std::string>() == \"" << result << "\"\n";
        }

        // Now shutdown the server
        {
            currentFuncName = "KillServer";
            [[maybe_unused]] const auto response = client.call_func("KillServer");
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
