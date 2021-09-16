# BSD 3-Clause License
#
# Copyright (c) 2020-2021, Jackson Harmer
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Allows for generating a larger or smaller rpc_dispatch_helper.hpp file as the max number of functions you can attach in one macro call must be known at compile-time."""

import sys

num_iters = 20
fname = "include/rpc_dispatch_helper.hpp"

f_header = """///@file rpc_dispatch_helper.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Helper macros for server dispatching of remote function calls
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2021, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

"""

if len(sys.argv) > 1:
    num_iters = int(sys.argv[1])
    if len(sys.argv) > 2:
        fname = str(sys.argv[2])

with open(fname, "w") as f:
    f.write(f_header)
    f.write("#pragma once\n\n")
    f.write(
        "#if !defined(RPC_HPP_DOXYGEN_GEN) && (defined(RPC_HPP_SERVER_IMPL) || defined(RPC_HPP_MODULE_IMPL))\n"
    )
    f.write("#define EXPAND(x) x\n\n")

    f.write("#define RPC_FE_0(WHAT)\n")
    f.write("#define RPC_FE_1(WHAT, X) WHAT(X)\n")

    if num_iters >= 2:
        for x in range(2, num_iters + 1):
            f.write(
                f"#define RPC_FE_{x}(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_{x - 1}(WHAT, __VA_ARGS__))\n"
            )

    f.write("\n#define RPC_FE2_0(WHAT)\n")
    f.write("#define RPC_FE2_1(WHAT, X)\n")
    f.write("#define RPC_FE2_2(WHAT, X, Y) WHAT(X, Y)\n")

    if num_iters >= 3:
        for x in range(3, num_iters + 1):
            f.write(
                f"#define RPC_FE2_{x}(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_{x - 1}(WHAT, X, __VA_ARGS__))\n"
            )

    f.write("\n#define RPC_GET_MACRO(")

    for x in range(0, num_iters + 1):
        f.write(f"_{x}, ")

    f.write("NAME, ...) NAME\n")

    f.write("#define RPC_FOR_EACH(ACTION, ...) EXPAND(RPC_GET_MACRO(_0, __VA_ARGS__, ")

    for x in reversed(range(1, num_iters + 1)):
        f.write(f"RPC_FE_{x}, ")

    f.write("RPC_FE_0)(ACTION, __VA_ARGS__))\n")

    f.write("#define RPC_FOR_EACH2(ACTION, ...) EXPAND(RPC_GET_MACRO(_0, __VA_ARGS__, ")

    for x in reversed(range(1, num_iters + 1)):
        f.write(f"RPC_FE2_{x}, ")

    f.write("RPC_FE2_0)(ACTION, __VA_ARGS__))\n")
    f.write("#endif\n\n")

    f.write("#if defined(RPC_HPP_SERVER_IMPL) || defined(RPC_HPP_MODULE_IMPL)\n")
    f.write(
        "#    define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) extern RETURN (*FUNCNAME)(__VA_ARGS__)\n"
    )
    f.write("#elif defined(RPC_HPP_CLIENT_IMPL)\n")
    f.write(
        "#    define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) RETURN (*FUNCNAME)(__VA_ARGS__) = nullptr"
    )
    f.write("#endif\n\n")

    f.write("#if defined(RPC_HPP_SERVER_IMPL) || defined(RPC_HPP_MODULE_IMPL)\n")
    f.write(
        "///@brief Attaches function (provided by FUNCNAME) to the server dispatch function\n"
    )
    f.write(
        "#define RPC_ATTACH_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return this->dispatch_func(FUNCNAME, serial_obj); }\n\n"
    )
    f.write("///@brief Attaches multiple functions to the server dispatch function\n")
    f.write(
        "#define RPC_ATTACH_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_FUNC, FUNCNAME, __VA_ARGS__))\n\n"
    )
    f.write(
        "///@brief Attaches function (provided by FUNCNAME) to the server dispatch funciton with server-side caching\n"
    )
    f.write(
        "#define RPC_ATTACH_CACHED_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return this->dispatch_cached_func(FUNCNAME, serial_obj); }\n\n"
    )
    f.write(
        "///@brief Attaches multiple functions to the server dispatch function with server-side caching\n"
    )
    f.write(
        "#define RPC_ATTACH_CACHED_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_CACHED_FUNC, FUNCNAME, __VA_ARGS__))\n\n"
    )
    f.write(
        "///@brief Attaches function (provided by FUNCNAME) to the server dispatch function with a different function name (provided by FUNC_ALIAS)\n"
    )
    f.write(
        "#define RPC_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { return this->dispatch_func(FUNCNAME, serial_obj); }\n\n"
    )
    f.write(
        "///@brief Attaches function (provided by FUNCNAME) to the server dispatch function with multiple different function names\n"
    )
    f.write(
        "#define RPC_MULTI_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))\n\n"
    )
    f.write(
        "///@brief Attaches function (provided by FUNCNAME) to the server dispatch function with a different function name (provided by FUNC_ALIAS) and server-side caching\n"
    )
    f.write(
        "#define RPC_ALIAS_CACHED_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { return this->dispatch_cached_func(FUNCNAME, serial_obj); }\n\n"
    )
    f.write(
        "///@brief Attaches function (provided by FUNCNAME) to the server dispatch function with multiple different function names and server-side caching\n"
    )
    f.write(
        "#define RPC_MULTI_ALIAS_CACHED_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_CACHED_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))\n\n"
    )
    f.write(
        "///@brief Implements the @ref rpc::server::dispatch_impl function for you and attaches the listed functions\n"
    )
    f.write(
        '#define RPC_DEFAULT_DISPATCH(FUNCNAME, ...) EXPAND(const auto func_name = rpc::pack_adapter<adapter_t>::get_func_name(serial_obj); RPC_ATTACH_FUNCS(FUNCNAME, __VA_ARGS__) throw std::runtime_error("RPC error: Called function: "" + func_name + "" not found!");)\n'
    )
    f.write("#endif")
