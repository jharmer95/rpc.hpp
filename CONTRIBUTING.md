# How to Contribute

## Code of Conduct

This project and everyone participating in it is governed by the
[rpc.hpp Code of Conduct](CODE_OF_CONDUCT.md).
By participating, you are expected to uphold this code.\
Please report unacceptable behavior to jharmer95@gmail.com.

## Reporting an Issue

You may report an issue from the [issues](https://github.com/jharmer95/rpc.hpp/issues) page.

Before submitting a new issue, please go through the following steps:

1. Check if you are running the latest version
  - The easiest way to do this is to open the file `rcp.hpp` on your system
  - The version number will be in a comment near the top of the file
  - Compare that to the [latest release](https://github.com/jharmer95/rpc.hpp/releases/latest)
  - If your version is out of date, please update to the newest version before reporting an issue
2. Perform a search to see if a similar issue has already been opened
  - If your issue is the same, give that issue a :+1:
    - **Please don't add additional comments like _"Me too"_ or _"I need this fix"_**,
    a :+1: conveys this much more succinctly and allows for an easy way to visualize the
    popularity/demand for this issue.
  - Even if your issue is not _exactly_ the same, it may be more beneficial to provide some extra
    context or details for your situation via a comment vs. a new issue.
    - Sometimes it easier to understand the scope of an issue this way and can allow for one patch
    to be made instead of multiple

Please stick to using one of the provided templates to ensure that issues are consistent.

### Feature Requests

Using the Feature Request template, make sure to replace the comments with the indicated content.
Hopefully, this can be replaced by GitHub Issue Forms soon.

An example feature request might look like:

```markdown
# Feature Request: Add Support For SomeSerialLibrary

## Brief Summary of Request
<!-- insert a brief summary of your feature request here -->
Add adapter to support for serialization using
[SomeSerialLibrary](https://github.com/fake-author/some-serial-lib)

## What Is The Value Added With This Feature
<!-- provide some reasons why you believe this feature is valuable -->
SomeSerialLibrary is a really powerful serialization library and an adapter for
it would be of great benefit to the community.

Some features of the library are:

- Feature A
- Feature B

## Possible Implementation
<!-- if you have some ideas on how this can actually be done put them here -->
- Create an additional header: `rpc_adatpers/rpc_some_serial_lib.hpp`

## Desired Outcome
<!-- describe the outcome you'd like to see here as well as provide some
     code/pseudocode to indicate the usage or API of your request -->

    ```C++
    class MyClient : public rpc::client_interface<some_serial_lib_adapter>
    {
    };

    // ...

    auto result = MyClient.call_func("SomeFunc");
    ```

## Other Details
<!-- feel free to put any other details here -->
```

### Bug Reports

Using the Bug Report template, make sure to replace the comments with the indicated content.
Hopefully, this can be replaced by GitHub Issue Forms soon.

> **NOTE:** If you believe that an issue is most likely a security concern/vulnerability,
> **DO NOT** file a bug report.
>
> Please see the [security policy](SECURITY.md) on how to report this!

An example bug report might look like:

```markdown
# Bug Report: rapidjson adapter fails to compile with Clang 12

## System Information

- Platform: x86_64
- Operating System: Ubuntu 21.10
- Compiler: clang 12.0.1

## Brief Summary of Bug
<!-- here you can provide a brief summary of the bug -->
When compiling with Clang 12.0.1, the error:
"This fake error occurred (rpc_adapters/rpc_rapidjson.hpp:80)" is given.

- Issue does not exist in gcc.

## Steps To Reproduce
<!-- please list the **exact** steps taken to encounter this bug or the cases
     when it occurs -->
Using example code, compile with Clang 12 or newer with `-D RPC_HPP_ENABLE_RAPIDJSON`

## Areas Of Concern
<!-- what parts of the project are impacted by this bug, everything? and
     experimental feature? only a few functions? -->
- Prevents using rapidjson with the newer versions of Clang.
- Requires switching to gcc or an older version of Clang.

## Estimated Severity
<!-- Please "check" one of the boxes by replacing the blank space in the square
     brackets with an X -->

- [ ] Pedantic (code-style, design)
- [ ] Minimal (compiler warnings, legacy support)
- [ ] Minor (incompatibilities, problems that have easy work-arounds)
- [X] Major (breaking changes, problems with more advanced work-arounds)
- [ ] Critical (major security issues, complete failure)

## Possible Fixes
<!-- if you know of a way that could possibly fix this issue, list it here,
     feel free to fork and submit a PR once this bug report is submitted -->
```

## Contributing Code

`rpc.hpp` welcomes code contributions from anyone of any skill level or
background.

Contributors must agree to the (code of conduct)[CODE_OF_CONDUCT.md] and
additionally, should follow the guidelines provided in this section to ensure
their changes are reviewed and accepted.

### Setting Up Your Environment

While `rpc.hpp` is a header-only library, the project relies on using a buildsystem to
perform certain tasks.
It is therefore required that CMake (version 3.12 or newer) is installed on your machine and
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

- [catch2](https://github.com/catchorg/Catch2) >= 2.11 < 3.0

These dependencies can be installed in one of three ways:

1. Installing manually from source, binary, or system package manager
2. Install via [vcpkg](https://github.com/Microsoft/vcpkg)
3. Auto-install via [Conan](https://conan.io)

##### Setting up `vcpkg`

1. In a terminal, navigate to the location where `vcpkg` should be installed
(`$HOME` is fine for most)
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
  - It is recommended to install and use `pipx` rather than `pip` for installing applications like
  Conan, however
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

- In this example, the "Ninja" generator is used as it is fast, but `-G Ninja` may be omitted to
use the system default make system
- With this example, all three adapters are to be built. By omitting one or more, they will not be
built (example: `-D BUILD_ADAPTER_BOOST_JSON` could be omitted)
  - `BUILD_ADAPTER_NJSON` is required for testing
- If using Conan to auto-install dependencies, make sure to set its option (add `-D DEPENDS_CONAN`)
- If using vcpkg to manage dependencies, make sure to set its option (add `-D DEPENDS_VCPKG`)
  - You may also need to provide CMake with a vcpkg toolchain file:
  (`-D CMAKE_TOOLCHAIN_FILE="/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"`)
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
- `GENERATE_DOXYGEN` and `CODE_COVERAGE` should only be defined when doing a documentation or
coverage build, respectively

### Code Conventions

`rpc.hpp` aims to maintain a clean, easy to read, and idiomatic codebase. It does this by following
best practices such as those provided by:

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- Jason Turner's [cppbestpractices](https://github.com/lefticus/cppbestpractices)

#### Code Style

To simplify keeping a consistent code style, a `.clang-format` file is provided. Please make sure
you have clang-format installed and run it on any changed C++ files (`.h`, `.hpp`, `.cpp`, etc.)
before submitting a PR.

In addition to this, there are a few naming conventions to stick to:

##### Naming Variables

The naming convention is very similar to that of the C++ standard library:

- All variables should be meaningfully named unless they are trivial such as:
  - A loop index (`i`, `j`, `k`)
  - A loop iterator (`it`, `it2`, etc.)
  - A temporary variable being consumed within the next few lines, like for a swap (`tmp`)
  - An unrestricted generic template typename (`T`, `U`)
- Variables do not need to indicate their type
  - No hungarian notation (**no** `iSomeVar`, `fSomeVar`)
  - No type prefixes/suffixes (**no** `length_int`, `string_name`)
  - (Smart) pointers/references _may_ include a `ptr_`/`ref_` prefix when needed for clarity
- All variable/function/parameter/class/struct/namespace/member names should be `snake_case` with a
few exceptions:
  - `SCREAMING_SNAKE_CASE`:
    - Macros
    - Global variables
    - `constexpr` variables
  - `CamelCase`
    - `template` parameters
- `private`/`protected` non-static member variables should have a `m_` prefix
- `private`/`protected` static member variables should have a `s_` prefix
- Type aliases should have a `_t` suffix
- Boolean variables should indicate what they are testing, (ex. `is_full`, `has_value`, `was_used`)
  - Booleans should only be named in the _affirmative_ (**no** `is_not_empty`, `no_value`)

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
