// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "wrapper.h"
#include "wrapper.hpp"

extern "C" {
    safety_hook_inline_t shi_init(void) 
    {
        return reinterpret_cast<WrapperSafetyHookInline*>(new WrapperSafetyHookInline());
    }
    
    safety_hook_inline_t shi_create(void* target, void* destination, int flags) 
    {
        return reinterpret_cast<WrapperSafetyHookInline*>(new WrapperSafetyHookInline(target, destination, static_cast<SafetyHookInline::Flags>(flags)));
    }

    void shi_destroy(safety_hook_inline_t handle) 
    {
        delete reinterpret_cast<WrapperSafetyHookInline*>(handle);
    }

    void* shi_get_original_pvoid(safety_hook_inline_t handle) 
    {
        return reinterpret_cast<WrapperSafetyHookInline*>(handle)->get_original_pvoid();
    }

    safety_hook_mid_t shm_init(void) 
    {
        return reinterpret_cast<WrapperSafetyHookMid*>(new WrapperSafetyHookMid());
    }

    safety_hook_mid_t shm_create(void* target, void* destination, int flags) 
    {
        return reinterpret_cast<WrapperSafetyHookMid*>(new WrapperSafetyHookMid(target, reinterpret_cast<safetyhook::MidHookFn>(destination), static_cast<SafetyHookMid::Flags>(flags)));
    }

    void shm_destroy(safety_hook_mid_t handle) 
    {
        delete reinterpret_cast<WrapperSafetyHookMid*>(handle);
    }
}