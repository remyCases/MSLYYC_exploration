// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remy_Cases/MSLYYC_exploration.

#ifndef FUNCTION_WRAPPER_H_
#define FUNCTION_WRAPPER_H_

#include "dxgi.h"
#include "gml_structs.h"

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

#define FUNCTION_WRAPPER(F, T, ...) 							\
typedef struct SS_CAT_UND(fn, T, ARG_LENGTH(__VA_ARGS__), s)	\
{																\
	typedef struct args_s {										\
		/*TODO: implement that*/								\
	} args_t; 													\
	args_t args;												\
	T (*function)(args_t);										\
	bool called_original;										\
	T result;													\
	T (*Call)(); 												\
	T (*Call)(__VA_ARGS__);										\
} F;

FUNCTION_WRAPPER(FWCodeEvent, bool, instance_t*, instance_t*, code_t*, int, rvalue_t*)
FUNCTION_WRAPPER(FWFrame, HRESULT, IDXGISwapChain*, UINT, UINT)
FUNCTION_WRAPPER(FWResize, HRESULT, IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
FUNCTION_WRAPPER(FWWndProc, LRESULT, HWND, UINT, WPARAM, LPARAM)
CALL(FWWndProc, LRESULT)
ARG_LENGTH_(instance_t*)
#endif  /* !FUNCTION_WRAPPER_H_ */