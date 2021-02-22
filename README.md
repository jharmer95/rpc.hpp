![cmake](https://github.com/jharmer95/rpc.hpp/workflows/cmake/badge.svg?branch=main&event=push) [![codecov](https://codecov.io/gh/jharmer95/rpc.hpp/branch/main/graph/badge.svg)](https://codecov.io/gh/jharmer95/rpc.hpp)

# rpc.hpp

A simple header-only library for supporting remote procedure calls using a variety of extensible features

## Features

- Cross-platform
    - Supports every major compiler/operating system
- Modern
    - Utilizes features like C++17's [`constexpr if`](https://en.cppreference.com/w/cpp/language/if)
- Type safe support for various types
    - Supports most built-in/STL types out of the box
    - Can add your own "extensions" to allow serialization and sending of any custom type
- Easy to use

### Upcoming Features

- Utilizing C++20's concepts/`requires` statement will make the library a lot simpler, cleaner, and easy to read.
- Better error/exception handling

## Documentation

See Doxygen docs [here](https://jharmer95.github.io/rpc.hpp/)
