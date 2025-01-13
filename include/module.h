// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef MODULE_H_
#define MODULE_H_

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

typedef struct module_s 
{
    union
    {
        uint8_t bitfield;
        struct
        {
            // If this bit is set, the module's Initialize function has been called.
            bool is_initialized : 1;

            // If this bit is set, the module's Preload function has been called.
            // This call to Preload happens before the call to Initialize.
            // 
            // If the Aurie Framework is injected into a running process, this function is called
            // right before the call to Initialize.
            // Otherwise, this function is guaranteed to run before the main process's entrypoint.
            bool is_preloaded : 1;

            // If this bit is set, the module is marked for deletion and will be unloaded by the next
            // call to Aurie::Internal::MdpPurgeMarkedModules
            bool marked_for_purge : 1;

            // If this bit is set, the module was loaded by a MdMapImage call from another module.
            // This makes it such that its ModulePreload function never gets called.
            bool is_runtime_loaded : 1;
        };
    } flags;

    // Describes the image base (and by extent the module).
    union
    {
        HMODULE module;
        void* pointer;
        unsigned long long address;
    } image_base;

    // Specifies the image size in memory.
    uint32_t image_size;

    // The path of the loaded image.
    char* image_path;

    // The address of the Windows entrypoint of the image.
    union
    {
        void* pointer;
        unsigned long long address;
    } ImageEntrypoint;
} module_t;

#endif  /* !MODULE_H_ */