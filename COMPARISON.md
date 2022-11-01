# Comparison of RPC Options

## rpc.hpp Adapter Comparison

For relative performance benchmarks for the adapters of `rpc.hpp` see [below](#performance)

## Alternatives for Consideration

### rpclib

### gRPC

## Comparison to Alternatives

### Features

| feature                                          | `rpc.hpp`                                                                                                                | `rpclib`           | `gRPC`                                                     |
|--------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------|--------------------|------------------------------------------------------------|
| Get result of remote function call               | :heavy_check_mark:                                                                                                       | :heavy_check_mark: | :heavy_check_mark:                                         |
| Bind reference parameters of RPC                 | :heavy_check_mark:                                                                                                       | :x:                | :x:                                                        |
| Call RPCs without headers from the server        | :heavy_check_mark:                                                                                                       | :heavy_check_mark: | :x:                                                        |
| Doesn't require external compilation             | :heavy_check_mark:                                                                                                       | :heavy_check_mark: | :x:                                                        |
| No external dependencies (outside of networking) | :heavy_check_mark:                                                                                                       | :heavy_check_mark: | :x: <br> abseil, c-ares, <br> protobuf, openssl, re2, zlib |
| Header-only                                      | :heavy_check_mark:                                                                                                       | :x:                | :x:                                                        |
| Adaptable via "plugins"                          | :heavy_check_mark:                                                                                                       | :x:                | :x:                                                        |
| Compile-time type safety                         | :eight_spoked_asterisk: <br> (if using shared headers and `call_header_func`)                                            | :x:                | :heavy_check_mark:                                         |
| Serialization method(s)                          | Any provided by adapters (currently: JSON, bitsery)                                                                      | msgpack            | flatbuffers, protobuffers                                  |
| RPC/IPC methods supported                        | TCP, Websockets, DLL, shared memory, and more... <br> (Just implement the client and server interfaces to pass messages) | TCP only           | HTTP only                                                  |

### Performance

The following tests were performed using nanobench.
In each case, the call is done over a TCP socket on the same machine (inter-process call) to reduce
noise from network traffic.

For each of these tests, server-side caching was _disabled_ for `rpc.hpp`.
To see the additional benefits of enabling the function cache, see the [caching](#function-caching)
section.

Each "op" is a call to the defined function on the server.

#### Fibonacci

```C++
uint64_t Fibonacci(uint64_t n)
{
    return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}
```

| relative |      ns/op |     op/s | err% | total | RPC Method                        |
|---------:|-----------:|---------:|-----:|------:|:----------------------------------|
|   100.0% | 131,292.32 | 7,616.59 | 1.1% |  3.25 | `rpc.hpp (asio::tcp, njson)`      |
|   105.0% | 125,005.46 | 7,999.65 | 2.6% |  3.14 | `rpc.hpp (asio::tcp, rapidjson)`  |
|   110.6% | 118,692.83 | 8,425.11 | 1.6% |  2.90 | `rpc.hpp (asio::tcp, Boost.JSON)` |
|   127.8% | 102,718.72 | 9,735.32 | 1.2% |  2.48 | `rpc.hpp (asio::tcp, bitsery)`    |
|    61.9% | 212,014.89 | 4,716.65 | 0.8% |  5.14 | `rpclib`                          |
|    74.8% | 175,597.70 | 5,694.84 | 1.0% |  4.23 | `gRPC`                            |

#### HashComplex

```C++
struct ComplexObject
{
    int id{};
    std::string name{};
    bool flag1{};
    bool flag2{};
    std::array<uint8_t, 12> vals{};
};

std::string HashComplex(ComplexObject cx)
{
    std::stringstream hash;

    if (cx.flag1)
    {
        std::reverse(cx.vals.begin(), cx.vals.end());
    }

    for (size_t i = 0; i < cx.name.size(); ++i)
    {
        const int acc = cx.flag2 ? cx.name[i] + cx.vals[i % 12] : cx.name[i] - cx.vals[i % 12];
        hash << std::hex << acc;
    }

    return hash.str();
}
```

| relative |      ns/op |      op/s | err% | total | RPC Method                        |
|---------:|-----------:|----------:|-----:|------:|:----------------------------------|
|   100.0% | 135,644.92 |  7,372.19 | 2.2% |  3.30 | `rpc.hpp (asio::tcp, njson)`      |
|   125.8% | 107,826.49 |  9,274.16 | 2.3% |  2.58 | `rpc.hpp (asio::tcp, rapidjson)`  |
|   118.8% | 114,175.54 |  8,758.44 | 1.3% |  2.81 | `rpc.hpp (asio::tcp, Boost.JSON)` |
|   181.5% |  74,727.74 | 13,381.91 | 2.7% |  1.86 | `rpc.hpp (asio::tcp, bitsery)`    |
|    72.7% | 186,623.39 |  5,358.38 | 2.2% |  4.48 | `rpclib`                          |
|    86.7% | 156,521.63 |  6,388.89 | 2.6% |  3.78 | `gRPC`                            |

#### StdDev

```C++
double StdDev(double n1, double n2, double n3, double n4, double n5,
    double n6, double n7, double n8, double n9, double n10)
{
    const auto avg = Average(
        n1 * n1, n2 * n2, n3 * n3, n4 * n4, n5 * n5, n6 * n6,
        n7 * n7, n8 * n8, n9 * n9, n10 * n10);

    return std::sqrt(avg);
}
```

| relative |      ns/op |      op/s | err% | total | RPC Method                        |
|---------:|-----------:|----------:|-----:|------:|:----------------------------------|
|   100.0% | 112,864.05 |  8,860.22 | 2.8% |  2.82 | `rpc.hpp (asio::tcp, njson)`      |
|   155.0% |  72,815.62 | 13,733.32 | 0.9% |  1.80 | `rpc.hpp (asio::tcp, rapidjson)`  |
|   148.5% |  75,994.39 | 13,158.87 | 1.0% |  1.87 | `rpc.hpp (asio::tcp, Boost.JSON)` |
|   189.4% |  59,586.13 | 16,782.43 | 2.7% |  1.45 | `rpc.hpp (asio::tcp, bitsery)`    |
|    66.4% | 169,877.21 |  5,886.60 | 2.0% |  4.17 | `rpclib`                          |
|    89.1% | 126,717.13 |  7,891.59 | 3.2% |  3.08 | `gRPC`                            |

#### AverageContainer

```C++
template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}
```

| relative |      ns/op |      op/s | err% | total | RPC Method                        |
|---------:|-----------:|----------:|-----:|------:|:----------------------------------|
|   100.0% | 116,881.56 |  8,555.67 | 4.8% |  2.88 | `rpc.hpp (asio::tcp, njson)`      |
|   149.0% |  78,432.09 | 12,749.88 | 4.9% |  2.00 | `rpc.hpp (asio::tcp, rapidjson)`  |
|   133.5% |  87,575.67 | 11,418.70 | 3.0% |  2.11 | `rpc.hpp (asio::tcp, Boost.JSON)` |
|   176.0% |  66,399.04 | 15,060.46 | 1.8% |  1.64 | `rpc.hpp (asio::tcp, bitsery)`    |
|    67.0% | 174,553.87 |  5,728.89 | 3.8% |  4.30 | `rpclib`                          |
|    91.5% | 127,777.78 |  7,826.09 | 3.7% |  3.19 | `gRPC`                            |

#### Sequential

The "Sequential" benchmark aims to replicate real-world behavior where the input of one function
might depend on the result of another:

```C++
// Server
std::vector<uint64_t> GenRandInts(const uint64_t min, const uint64_t max, const size_t sz)
{
    std::vector<uint64_t> vec;
    vec.reserve(sz);

    for (size_t i = 0; i < sz; ++i)
    {
        vec.push_back(static_cast<uint64_t>(std::rand()) % (max - min + 1) + min);
    }

    return vec;
}
```

```C++
// Client
std::vector<uint64_t> vec = SomeRPCMethod("GenRandInts", 5, 30, 1'000);

for (auto& val : vec)
{
    uint64_t val = SomeRPCMethod("Fibonacci", val);
}

double avg = SomeRPCMethod("AverageContainer", vec);
```

| relative |          ns/op | op/s | err% | total | RPC Method                        |
|---------:|---------------:|-----:|-----:|------:|:----------------------------------|
|   100.0% | 603,869,933.33 | 1.66 | 1.1% | 20.76 | `rpc.hpp (asio::tcp, njson)`      |
|   107.3% | 562,596,833.33 | 1.78 | 2.3% | 19.03 | `rpc.hpp (asio::tcp, rapidjson)`  |
|   107.6% | 561,025,000.00 | 1.78 | 2.2% | 19.14 | `rpc.hpp (asio::tcp, Boost.JSON)` |
|   110.8% | 545,018,366.67 | 1.83 | 2.2% | 18.59 | `rpc.hpp (asio::tcp, bitsery)`    |
|    89.6% | 673,604,233.33 | 1.48 | 2.2% | 23.03 | `rpclib`                          |
|    86.9% | 695,083,500.00 | 1.44 | 2.1% | 23.45 | `gRPC`                            |

## Summary

`rpc.hpp` can offer some advantages over other RPC solutions, namely in its flexibility,
ease of use, and relative performance.

### Compared to rpclib

`rpc.hpp` is very similar in design to rpclib. Both provide a way to serialize user types and
package functions and a simple interface to both bind function calls to the server and call them
from a client.

The main advantage `rpc.hpp` provides over rpclib is in its flexibility.
While rpclib focuses on its `rpc-msgpack` implementation, `rpc.hpp` provides a fairly generic way
to package functions, allowing for the use of different serialization techniques and formats.

Additionally, rpclib only has one transport method: using `asio` to pass messages via TCP.
`rpc.hpp` allows servers to be customized to use any transport method the developer wants, including
virtual transport via something like a DLL boundary or shared memory.

`rpc.hpp` also provides a pretty significant performance improvement over rpclib, even without
caching enabled (between 10-38% faster in testing when using nlohmann-json, sometimes over 100%
faster when using bitsery).

### Compared to gRPC

While gRPC provides a very robust client/server design and some impressive tooling to create safe
interfaces across _many_ different programming languages, it can also be cumbersome to use and has
a pretty restrictive interface (mostly by design).

Unlike gRPC, `rpc.hpp` does not require external dependencies or building large libraries and does
not require an additional compilation step like gRPC does with its protocol buffers. It can also be
used with different message passing techniques, where gRPC is stuck with HTTP/2 requests.

For many, having the stability and proven performance (not to mention the support of Google) that
comes with a complete solution like gRPC may be ideal in a production setting. At this time,
`rpc.hpp` cannot compete with the development resources that gRPC has.

With that being said, the flexibility of `rpc.hpp` can be an advantage.
Because `rpc.hpp` does not provide a default server implementation, developers are free to create
their own robust system using other proven libraries and frameworks.
