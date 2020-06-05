import sys

num_iters = 50
fname = "include/rpc_dispatch_helper.hpp"

if len(sys.argv) > 1:
    num_iters = int(sys.argv[1])
    if len(sys.argv) > 2:
        fname = str(sys.argv[2])

with open(fname, "w") as f:
    f.write("#pragma once\n\n")
    f.write("#define EXPAND(x) x\n")
    f.write("#define _FE_0(WHAT)\n")
    f.write("#define _FE_1(WHAT, X) WHAT(X)\n")

    if num_iters >= 2:
        for x in range(2, num_iters + 1):
            f.write(
                f"#define _FE_{x}(WHAT, X, ...) EXPAND(WHAT(X)_FE_{x - 1}(WHAT, __VA_ARGS__))\n"
            )

    f.write("\n#define _RPC_GET_MACRO(")

    for x in range(0, num_iters + 1):
        f.write(f"_{x}, ")

    f.write("NAME, ...) NAME\n")
    f.write("#define RPC_FOR_EACH(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, ")

    for x in reversed(range(1, num_iters + 1)):
        f.write(f"_FE_{x}, ")

    f.write("_FE_0)(ACTION, __VA_ARGS__))\n\n")
    f.write(
        "#define RPC_ATTACH_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return rpc::run_callback(FUNCNAME, fc); }\n"
    )
    f.write(
        "#define RPC_ATTACH_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_FUNC, FUNCNAME, __VA_ARGS__))\n\n"
    )
    f.write(
        "#define RPC_ALIAS_FUNC(FUNC_ALIAS, FUNCNAME) if (func_name == #FUNC_ALIAS) { return rpc::run_callback(FUNCNAME, fc); }\n\n"
    )
    f.write(
        '#define RPC_DEFAULT_DISPATCH(FUNCNAME, ...) EXPAND(template<typename Serial> rpc::func_result<Serial> rpc::dispatch(const func_call<Serial>& fc) { const auto func_name = fc.get_func_name(); RPC_ATTACH_FUNCS(FUNCNAME, __VA_ARGS__); throw std::runtime_error("RPC error: Called function: "" + func_name + "" not found!");})\n'
    )
