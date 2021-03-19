#pragma once

#define EXPAND(x) x

#define RPC_FE_0(WHAT)
#define RPC_FE_1(WHAT, X) WHAT(X)
#define RPC_FE_2(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_1(WHAT, __VA_ARGS__))
#define RPC_FE_3(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_2(WHAT, __VA_ARGS__))
#define RPC_FE_4(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_3(WHAT, __VA_ARGS__))
#define RPC_FE_5(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_4(WHAT, __VA_ARGS__))
#define RPC_FE_6(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_5(WHAT, __VA_ARGS__))
#define RPC_FE_7(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_6(WHAT, __VA_ARGS__))
#define RPC_FE_8(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_7(WHAT, __VA_ARGS__))
#define RPC_FE_9(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_8(WHAT, __VA_ARGS__))
#define RPC_FE_10(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_9(WHAT, __VA_ARGS__))
#define RPC_FE_11(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_10(WHAT, __VA_ARGS__))
#define RPC_FE_12(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_11(WHAT, __VA_ARGS__))
#define RPC_FE_13(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_12(WHAT, __VA_ARGS__))
#define RPC_FE_14(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_13(WHAT, __VA_ARGS__))
#define RPC_FE_15(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_14(WHAT, __VA_ARGS__))
#define RPC_FE_16(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_15(WHAT, __VA_ARGS__))
#define RPC_FE_17(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_16(WHAT, __VA_ARGS__))
#define RPC_FE_18(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_17(WHAT, __VA_ARGS__))
#define RPC_FE_19(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_18(WHAT, __VA_ARGS__))
#define RPC_FE_20(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_19(WHAT, __VA_ARGS__))
#define RPC_FE_21(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_20(WHAT, __VA_ARGS__))
#define RPC_FE_22(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_21(WHAT, __VA_ARGS__))
#define RPC_FE_23(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_22(WHAT, __VA_ARGS__))
#define RPC_FE_24(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_23(WHAT, __VA_ARGS__))
#define RPC_FE_25(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_24(WHAT, __VA_ARGS__))
#define RPC_FE_26(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_25(WHAT, __VA_ARGS__))
#define RPC_FE_27(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_26(WHAT, __VA_ARGS__))
#define RPC_FE_28(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_27(WHAT, __VA_ARGS__))
#define RPC_FE_29(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_28(WHAT, __VA_ARGS__))
#define RPC_FE_30(WHAT, X, ...) EXPAND(WHAT(X)RPC_FE_29(WHAT, __VA_ARGS__))

#define RPC_FE2_0(WHAT)
#define RPC_FE2_1(WHAT, X)
#define RPC_FE2_2(WHAT, X, Y) WHAT(X, Y)
#define RPC_FE2_3(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_2(WHAT, X, __VA_ARGS__))
#define RPC_FE2_4(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_3(WHAT, X, __VA_ARGS__))
#define RPC_FE2_5(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_4(WHAT, X, __VA_ARGS__))
#define RPC_FE2_6(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_5(WHAT, X, __VA_ARGS__))
#define RPC_FE2_7(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_6(WHAT, X, __VA_ARGS__))
#define RPC_FE2_8(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_7(WHAT, X, __VA_ARGS__))
#define RPC_FE2_9(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_8(WHAT, X, __VA_ARGS__))
#define RPC_FE2_10(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_9(WHAT, X, __VA_ARGS__))
#define RPC_FE2_11(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_10(WHAT, X, __VA_ARGS__))
#define RPC_FE2_12(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_11(WHAT, X, __VA_ARGS__))
#define RPC_FE2_13(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_12(WHAT, X, __VA_ARGS__))
#define RPC_FE2_14(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_13(WHAT, X, __VA_ARGS__))
#define RPC_FE2_15(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_14(WHAT, X, __VA_ARGS__))
#define RPC_FE2_16(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_15(WHAT, X, __VA_ARGS__))
#define RPC_FE2_17(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_16(WHAT, X, __VA_ARGS__))
#define RPC_FE2_18(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_17(WHAT, X, __VA_ARGS__))
#define RPC_FE2_19(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_18(WHAT, X, __VA_ARGS__))
#define RPC_FE2_20(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_19(WHAT, X, __VA_ARGS__))
#define RPC_FE2_21(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_20(WHAT, X, __VA_ARGS__))
#define RPC_FE2_22(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_21(WHAT, X, __VA_ARGS__))
#define RPC_FE2_23(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_22(WHAT, X, __VA_ARGS__))
#define RPC_FE2_24(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_23(WHAT, X, __VA_ARGS__))
#define RPC_FE2_25(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_24(WHAT, X, __VA_ARGS__))
#define RPC_FE2_26(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_25(WHAT, X, __VA_ARGS__))
#define RPC_FE2_27(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_26(WHAT, X, __VA_ARGS__))
#define RPC_FE2_28(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_27(WHAT, X, __VA_ARGS__))
#define RPC_FE2_29(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_28(WHAT, X, __VA_ARGS__))
#define RPC_FE2_30(WHAT, X, Y, ...) EXPAND(WHAT(X, Y)RPC_FE2_29(WHAT, X, __VA_ARGS__))

#define RPC_GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, NAME, ...) NAME
#define RPC_FOR_EACH(ACTION, ...) EXPAND(RPC_GET_MACRO(_0, __VA_ARGS__, RPC_FE_30, RPC_FE_29, RPC_FE_28, RPC_FE_27, RPC_FE_26, RPC_FE_25, RPC_FE_24, RPC_FE_23, RPC_FE_22, RPC_FE_21, RPC_FE_20, RPC_FE_19, RPC_FE_18, RPC_FE_17, RPC_FE_16, RPC_FE_15, RPC_FE_14, RPC_FE_13, RPC_FE_12, RPC_FE_11, RPC_FE_10, RPC_FE_9, RPC_FE_8, RPC_FE_7, RPC_FE_6, RPC_FE_5, RPC_FE_4, RPC_FE_3, RPC_FE_2, RPC_FE_1, RPC_FE_0)(ACTION, __VA_ARGS__))
#define RPC_FOR_EACH2(ACTION, ...) EXPAND(RPC_GET_MACRO(_0, __VA_ARGS__, RPC_FE2_30, RPC_FE2_29, RPC_FE2_28, RPC_FE2_27, RPC_FE2_26, RPC_FE2_25, RPC_FE2_24, RPC_FE2_23, RPC_FE2_22, RPC_FE2_21, RPC_FE2_20, RPC_FE2_19, RPC_FE2_18, RPC_FE2_17, RPC_FE2_16, RPC_FE2_15, RPC_FE2_14, RPC_FE2_13, RPC_FE2_12, RPC_FE2_11, RPC_FE2_10, RPC_FE2_9, RPC_FE2_8, RPC_FE2_7, RPC_FE2_6, RPC_FE2_5, RPC_FE2_4, RPC_FE2_3, RPC_FE2_2, RPC_FE2_1, RPC_FE2_0)(ACTION, __VA_ARGS__))

#define RPC_ATTACH_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return dispatch_func<Serial>(FUNCNAME, serial_obj); }
#define RPC_ATTACH_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_FUNC, FUNCNAME, __VA_ARGS__))

#define RPC_ATTACH_CACHED_FUNC(FUNCNAME) if (func_name == #FUNCNAME) { return dispatch_func<Serial>(FUNCNAME, serial_obj, true); }
#define RPC_ATTACH_CACHED_FUNCS(FUNCNAME, ...) EXPAND(RPC_FOR_EACH(RPC_ATTACH_CACHED_FUNC, FUNCNAME, __VA_ARGS__))

#define RPC_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { return dispatch_func<Serial>(FUNCNAME, serial_obj); }
#define RPC_MULTI_ALIAS_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))

#define RPC_ALIAS_CACHED_FUNC(FUNCNAME, FUNC_ALIAS) if (func_name == #FUNC_ALIAS) { return dispatch_func<Serial>(FUNCNAME, serial_obj, true); }
#define RPC_MULTI_ALIAS_CACHED_FUNC(FUNCNAME, FUNC_ALIAS,...) EXPAND(RPC_FOR_EACH2(RPC_ALIAS_CACHED_FUNC, FUNCNAME, FUNC_ALIAS, __VA_ARGS__))

#define RPC_DEFAULT_DISPATCH(FUNCNAME, ...) EXPAND(template<typename Serial> void rpc::server::dispatch(typename Serial::doc_type& serial_obj) { const auto func_name = serial_adapter<Serial>::extract_func_name(serial_obj); RPC_ATTACH_FUNCS(FUNCNAME, __VA_ARGS__) serial_adapter<Serial>::set_err_mesg(serial_obj, "RPC error: Called function: \"" + func_name + "\" not found!");})
