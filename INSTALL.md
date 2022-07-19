# Installation Guide

## Header Installation

As `rpc.hpp` is a header-only library, you only need to install the header files to your system.

This can be done by cloning the project's `main` branch or downloading it as an archive from GitHub.
Once you have the files, you may copy the contents of the `include` directory to your system or
user's include path, or to the relevant place in your project's directory.

## Installing With CMake

You may also install the headers by cloning the repository and running the
following commands (this will only install nlohmann/json adapter
see [Build Options](#build-options) on how to enable more):

```shell
$ cmake -B build -DBUILD_ADAPTER_NJSON=ON
```

```shell
$ cmake --install build
```

CMake version 3.16 or newer is required for this method, see below for instructions to install CMake
if it is not already installed.

## Building and Testing the Project

If you are looking to contribute to `rpc.hpp`, or just want to run the tests/benchmarks, you will
need to make sure that you can build the project and its test suite.

### Setting Up Your Environment

While `rpc.hpp` is a header-only library, the project relies on using a buildsystem to
perform certain tasks.
It is therefore required that CMake (version 3.16 or newer) is installed on your machine and
available on your `$PATH` to build the project.

Additionally, `rpc.hpp` requires at least one C++ compiler that fully supports C++17 or newer:

**Linux/macOS/BSD**

- `gcc` >= 7
- `clang` >= 5

**Windows**

- MSVC (`cl.exe`) >= 19.14 (Visual Studio 2017 15.7 or newer)

On Windows you will have to install Visual Studio 2017 or newer.
You may install the full IDE with the C++ "Workload" selected or simply install the
[standalone build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019)
instead.

These development dependencies can be installed in the following ways, depending on your operating
system (replace `gcc` w/ `clang` if desired):

**Arch / Manjaro**

```shell
$ sudo pacman -S gcc cmake ninja
```

**CentOS 7 / RHEL 7**

```shell
$ sudo dnf install devtoolset-10-gcc cmake ninja-build
```

**CentOS 8+ / Fedora 33+ / RHEL 8+**

```shell
$ sudo dnf install gcc cmake ninja-build
```

**Debian 10+ / PopOS/Ubuntu 18.04+**

```shell
$ sudo apt install gcc cmake ninja-build
```

**FreeBSD 12+**

```shell
# pkg install cmake ninja
```

**OpenSUSE / SLES**

```shell
$ sudo zypper install gcc cmake ninja
```

**Windows - Chocolatey**

```powershell
> choco install cmake ninja
```

**Windows - Scoop**

```powershell
> scoop install cmake ninja
```

#### Library Dependencies

While not strictly _required_, the adapter headers are considered a core part
of the library and therefore should build without issue after any code change,
and should be tested.

(NOTE: if the community ends up creating a lot of adapters, this may change)

---

The libraries needed to build the current adapters are:

- [nlohmann/json](https://github.com/nlohmann/json) >= 3.9.0
- [rapidjson](https://github.com/tencent/rapidjson) >= 1.1.0
- [Boost.JSON](https://www.boost.org/doc/libs/1_77_0/libs/json/doc/html/index.html) >= 1.75.0

For building the unit tests, there are additional libraries required:

- [asio](http://think-async.com/Asio) >= 1.14
- [doctest](https://github.com/onqtam/doctest) >= 2.4

Building the benchmarks also requires one library:

- [nanobench](https://github.com/martinus/nanobench) >= 4.3.0

These dependencies are installed via vcpkg (a submodule of this project)

##### Setting up `vcpkg`

1. Make sure the submodule is initialized

From the project root:

```shell
$ git submodule update --init
```

2. "Boostrap" `vcpkg`

**Linux/macOS/BSD**

```shell
$ vcpkg/bootstrap-vcpkg.sh
```

**Windows**

```powershell
> vcpkg\bootstrap-vcpkg.bat
```

### Building the Project

1. Enter the top-level directory of the project
2. Configure the project

```shell
$ cmake -B build -G Ninja -D BUILD_ADAPTER_BITSERY -D BUILD_ADAPTER_BOOST_JSON=ON -D BUILD_ADAPTER_NJSON=ON -D BUILD_ADAPTER_RAPIDJSON=ON -D BUILD_TESTING=ON
```

NOTE: The above command can be altered based on your needs:

- In this example, the "Ninja" generator is used as it is fast, but `-G Ninja` may be omitted to
use the system default make system
- With this example, all adapters are to be built. By omitting one or more, they will not be
built (example: `-D BUILD_ADAPTER_BOOST_JSON=ON` could be omitted or set to `=OFF`)
  - **NOTE:** `BUILD_ADAPTER_NJSON` is required for testing
- See [Build Options](#build-options) for more details on these and other configuration options

3. Build the project

```shell
$ cmake --build build
```

#### Build Options

| Option | Description |
|--|--|
| `BENCH_GRPC` | Build gRPC server and client for benchmark comparison |
| `BENCH_RPCLIB` | Build rpclib server and client for benchmark comparison |
| `BUILD_ADAPTER_BITSERY` | Build the adapter for Bitsery |
| `BUILD_ADAPTER_BOOST_JSON` | Build the adapter for Boost.JSON |
| `BUILD_ADAPTER_NJSON` | Build the adapter for nlohmann/json (`ON` by default) |
| `BUILD_ADAPTER_RAPIDJSON` | Build the adapter for rapidjson |
| `BUILD_BENCHMARK` | Build the benchmarking suite |
| `BUILD_EXAMPLES` | Build the examples |
| `BUILD_TESTING` | Build the testing tree (`ON` by default) |
| `CODE_COVERAGE` | Enable coverage reporting |
| `GENERATE_DOXYGEN` | Generate Doxygen documentation from comments |

NOTE:

- `BUILD_BENCHMARK` requires `BUILD_TESTING`
- `BENCH_GRPC` and `BENCH_RPCLIB` require `BUILD_BENCHMARK`
- `GENERATE_DOXYGEN` and `CODE_COVERAGE` should only be defined when doing a documentation or
coverage build, respectively

### Running Tests

To build the tests, make sure your build environment is set up. Also make sure that the
`BUILD_TESTING` option is set in your `CMakeCache.txt`.

**NOTE:** The test server will have to be running for the tests to succeed. You could run it in a
separate terminal session, or add `tests/test_server & ` at the beginning of the `ctest` command
below. The tests will successfully kill the server on completion.

From the project _build_ directory run:

```shell
$ ctest --test-dir tests
```
