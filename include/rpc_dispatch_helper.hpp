#pragma once

#define EXPAND(x) x

#define _FE_0(WHAT)
#define _FE_1(WHAT, X) WHAT(X)
#define _FE_2(WHAT, X, ...) EXPAND(WHAT(X)_FE_1(WHAT, __VA_ARGS__))
#define _FE_3(WHAT, X, ...) EXPAND(WHAT(X)_FE_2(WHAT, __VA_ARGS__))
#define _FE_4(WHAT, X, ...) EXPAND(WHAT(X)_FE_3(WHAT, __VA_ARGS__))
#define _FE_5(WHAT, X, ...) EXPAND(WHAT(X)_FE_4(WHAT, __VA_ARGS__))
#define _FE_6(WHAT, X, ...) EXPAND(WHAT(X)_FE_5(WHAT, __VA_ARGS__))
#define _FE_7(WHAT, X, ...) EXPAND(WHAT(X)_FE_6(WHAT, __VA_ARGS__))
#define _FE_8(WHAT, X, ...) EXPAND(WHAT(X)_FE_7(WHAT, __VA_ARGS__))
#define _FE_9(WHAT, X, ...) EXPAND(WHAT(X)_FE_8(WHAT, __VA_ARGS__))
#define _FE_10(WHAT, X, ...) EXPAND(WHAT(X)_FE_9(WHAT, __VA_ARGS__))
#define _FE_11(WHAT, X, ...) EXPAND(WHAT(X)_FE_10(WHAT, __VA_ARGS__))
#define _FE_12(WHAT, X, ...) EXPAND(WHAT(X)_FE_11(WHAT, __VA_ARGS__))
#define _FE_13(WHAT, X, ...) EXPAND(WHAT(X)_FE_12(WHAT, __VA_ARGS__))
#define _FE_14(WHAT, X, ...) EXPAND(WHAT(X)_FE_13(WHAT, __VA_ARGS__))
#define _FE_15(WHAT, X, ...) EXPAND(WHAT(X)_FE_14(WHAT, __VA_ARGS__))
#define _FE_16(WHAT, X, ...) EXPAND(WHAT(X)_FE_15(WHAT, __VA_ARGS__))
#define _FE_17(WHAT, X, ...) EXPAND(WHAT(X)_FE_16(WHAT, __VA_ARGS__))
#define _FE_18(WHAT, X, ...) EXPAND(WHAT(X)_FE_17(WHAT, __VA_ARGS__))
#define _FE_19(WHAT, X, ...) EXPAND(WHAT(X)_FE_18(WHAT, __VA_ARGS__))
#define _FE_20(WHAT, X, ...) EXPAND(WHAT(X)_FE_19(WHAT, __VA_ARGS__))

#define _FE2_0(WHAT)
#define _FE2_1(WHAT, X)
#define _FE2_2(WHAT, X, Y) WHAT(X, Y)
#define _FE2_3(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_2(WHAT, X, __VA_ARGS__))
#define _FE2_4(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_3(WHAT, X, __VA_ARGS__))
#define _FE2_5(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_4(WHAT, X, __VA_ARGS__))
#define _FE2_6(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_5(WHAT, X, __VA_ARGS__))
#define _FE2_7(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_6(WHAT, X, __VA_ARGS__))
#define _FE2_8(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_7(WHAT, X, __VA_ARGS__))
#define _FE2_9(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_8(WHAT, X, __VA_ARGS__))
#define _FE2_10(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_9(WHAT, X, __VA_ARGS__))
#define _FE2_11(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_10(WHAT, X, __VA_ARGS__))
#define _FE2_12(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_11(WHAT, X, __VA_ARGS__))
#define _FE2_13(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_12(WHAT, X, __VA_ARGS__))
#define _FE2_14(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_13(WHAT, X, __VA_ARGS__))
#define _FE2_15(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_14(WHAT, X, __VA_ARGS__))
#define _FE2_16(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_15(WHAT, X, __VA_ARGS__))
#define _FE2_17(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_16(WHAT, X, __VA_ARGS__))
#define _FE2_18(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_17(WHAT, X, __VA_ARGS__))
#define _FE2_19(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_18(WHAT, X, __VA_ARGS__))
#define _FE2_20(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_19(WHAT, X, __VA_ARGS__))

#define _RPC_GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, NAME, ...) NAME
#define RPC_FOR_EACH(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, _FE_20, _FE_19, _FE_18, _FE_17, _FE_16, _FE_15, _FE_14, _FE_13, _FE_12, _FE_11, _FE_10, _FE_9, _FE_8, _FE_7, _FE_6, _FE_5, _FE_4, _FE_3, _FE_2, _FE_1, _FE_0)(ACTION, __VA_ARGS__))
#define RPC_FOR_EACH2(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, _FE2_20, _FE2_19, _FE2_18, _FE2_17, _FE2_16, _FE2_15, _FE2_14, _FE2_13, _FE2_12, _FE2_11, _FE2_10, _FE2_9, _FE2_8, _FE2_7, _FE2_6, _FE2_5, _FE2_4, _FE2_3, _FE2_2, _FE2_1, _FE2_0)(ACTION, __VA_ARGS__))

#define RPC_ATTACH_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return rpc::run_callback(FUNCNAME, fc); }
#define RPC_ATTACH_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_FUNC, FUNCNAME, __VA_ARGS__))

#define RPC_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { return rpc::run_callback(FUNCNAME, fc); }
#define RPC_MULTI_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))

#define RPC_DEFAULT_DISPATCH(FUNCNAME, ...) EXPAND(template<typename Serial> rpc::func_result<Serial> rpc::dispatch(const func_call<Serial>& fc) { const auto func_name = fc.get_func_name(); RPC_ATTACH_FUNCS(FUNCNAME, __VA_ARGS__); throw std::runtime_error("RPC error: Called function: \"" + func_name + "\" not found!");})
