![cmake](https://github.com/jharmer95/rpc.hpp/workflows/cmake/badge.svg?branch=main&event=push) [![codecov](https://codecov.io/gh/jharmer95/rpc.hpp/branch/main/graph/badge.svg)](https://codecov.io/gh/jharmer95/rpc.hpp)

# rpc.hpp

A simple header-only library for supporting remote procedure calls using a variety of extensible
features.

## License

`rpc.hpp` is licensed under the [BSD 3-Clause License](LICENSE).

## Features

- Cross-platform.
  - Supports every major compiler/operating system.
- Easy to use.
  - Only requires a few lines of code to provide the implementation of your client or server.
- Modern.
  - Takes advantage of a lot of compile-time and generic programming features such as C++17's
    `constexpr if`.
- Fast.
  - See [COMPARISON](COMPARISON.md) to see how this library compares to the likes of
    `librpc` and `gRPC`.
- Support for various types.
  - `std::string` / `std::vector` supported out of the box (more STL containers to come).
  - Users can create serialization member functions for their own custom types.
    - Users can also provide `template` methods for serializing types outside of their control
- Extensible support via "adapters".
  - Currently supported:
    - [nlohmann-json](https://github.com/nlohmann/json)
    - [rapidjson](https://github.com/Tencent/rapidjson)
    - [Boost.JSON](https://github.com/boostorg/json)
    - [bitsery](https://github.com/fraillt/bitsery)

## Known Limitations

- Pointers and arrays (other than `const char*` / `const char[]` for strings) _**cannot**_ be
    passed across an RPC boundary.
- Functions with default arguments _**should not**_ be used with remote procedure calls as the
    default argument(s) must be provided each time anyway.
- Function overloads _**will not work**_ across an RPC boundary as the server will have an
    ambiguous call.
  - This includes non-explicit template functions
      (see [examples](examples) for the syntax for calling template functions remotely).
- Not (statically) type-safe across the RPC boundary.
  - This is the nature of a dynamic system like remote procedure calls.
  - Return type and explicit parameters must be specified at call site.
  - Exceptions will be able to catch function signature mismatches.
  - Function signatures _can_ be provided via a shared header to allow for strict compile-time
      type-checking (by using `call_header_func()`).

## Documentation

See Doxygen docs [here](https://jharmer95.github.io/rpc.hpp/).

## Basic Example

For more examples see [examples](examples).

server.cpp

```C++
#define RPC_HPP_SERVER_IMPL

#include <rpc_adapters/rpc_njson.hpp>

#include <string>

using rpc_hpp::adapters::njson;
using rpc_hpp::adapters::njson_adapter;

int Add(int n1, int n2)
{
    return n1 + n2;
}

void AppendStr(std::string& str, const char* append)
{
    str.append(append);
}

class RpcServer : public rpc_hpp::server_interface<njson_adapter>
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

        // Sink data into the dispatch function to get result data
		auto result_data = dispatch(std::move(data));

        // Send result data back to client...
    }
};

int main()
{
    RpcServer my_server{"address"};
    my_server.bind("Add", &Add);
    my_server.bind("AppendStr",
        [](std::string& str, const std::string& append) {
            AppendStr(str, append.c_str());
        }
    );

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

using rpc_hpp::adapters::njson_adapter;

class RpcClient : public rpc_hpp::client_interface<njson_adapter>
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
