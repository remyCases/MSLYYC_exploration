// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include <safetyhook.hpp>

class WrapperSafetyHookInline {
public:
    WrapperSafetyHookInline(void* target, void* destination, SafetyHookInline::Flags flags = SafetyHookInline::Default);
    ~WrapperSafetyHookInline();
    
    // Add methods that mirror your C++ library functionality
    void* get_original_pvoid(void);
    
private:
    SafetyHookInline* safety_hook_inline_instance;
};

class WrapperSafetyHookMid {
public:
    WrapperSafetyHookMid(void* target, safetyhook::MidHookFn destination, SafetyHookMid::Flags flags = SafetyHookMid::Default);
    ~WrapperSafetyHookMid();
    
    // Add methods that mirror your C++ library functionality
    int someOperation(int value);
    
private:
    SafetyHookMid* safety_hook_mid_instance;
};

#endif  /* !WRAPPER_HPP */
