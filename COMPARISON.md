# Comparison of RPC Options

## rpc.hpp Adapter Comparison

For relative performance benchmarks for the adapters of `rpc.hpp` see [below](#performance)

## Alternatives for Consideration

### rpclib

### gRPC

### libjson-rpc-cpp

## Comparison to Alternatives

### Features

| feature | `rpc.hpp` | `rpclib` | `gRPC` |
|---------|-----------|----------|--------|
| Get result of remote function call | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Bind reference parameters of RPC | :heavy_check_mark: | :x: | :x: |
| Call RPCs without headers from the server | :heavy_check_mark: | :heavy_check_mark: | :x: |
| Doesn't require external compilation | :heavy_check_mark: | :heavy_check_mark: | :x: |
| No external dependencies (outside of networking) | :heavy_check_mark: | :heavy_check_mark: | :x: |
| Header-only | :heavy_check_mark: | :x: | :x: |
| Adaptable via "plugins" | :heavy_check_mark: | :x: | :x: |
| Compile-time type safety | :eight_spoked_asterisk: <br> if using shared headers <br> and `call_header_func` | :x: | :heavy_check_mark: |

### Build Data

| RPC Method | Server Size | Server LOC | Server Build Time |
|------------|-------------|------------|-------------------|
| `rpc.hpp` | X MB | X | X.X s |
| `rpclib` | X MB | X | X.X s |
| `gRPC` | X MB | X | X.X s |

| RPC Method | Client Size | Client LOC | Client Build Time |
|------------|-------------|------------|-------------------|
| `rpc.hpp` | `rpc.hpp` | X MB | X | X.X s |
| `rpclib` | `rpclib` | X MB | X | X.X s |
| `gRPC` | `gRPC` | X MB | X | X.X s |

### Performance

The following tests were performed using nanobench.
In each case, the call is done over a TCP socket on the same machine (interprocess call) to reduce
noise from network traffic.

Each "op" is a call to the defined function on the server.

#### Fibonacci

```C++
uint64_t Fibonacci(uint64_t n)
{
    return number < 2 ? 1 : Fibonacci(number - 1) + Fibonacci(number - 2);
}
```

| relative |               ns/op |                op/s |    err% |     total | RPC Method
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------
|   100.0% |          146,790.64 |            6,812.42 |    3.6% |      3.56 | `rpc.hpp (nlohmann_json)`
|   103.9% |          141,237.68 |            7,080.26 |    4.1% |      3.46 | `rpc.hpp (rapidjson)`
|   117.3% |          125,146.63 |            7,990.63 |    1.7% |      3.04 | `rpc.hpp (bitsery)`
|    63.0% |          232,848.79 |            4,294.63 |    2.9% |      5.67 | `rpclib`
|    71.2% |          206,190.91 |            4,849.87 |    3.7% |      5.22 | `gRPC`

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

| relative |               ns/op |                op/s |    err% |     total | RPC Method
|---------:|--------------------:|--------------------:|--------:|----------:|:-------------------
|   100.0% |          167,596.28 |            5,966.72 |    3.5% |      4.05 | `rpc.hpp (nlohmann_json)`
|   136.1% |          123,115.62 |            8,122.45 |    1.0% |      2.97 | `rpc.hpp (rapidjson)`
|   198.0% |           84,649.27 |           11,813.45 |    2.1% |      2.07 | `rpc.hpp (bitsery)`
|    84.1% |          199,231.66 |            5,019.28 |    2.6% |      5.36 | `rpclib`
|   110.5% |          151,613.72 |            6,595.71 |    1.0% |      3.68 | `gRPC`

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

| relative |               ns/op |                op/s |    err% |     total | RPC Method
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------
|   100.0% |          115,779.88 |            8,637.08 |    4.1% |      2.84 | `rpc.hpp (nlohmann_json)`
|   141.1% |           82,036.47 |           12,189.70 |    0.9% |      2.01 | `rpc.hpp (rapidjson)`
|   167.1% |           69,275.12 |           14,435.20 |    1.3% |      1.68 | `rpc.hpp (bitsery)`
|    64.6% |          179,154.34 |            5,581.78 |    1.6% |      4.34 | `rpclib`
|    82.9% |          139,585.65 |            7,164.06 |    0.9% |      3.37 | `gRPC`

#### AverageContainer

```C++
template<typename T>
double AverageContainer(const std::vector<T>& vec)
{
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.00);
    return sum / static_cast<double>(vec.size());
}
```

| relative |               ns/op |                op/s |    err% |     total | RPC Method
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------
|   100.0% |          108,568.81 |            9,210.75 |    1.0% |      2.65 | `rpc.hpp (nlohmann_json)`
|   133.3% |           81,460.05 |           12,275.96 |    3.6% |      1.99 | `rpc.hpp (rapidjson)`
|   175.5% |           61,878.97 |           16,160.58 |    1.4% |      1.50 | `rpc.hpp (bitsery)`
|    60.9% |          178,404.34 |            5,605.24 |    3.8% |      4.38 | `rpclib`
|    82.1% |          132,277.20 |            7,559.88 |    1.0% |      3.25 | `gRPC`

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

| relative |               ns/op |                op/s |    err% |     total | RPC Method
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------
|   100.0% |      550,886,800.00 |                1.82 |    2.7% |     18.62 | `rpc.hpp (nlohmann_json)`
|   106.8% |      515,726,266.67 |                1.94 |    2.1% |     17.57 | `rpc.hpp (rapidjson)`
|   109.6% |      502,559,966.67 |                1.99 |    2.6% |     17.28 | `rpc.hpp (bitsery)`
|    84.0% |      655,659,733.33 |                1.53 |    1.1% |     22.27 | `rpclib`
|    80.1% |      687,936,433.33 |                1.45 |    0.5% |     23.74 | `gRPC`
