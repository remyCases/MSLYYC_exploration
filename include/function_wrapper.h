// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remy_Cases/MSLYYC_exploration.

#ifndef FUNCTION_WRAPPER_H_
#define FUNCTION_WRAPPER_H_

#include "dxgi.h"
#include "gml_structs.h"
#include <stdbool.h>

#define CALL(F, T)										\
T Call(F* f) {											\
	f->called_original = true;							\
	f->result = f->function(f->args);					\
	return f->result;									\
}

#define CALLA(F, T, ...)								\
T Call(F* f, __VA_ARGS__) {								\
	f->called_original = true;							\
	f->result = f->function(__VA_ARGS__);				\
	return f->result;									\
}

#define EXPANDS(N, ...) S_CAT(EXPAND_, N)(__VA_ARGS__)
#define EXPAND_0(...)
#define EXPAND_1(a0, ...) a0
#define EXPAND_2(a0, ...) a0 EXPAND_1(__VA_ARGS__)
#define EXPAND_3(a0, ...) a0 EXPAND_2(__VA_ARGS__)
#define EXPAND_4(a0, ...) a0 EXPAND_3(__VA_ARGS__)
#define EXPAND_5(a0, ...) a0 EXPAND_4(__VA_ARGS__)
#define EXPAND_6(a0, ...) a0 EXPAND_5(__VA_ARGS__)

#define TO_ARGS(N, ...) GET_ARG_N(N, __VA_ARGS__) S_CAT(_, N);

#define GET_ARG_N(N, ...) GET_ARG_N_(N, __VA_ARGS__)
#define GET_ARG_N_(N, ...) S_CAT(GET_ARG_, N)(__VA_ARGS__)

#define GET_ARG_0(a0, ...) a0
#define GET_ARG_1(a0, a1, ...) a1
#define GET_ARG_2(a0, a1, a2, ...) a2
#define GET_ARG_3(a0, a1, a2, a3, ...) a3
#define GET_ARG_4(a0, a1, a2, a3, a4, ...) a4
#define GET_ARG_5(a0, a1, a2, a3, a4, a5, ...) a5
#define GET_ARG_6(a0, a1, a2, a3, a4, a5, a6, ...) a6

#define FUNCTION_WRAPPER(F, T, ...) 								\
typedef struct SS_CAT_UND(arg, T, ARG_LENGTH(__VA_ARGS__), s) {		\
	EXPANDS(ARG_LENGTH(__VA_ARGS__), ARG_COUNT(TO_ARGS, __VA_ARGS__))\
} SS_CAT_UND(arg, T, ARG_LENGTH(__VA_ARGS__), t);					\
typedef struct SS_CAT_UND(fn, T, ARG_LENGTH(__VA_ARGS__), s)		\
{																	\
	SS_CAT_UND(arg, T, ARG_LENGTH(__VA_ARGS__), t) args;			\
	T (*function)(SS_CAT_UND(arg, T, ARG_LENGTH(__VA_ARGS__), t));	\
	bool called_original;											\
	T result;														\
	T (*Call)(); 													\
	T (*CallA)(__VA_ARGS__);										\
} F;

FUNCTION_WRAPPER(FWCodeEvent, bool, instance_t*, instance_t*, code_t*, int, rvalue_t*)
FUNCTION_WRAPPER(FWFrame, HRESULT, IDXGISwapChain*, UINT, UINT)
FUNCTION_WRAPPER(FWResize, HRESULT, IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
FUNCTION_WRAPPER(FWWndProc, LRESULT, HWND, UINT, WPARAM, LPARAM)
CALL(FWWndProc, LRESULT)
#endif  /* !FUNCTION_WRAPPER_H_ */