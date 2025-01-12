// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdint.h>
#include "module.h"

typedef enum EVENT_TRIGGERS
{
    EVENT_OBJECT_CALL = 1,	// The event represents a Code_Execute() call.
    EVENT_FRAME = 2,		// The event represents an IDXGISwapChain::Present() call.
    EVENT_RESIZE = 3,		// The event represents an IDXGISwapChain::ResizeBuffers() call.
    EVENT_UNUSED = 4,		// This value is unused.
    EVENT_WNDPROC = 5		// The event represents a WndProc() call.
} EVENT_TRIGGERS;

typedef struct module_callback_descriptor_s
{
    module_t* owner_module;
    EVENT_TRIGGERS trigger;
    int32_t priority;
    void* routine;
} module_callback_descriptor_t;

typedef struct module_callback_descriptor_array_s
{
    module_callback_descriptor_t* arr;
    size_t size;
    size_t capacity;
} module_callback_descriptor_array_t;

int init_module_callbacks();
int free_module_callbacks();
int create_callback(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority);
int remove_callback(module_t* module, void* routine);