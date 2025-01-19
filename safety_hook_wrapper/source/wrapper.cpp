// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "wrapper.hpp"

WrapperSafetyHookInline::WrapperSafetyHookInline(void) {
    safety_hook_inline_instance = {};
}

WrapperSafetyHookInline::WrapperSafetyHookInline(void* target, void* destination) {
    *safety_hook_inline_instance = safetyhook::create_inline(target, destination, SafetyHookInline::Default);
}

WrapperSafetyHookInline::WrapperSafetyHookInline(void* target, void* destination, SafetyHookInline::Flags flags) {
    *safety_hook_inline_instance = safetyhook::create_inline(target, destination, flags);
}

WrapperSafetyHookInline::~WrapperSafetyHookInline() {
    delete safety_hook_inline_instance;
}

void* WrapperSafetyHookInline::get_original_pvoid() {
    return this->safety_hook_inline_instance->original<void*>();
}

WrapperSafetyHookMid::WrapperSafetyHookMid(void) {
    safety_hook_mid_instance = {};
}

WrapperSafetyHookMid::WrapperSafetyHookMid(void* target, safetyhook::MidHookFn destination) {
    *safety_hook_mid_instance = safetyhook::create_mid(target, destination, SafetyHookMid::Default);
}

WrapperSafetyHookMid::WrapperSafetyHookMid(void* target, safetyhook::MidHookFn destination, SafetyHookMid::Flags flags) {
    *safety_hook_mid_instance = safetyhook::create_mid(target, destination, flags);
}

WrapperSafetyHookMid::~WrapperSafetyHookMid() {
    delete safety_hook_mid_instance;
}