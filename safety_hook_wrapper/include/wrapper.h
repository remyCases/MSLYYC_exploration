// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WrapperSafetyHookInline* safety_hook_inline_t;

safety_hook_inline_t shi_init(void);
safety_hook_inline_t shi_create_default_flag(void*, void*);
safety_hook_inline_t shi_create(void*, void*, int);
void shi_destroy(safety_hook_inline_t);
void* shi_get_original_pvoid(safety_hook_inline_t);

typedef struct WrapperSafetyHookMid* safety_hook_mid_t;

safety_hook_mid_t shm_init(void);
safety_hook_mid_t shm_create_default_flag(void*, void*);
safety_hook_mid_t shm_create(void*, void*, int);
void shm_destroy(safety_hook_mid_t);

#ifdef __cplusplus
}
#endif  /* !__cplusplus */

#endif  /* !WRAPPER_H */
