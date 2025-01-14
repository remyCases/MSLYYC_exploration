// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "wrapper.hpp"

WrapperSafetyHookInline::WrapperSafetyHookInline(void* target, void* destination, SafetyHookInline::Flags flags = SafetyHookInline::Default) {
    *safety_hook_inline_instance = safetyhook::create_inline(target, destination, flags);
}

WrapperSafetyHookInline::~WrapperSafetyHookInline() {
    delete safety_hook_inline_instance;
}

WrapperSafetyHookMid::WrapperSafetyHookMid(void* target, safetyhook::MidHookFn destination, SafetyHookMid::Flags flags = SafetyHookMid::Default) {
    *safety_hook_mid_instance = safetyhook::create_mid(target, destination, flags);
}

WrapperSafetyHookMid::~WrapperSafetyHookMid() {
    delete safety_hook_mid_instance;
}