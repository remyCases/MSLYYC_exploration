// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef MEMORY_MANAGEMENT_H_
#define MEMORY_MANAGEMENT_H_

#ifndef MSL_FWK_MAJOR
#define MSL_FWK_MAJOR 1
#endif // MSL_FWK_MAJOR

#ifndef MSL_FWK_MINOR
#define MSL_FWK_MINOR 0
#endif // MSL_FWK_MINOR

#ifndef MSL_FWK_PATCH
#define MSL_FWK_PATCH 0
#endif // MSL_FWK_PATCH

#include "interface.h"

int mm_allocate_persistent_memory(size_t, void**);
int mm_allocate_memory(module_t*, size_t, void**);
int mm_free_persistent_memory(void*);
int mm_free_memory(module_t*, void*);
int mm_sigscan_module(const wchar_t*, const unsigned char*, const char*, size_t*);
int mm_sigscan_region(unsigned char*, const size_t, const unsigned char*, const char*, size_t*);
int mm_create_hook(module_t*, char*, void*, void*, void**);
int mm_hook_exists(module_t*, char*, bool*);
int mm_remove_hook(module_t*, char*);
int mm_get_hook_trampoline(module_t*, char*, void**);
int mm_get_framework_version(short*, short*, short*);
int mm_create_mid_function_hook(module_t*, char*, void*, MidHookFunction);

#endif  /* !MEMORY_MANAGEMENT_H_ */