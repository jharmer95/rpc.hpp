import sys

num_iters = 20
fname = "include/rpc_dispatch_helper.hpp"

if len(sys.argv) > 1:
    num_iters = int(sys.argv[1])
    if len(sys.argv) > 2:
        fname = str(sys.argv[2])

with open(fname, "w") as f:
    f.write("#pragma once\n\n")
    f.write("#define EXPAND(x) x\n\n")

    f.write("#define _FE_0(WHAT)\n")
    f.write("#define _FE_1(WHAT, X) WHAT(X)\n")

    if num_iters >= 2:
        for x in range(2, num_iters + 1):
            f.write(
                f"#define _FE_{x}(WHAT, X, ...) EXPAND(WHAT(X)_FE_{x - 1}(WHAT, __VA_ARGS__))\n"
            )

    f.write("\n#define _FE2_0(WHAT)\n")
    f.write("#define _FE2_1(WHAT, X)\n")
    f.write("#define _FE2_2(WHAT, X, Y) WHAT(X, Y)\n")

    if num_iters >= 3:
        for x in range(3, num_iters + 1):
            f.write(
                f"#define _FE2_{x}(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_{x - 1}(WHAT, X, __VA_ARGS__))\n"
            )

    f.write("\n#define _RPC_GET_MACRO(")

    for x in range(0, num_iters + 1):
        f.write(f"_{x}, ")

    f.write("NAME, ...) NAME\n")

    f.write("#define RPC_FOR_EACH(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, ")

    for x in reversed(range(1, num_iters + 1)):
        f.write(f"_FE_{x}, ")

    f.write("_FE_0)(ACTION, __VA_ARGS__))\n")

    f.write(
        "#define RPC_FOR_EACH2(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, "
    )

    for x in reversed(range(1, num_iters + 1)):
        f.write(f"_FE2_{x}, ")

    f.write("_FE2_0)(ACTION, __VA_ARGS__))\n\n")

    f.write(
        "#define RPC_ATTACH_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { auto pack = create_func(FUNCNAME, serial_obj); run_callback(FUNCNAME, pack); serial_obj = serial_adapter<Serial>::from_packed_func(pack); return; }\n"
    )
    f.write(
        "#define RPC_ATTACH_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_FUNC, FUNCNAME, __VA_ARGS__))\n\n"
    )
    f.write(
        "#define RPC_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { auto pack = create_func(FUNCNAME, serial_obj); run_callback(FUNCNAME, pack); serial_obj = serial_adapter<Serial>::from_packed_func(pack); return; }\n"
    )
    f.write(
        "#define RPC_MULTI_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))\n\n"
    )
    f.write(
        '#define RPC_DEFAULT_DISPATCH(FUNCNAME, ...) EXPAND(template<typename Serial> void rpc::server::dispatch(Serial& serial_obj) { const auto func_name = serial_adapter<Serial>::extract_func_name(serial_obj); RPC_ATTACH_FUNCS(FUNCNAME, __VA_ARGS__); throw std::runtime_error("RPC error: Called function: \"" + func_name + "\" not found!");})\n'
    )
