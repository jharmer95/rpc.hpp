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
    - Can be used on your server in 4 simple steps (see [EXAMPLES](EXAMPLES.md) for details):
        1. `#include rpc.hpp`
        2. Implement the `rpc::dispatch` function
        3. Add `Serialize()` and `DeSerialize()` functions to your classes or provide template specializations for the `rpc::Serialize<T>()` and `rpc::DeSerialize<T>()` functions
        4. Call `rpc::RunFromJSON()` from your code
    - Does all of the parameter parsing for you

### Upcoming Features

- Currently, only JSON serialization using [nlohmann/json](https://github.com/nlohmann/json) is supported
    - Would like to be able to add support for other popular serialization techniques (XML, ProtoBuf) and set this via template. So instead of `Serialize()` and `RunFromJSON()`, we would have: `Serialize<SerialType::JSON>()` or `RunRPC<SerialType::XML>()`
- Would also like to provide an interface for users to provide their own serialization API so that you could use something like [cereal](https://github.com/USCiLab/cereal), some corporate library, or whatever you preferred.
- Utilizing C++20's concepts/`requires` statement will make the library a lot simpler, cleaner, and easy to read.
- Better error/exception handling
