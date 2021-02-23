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
#define _FE_21(WHAT, X, ...) EXPAND(WHAT(X)_FE_20(WHAT, __VA_ARGS__))
#define _FE_22(WHAT, X, ...) EXPAND(WHAT(X)_FE_21(WHAT, __VA_ARGS__))
#define _FE_23(WHAT, X, ...) EXPAND(WHAT(X)_FE_22(WHAT, __VA_ARGS__))
#define _FE_24(WHAT, X, ...) EXPAND(WHAT(X)_FE_23(WHAT, __VA_ARGS__))
#define _FE_25(WHAT, X, ...) EXPAND(WHAT(X)_FE_24(WHAT, __VA_ARGS__))
#define _FE_26(WHAT, X, ...) EXPAND(WHAT(X)_FE_25(WHAT, __VA_ARGS__))
#define _FE_27(WHAT, X, ...) EXPAND(WHAT(X)_FE_26(WHAT, __VA_ARGS__))
#define _FE_28(WHAT, X, ...) EXPAND(WHAT(X)_FE_27(WHAT, __VA_ARGS__))
#define _FE_29(WHAT, X, ...) EXPAND(WHAT(X)_FE_28(WHAT, __VA_ARGS__))
#define _FE_30(WHAT, X, ...) EXPAND(WHAT(X)_FE_29(WHAT, __VA_ARGS__))

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
#define _FE2_21(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_20(WHAT, X, __VA_ARGS__))
#define _FE2_22(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_21(WHAT, X, __VA_ARGS__))
#define _FE2_23(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_22(WHAT, X, __VA_ARGS__))
#define _FE2_24(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_23(WHAT, X, __VA_ARGS__))
#define _FE2_25(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_24(WHAT, X, __VA_ARGS__))
#define _FE2_26(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_25(WHAT, X, __VA_ARGS__))
#define _FE2_27(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_26(WHAT, X, __VA_ARGS__))
#define _FE2_28(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_27(WHAT, X, __VA_ARGS__))
#define _FE2_29(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_28(WHAT, X, __VA_ARGS__))
#define _FE2_30(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)_FE2_29(WHAT, X, __VA_ARGS__))

#define _RPC_GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, NAME, ...) NAME
#define RPC_FOR_EACH(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, _FE_30, _FE_29, _FE_28, _FE_27, _FE_26, _FE_25, _FE_24, _FE_23, _FE_22, _FE_21, _FE_20, _FE_19, _FE_18, _FE_17, _FE_16, _FE_15, _FE_14, _FE_13, _FE_12, _FE_11, _FE_10, _FE_9, _FE_8, _FE_7, _FE_6, _FE_5, _FE_4, _FE_3, _FE_2, _FE_1, _FE_0)(ACTION, __VA_ARGS__))
#define RPC_FOR_EACH2(ACTION, ...) EXPAND(_RPC_GET_MACRO(_0, __VA_ARGS__, _FE2_30, _FE2_29, _FE2_28, _FE2_27, _FE2_26, _FE2_25, _FE2_24, _FE2_23, _FE2_22, _FE2_21, _FE2_20, _FE2_19, _FE2_18, _FE2_17, _FE2_16, _FE2_15, _FE2_14, _FE2_13, _FE2_12, _FE2_11, _FE2_10, _FE2_9, _FE2_8, _FE2_7, _FE2_6, _FE2_5, _FE2_4, _FE2_3, _FE2_2, _FE2_1, _FE2_0)(ACTION, __VA_ARGS__))

#define RPC_ATTACH_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return dispatch_func<Serial>(FUNCNAME, serial_obj); }
#define RPC_ATTACH_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_FUNC, FUNCNAME, __VA_ARGS__))

#define RPC_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { return dispatch_func<Serial>(FUNCNAME, serial_obj); }
#define RPC_MULTI_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))

#define RPC_DEFAULT_DISPATCH(FUNCNAME, ...) EXPAND(template<typename Serial> void rpc::server::dispatch(typename Serial::doc_type& serial_obj) { const auto func_name = serial_adapter<Serial>::extract_func_name(serial_obj); RPC_ATTACH_FUNCS(FUNCNAME, __VA_ARGS__) throw std::runtime_error("RPC error: Called function: \"" + func_name + "\" not found!");})
