![cmake](https://github.com/jharmer95/rpc.hpp/workflows/cmake/badge.svg?branch=main&event=push) [![codecov](https://codecov.io/gh/jharmer95/rpc.hpp/branch/main/graph/badge.svg)](https://codecov.io/gh/jharmer95/rpc.hpp)

# rpc.hpp

A simple header-only library for supporting remote procedure calls using a variety of extensible
features.

## License

`rpc.hpp` is licensed under the [BSD 3-Clause License](LICENSE).

## Features

- Cross-platform.
  - Supports every major compiler/operating system.
- Modern.
  - Utilizes features like C++17's [`constexpr if`](https://en.cppreference.com/w/cpp/language/if).
- Type-safe support for various types.
  - Supports most built-in/STL types out of the box.
  - Users can create serialization methods for their own custom types.
- Easy to use.
- Extensible support via "adapters".
  - Currently supported:
    - [nlohmann-json](https://github.com/nlohmann/json)
    - [rapidjson](https://github.com/Tencent/rapidjson)
    - [Boost.JSON](https://github.com/boostorg/json)
    - [bitsery](https://github.com/fraillt/bitsery)

## Known Limitations

- Functions with default arguments _**should not**_ be used with remote procedure calls as the default argument(s) must be provided each time anyway
- Function overloads _**will not**_ work across an RPC boundary as the server will have an ambiguous call
  - This includes non-explicit template functions (see [examples](examples) for the syntax for calling template functions remotely)
- Not (statically) type-safe across the RPC boundary
  - This is the nature of a dynamic system like remote procedure calls
  - Return type and explicit parameters must be specified at callsite
  - Exceptions will be able to catch function signature mismatches
  - Function signatures can be provided via a shared header to allow for compile-time type-checking (ala `call_header_func()`)

## Documentation

See Doxygen docs [here](https://jharmer95.github.io/rpc.hpp/).

## Basic Example

For more examples see [examples](examples).

server.cpp

```C++
#define RPC_HPP_SERVER_IMPL

#include <rpc_adapters/rpc_njson.hpp>

#include <string>

using rpc::adapters::njson;
using rpc::adapters::njson_adapter;

int Add(int n1, int n2)
{
    return n1 + n2;
}

void AppendStr(std::string& str, const char* append)
{
    str.append(append);
}

class RpcServer : public rpc::server_interface<njson_adapter>
{
public:
    RpcServer(const char* address)
    {
        // initialize server...
    }

    // ...

    void Run()
    {
        std::string data;

        // Get data from client...

        dispatch(data);

        // Send data back to client...
    }
};

int main()
{
    RpcServer my_server{"address"};
    my_server.bind("Add", &Add);
    my_server.bind("AppendStr", &AppendStr);

    while (true)
    {
        my_server.Run();
    }
}
```

client.cpp

```C++
#define RPC_HPP_CLIENT_IMPL

#include <rpc_adapters/rpc_njson.hpp>

#include <cassert>
#include <string>

using rpc::adapters::njson_adapter;

class RpcClient : public rpc::client_interface<njson_adapter>
{
public:
    RpcClient(const char* address)
    {
        // initialize client...
    }

    // ...

private:
    void send(const std::string& mesg) override
    {
        // Send mesg to server...
    }

    void send(std::string&& mesg) override
    {
        // Send mesg to server...
    }

    std::string receive() override
    {
        // Get message back from server...
    }
};

int main()
{
    RpcClient my_client{ "address" };

    const auto result = my_client.template call_func<int>("Sum", 1, 2);
    assert(result == 3);

    std::string str{ "Hello" };
    my_client.call_func("AppendStr", str, " world!");
    assert(str == "Hello world!");
}
```
