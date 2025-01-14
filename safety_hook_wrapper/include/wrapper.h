// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WrapperSafetyHookInline* safety_hook_inline_t;

safety_hook_inline_t safety_hook_inline_create(void* target, void* destination, int flags);
void safety_hook_inline_destroy(safety_hook_inline_t handle);
int safety_hook_inline_some_operation(safety_hook_inline_t handle, int value);

typedef struct WrapperSafetyHookMid* safety_hook_mid_t;

safety_hook_mid_t safety_hook_mid_create(void* target, void* destination, int flags);
void safety_hook_mid_destroy(safety_hook_mid_t handle);
int safety_hook_mid_some_operation(safety_hook_mid_t handle, int value);

#ifdef __cplusplus
}
#endif  /* !__cplusplus */

#endif  /* !WRAPPER_H */
