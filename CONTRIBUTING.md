# How to Contribute

## Code of Conduct

This project and everyone participating in it is governed by the [rpc.hpp Code of Conduct](CODE_OF_CONDUCT.md).
By participating, you are expected to uphold this code.\
Please report unacceptable behavior to jharmer95@gmail.com.

## Reporting an Issue

<!-- TODO -->

### Feature Requests

<!-- TODO -->

### Bug Reports

<!-- TODO -->

### Other Issues

<!-- TODO -->

## Contributing Code

<!-- TODO -->

### Setting Up Your Environment

While `rpc.hpp` is a header-only library, the project relies on using a buildsystem to perform certain tasks.
It is therefore required that CMake (version 3.12 or newer) is installed on your machine and available on your `$PATH` to build the project.

Additionally, `rpc.hpp` requires at least one C++ compiler that fully supports C++17 or newer:

**Linux/macOS/BSD**

- `gcc` >= 7
- `clang` >= 5

**Windows**

- MSVC (`cl.exe`) >= 19.14 (Visual Studio 2017 15.7 or newer)

On Windows you will have to install Visual Studio 2017 or newer.
You may install the full IDE with the C++ "Workload" selected or simply install the [standalone build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) instead.

These development dependencies can be installed in the following ways, depending on your operating system (replace `gcc` w/ `clang` if desired):

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

- [catch2](https://github.com/catchorg/Catch2) >= 2.11 < 3.0

These dependencies can be installed in one of three ways:

1. Installing manually from source, binary, or system package manager
2. Install via [vcpkg](https://github.com/Microsoft/vcpkg)
3. Auto-install via [Conan](https://conan.io)

##### Setting up `vcpkg`

1. In a terminal, navigate to the location where `vcpkg` should be installed (`$HOME` is fine for most)
2. Clone the `vcpkg` project and enter the source directory

```shell
$ git clone https://github.com/Microsoft/vcpkg
```

```shell
$ cd vcpkg
```

3. "Bootstrap" `vcpkg`

**Linux/macOS/BSD**

```shell
$ ./bootstrap-vcpkg.sh
```

**Windows**

```powershell
> .\bootstrap-vcpkg.bat
```

4. Add `vcpkg` to your path (optional)
5. Integrate your vcpkg installation into CMake/Visual Studio

```shell
$ vcpkg integrate install
```

6. Install dependencies

```shell
$ vcpkg install catch2 doctest nlohmann-json rapidjson boost-json
```

##### Setting up Conan

1. Make sure you have Python 3 and pip installed.
  - It is recommended to install and use `pipx` rather than `pip` for installing applications like Conan, however
2. Install Conan

  - With pipx (preferred)

  ```shell
  $ pipx install conan
  ```

  - With pip

  ```shell
  $ pip install --user conan
  ```

### Building the Project

1. Enter the top-level directory of the project
2. Create the build directory

```shell
$ mkdir build
```

3. Configure the project

```shell
$ cmake -B build -G Ninja -D BUILD_ADAPTER_BOOST_JSON -D BUILD_ADAPTER_NJSON -D BUILD_ADAPTER_RAPIDJSON -D BUILD_TESTING
```

NOTE: The above command can be altered based on your needs:

- In this example, the "Ninja" generator is used as it is fast, but `-G Ninja` may be omitted to use the system default make system
- With this example, all three adapters are to be built. By omitting one or more, they will not be built (example: `-D BUILD_ADAPTER_BOOST_JSON` could be omitted)
  - `BUILD_ADAPTER_NJSON` is required for testing
- If using Conan to auto-install dependencies, make sure to set its option (add `-D DEPENDS_CONAN`)
- If using vcpkg to manage dependencies, make sure to set its option (add `-D DEPENDS_VCPKG`)
  - You may also need to provide CMake with a vcpkg toolchain file: (`-D CMAKE_TOOLCHAIN_FILE="/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"`)
- See [Build Options](#build-options) for more details on these and other configuration options

4. Build the project

```shell
$ cmake --build build
```

#### Build Options

| Option | Description |
|--|--|
| `BUILD_ADAPTER_BOOST_JSON` | Build the adapter for Boost.JSON |
| `BUILD_ADAPTER_NJSON` | Build the adapter for nlohmann/json |
| `BUILD_ADAPTER_RAPIDJSON` | Build the adapter for rapidjson |
| `BUILD_BENCHMARK` | Build the benchmarking suite |
| `BUILD_EXAMPLES` | Build the examples |
| `BUILD_TESTING` | Build the testing tree |
| `CODE_COVERAGE` | Enable coverage reporting |
| `DEPENDS_CONAN` | Use Conan to manage C/C++ dependencies |
| `DEPENDS_VCPKG` | User vcpkg to manage C/C++ dependencies |
| `GENERATE_DOXYGEN` | Generate Doxygen documentation from comments |

NOTE:

- Only one or neither of `DEPENDS_CONAN` and `DEPENDS_VCPKG` may be defined
- `BUILD_BENCHMARK` requires `BUILD_TESTING`
- `GENERATE_DOXYGEN` and `CODE_COVERAGE` should only be defined when doing a documentation or coverage build, respectively

### Code Conventions

<!-- TODO -->

#### Code Style

<!-- TODO -->

### Running Tests

<!-- TODO -->

## Contributing Documentation

<!-- TODO -->

### Documentation Style

<!-- TODO -->

## Committing Changes

<!-- TODO -->

## Submitting Pull Requests

<!-- TODO -->
