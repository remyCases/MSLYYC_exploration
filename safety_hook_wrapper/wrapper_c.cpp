// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "wrapper.h"
#include "wrapper.hpp"

extern "C" {
    safety_hook_inline_t safety_hook_inline_create(void* target, void* destination, SafetyHookInline::Flags flags = SafetyHookInline::Default) {
        return reinterpret_cast<WrapperSafetyHookInline*>(new WrapperSafetyHookInline(target, destination, flags));
    }

    void safety_hook_inline_destroy(safety_hook_inline_t handle) {
        delete reinterpret_cast<WrapperSafetyHookInline*>(handle);
    }

    int safety_hook_inline_some_operation(safety_hook_inline_t handle, int value) {
        return reinterpret_cast<WrapperSafetyHookInline*>(handle)->someOperation(value);
    }

    safety_hook_mid_t safety_hook_mid_create(void* target, safetyhook::MidHookFn destination, SafetyHookMid::Flags flags = SafetyHookMid::Default) {
        return reinterpret_cast<WrapperSafetyHookMid*>(new WrapperSafetyHookMid(target, destination, flags));
    }

    void safety_hook_mid_destroy(safety_hook_mid_t handle) {
        delete reinterpret_cast<WrapperSafetyHookMid*>(handle);
    }

    int safety_hook_mid_some_operation(safety_hook_mid_t handle, int value) {
        return reinterpret_cast<WrapperSafetyHookMid*>(handle)->someOperation(value);
    }
}