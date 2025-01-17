// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include <safetyhook.hpp>

class WrapperSafetyHookInline {
public:
    WrapperSafetyHookInline(void);
    WrapperSafetyHookInline(void*, void*);
    WrapperSafetyHookInline(void*, void*, SafetyHookInline::Flags);
    ~WrapperSafetyHookInline();
    
    // Add methods that mirror your C++ library functionality
    void* get_original_pvoid(void);
    
private:
    SafetyHookInline* safety_hook_inline_instance;
};

class WrapperSafetyHookMid {
public:
    WrapperSafetyHookMid(void);
    WrapperSafetyHookMid(void*, safetyhook::MidHookFn);
    WrapperSafetyHookMid(void*, safetyhook::MidHookFn, SafetyHookMid::Flags);
    ~WrapperSafetyHookMid();
    
    // Add methods that mirror your C++ library functionality
    
private:
    SafetyHookMid* safety_hook_mid_instance;
};

#endif  /* !WRAPPER_HPP */
