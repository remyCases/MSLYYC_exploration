// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "Windows.h"
#include <stdint.h>
#include "inttypes.h"
#include "../include/memory_management.h"
#include "../include/pe_parser.h"
#include "../include/error.h"

module_t* global_initial_image;

int mm_allocate_persistent_memory_alloc(size_t size, void** allocation_base)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(mm_allocate_memory_alloc, global_initial_image, size, allocation_base);
    return last_status;
}

int mm_allocate_memory_alloc(module_t* owner, size_t size, void** allocation_base)
{
    int last_status = MSL_SUCCESS;
    memory_allocation_t allocation;
    
    CHECK_CALL(mmp_allocate_memory_alloc, size, owner, &allocation);
    CHECK_CALL(mmp_add_allocation_to_table, &allocation);

    *allocation_base = allocation.allocation_base;
    return last_status;
}

int mm_free_persistent_memory(void* allocation_base)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(mm_free_memory, global_initial_image, allocation_base);
    return last_status;
}

int mm_free_memory(module_t* owner, void* allocation_base)
{
    int last_status = MSL_SUCCESS;
    bool flag;
    CHECK_CALL(mmp_is_allocated_memory, owner, allocation_base, &flag)
    if (!flag)
    {
        return MSL_INVALID_PARAMETER;
    }

    CHECK_CALL(mmp_free_memory, owner, allocation_base, true);
    return last_status;
}

int mm_sigscan_module(const wchar_t* module_name, const unsigned char* pattern, const char* pattern_mask, size_t* pattern_base)
{
    int last_status = MSL_SUCCESS;
    *pattern_base = 0;
    // Capture the module we're searching for
    HMODULE module_handle = GetModuleHandleW(module_name);
    if (!module_handle) return MSL_INVALID_HANDLE_VALUE;

    // Query the text section address in the module
    uint64_t text_section_base = 0;
    size_t text_section_size = 0;
    CHECK_CALL(ppi_get_module_section_bounds, module_handle, ".text", &text_section_base, &text_section_size);
    CHECK_CALL(mm_sigscan_region, (unsigned char*)(module_handle) + text_section_base, text_section_size, pattern, pattern_mask, pattern_base);
    return last_status;
}

int mm_sigscan_region(unsigned char* region_base, const size_t region_size, const unsigned char* pattern, const char* pattern_mask, size_t* pattern_base)
{
    int last_status = MSL_SUCCESS;
    *pattern_base = 0;
    if (!pattern_mask || !strlen(pattern_mask)) return MSL_INVALID_PARAMETER;

    size_t _pattern_base = 0;
    CHECK_CALL(mmp_sigscan_region, region_base, region_size, pattern, pattern_mask, pattern_base);

    *pattern_base = _pattern_base;
    return last_status;
}

int mm_create_hook(module_t* module, char* hook_identifier, void* source_function, void* destination_function, void** trampoline)
{
    int last_status = MSL_SUCCESS;
    bool flag;
    CHECK_CALL(mm_hook_exists, module, hook_identifier, &flag);
    if (flag) return MSL_OBJECT_ALREADY_EXISTS;

    CHECK_CALL(mmp_freeze_current_process);

    // Creates and enables the actual hook
    inline_hook_t* created_hook;
    CHECK_CALL(mmp_create_inline_hook, module, hook_identifier, source_function, destination_function, &created_hook);

    if (!created_hook)
    {
        last_status = MSL_INSUFFICIENT_MEMORY;
        goto ret;
    }

    // If the hook is invalid, we're probably passing invalid parameters to it.
    if (!created_hook->hook_instance)
    {
        last_status = MSL_INVALID_PARAMETER;
        goto ret;
    }
    
    if (trampoline) *trampoline = shi_get_original_pvoid(created_hook->hook_instance);

    ret:
    CHECK_CALL(mmp_resume_current_process);
    return last_status;
}

int mm_hook_exists(module_t* module, char* hook_identifier, bool* hook)
{
    int last_status = MSL_SUCCESS;
    *hook = false;

    inline_hook_t* inline_hook_object = NULL;
    mid_hook_t* mid_hook_object = NULL;

    CHECK_CALL(mmp_lookup_inline_hook_by_name, module, hook_identifier, &inline_hook_object);
    *hook = inline_hook_object == NULL;

    if (!*hook)
    {
        CHECK_CALL(mmp_lookup_mid_hook_by_name, module, hook_identifier, &mid_hook_object);
        *hook = mid_hook_object == NULL;
    }
    
    return last_status;
}

int mm_remove_hook(module_t* module, char* hook_identifier)
{
    int last_status = MSL_SUCCESS;
    // The internal routine checks for the hook's existance, 
    // so we can just call it without performing any other checks.
    CHECK_CALL(mmp_remove_hook, module, hook_identifier, true);
    return last_status;
}

int mm_get_hook_trampoline(module_t* module, char* hook_identifier, void** trampoline)
{
    int last_status = MSL_SUCCESS;
    *trampoline = NULL;
    inline_hook_t* hook_object = NULL;

    CHECK_CALL(mmp_lookup_inline_hook_by_name, module, hook_identifier, &hook_object);
    *trampoline = shi_get_original_pvoid(hook_object->hook_instance);
    return last_status;
}

int mm_get_framework_version(short* major, short* minor, short* patch)
{
    int last_status = MSL_SUCCESS;
    if (major) *major = MSL_FWK_MAJOR;
    if (minor) *minor = MSL_FWK_MINOR;
    if (patch) *patch = MSL_FWK_PATCH;
    return last_status;
}

int mm_create_mid_function_hook(module_t* module, char* hook_identifier, void* source_address, MidHookFunction target_handler)
{
    int last_status = MSL_SUCCESS;
    bool flag;
    CHECK_CALL(mm_hook_exists, module, hook_identifier, &flag);

    if (flag) return MSL_OBJECT_ALREADY_EXISTS;

    CHECK_CALL(mmp_freeze_current_process);

    // Creates and enables the actual hook
    mid_hook_t* created_hook;
    CHECK_CALL(mmp_create_midHook, module, hook_identifier, source_address, target_handler, &created_hook);

    CHECK_CALL(mmp_resume_current_process);

    if (!created_hook) return MSL_INSUFFICIENT_MEMORY;

    // If the hook is invalid, we're probably passing invalid parameters to it.
    if (!created_hook->hook_instance) return MSL_INVALID_PARAMETER;

    return last_status;
}

int mmp_allocate_memory_alloc(const size_t allocation_size, module_t* owner_module, memory_allocation_t* allocation)
{
    int last_status = MSL_SUCCESS;
    allocation->allocation_base = malloc(allocation_size);
    allocation->allocation_size = allocation_size;
    allocation->owner_module = owner_module;

    return last_status;
}

int mmp_verify_callback(HMODULE hmodule, void* callback_routine)
{
    if (callback_routine && hmodule) return MSL_SUCCESS;
    return MSL_ACCESS_DENIED;
}

int mmp_free_memory(module_t* owner_module, void* allocation_base, bool remove_table_entry)
{
    int last_status = MSL_SUCCESS;
    if (remove_table_entry)
    {
        CHECK_CALL(mmp_remove_allocations_from_table, owner_module, allocation_base);
    }

    free(allocation_base);
    return last_status;
}

int mmp_add_allocation_to_table(memory_allocation_t* allocation)
{
    
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ADD_VECTOR(memory_allocation_t), &allocation->owner_module->memory_allocations, allocation);
    return last_status;
}

int mmp_is_allocated_memory(module_t* module, void* allocation_base, bool* allocated)
{
    memory_allocation_t* mem_alloc = NULL;
    for (size_t i = 0; i < module->memory_allocations.size; i++)
    {
        mem_alloc = &module->memory_allocations.arr[i];
        if (mem_alloc->allocation_base == allocation_base)
        {
            *allocated = true;
            return MSL_SUCCESS;
        }
    }
    *allocated = false;
    return MSL_SUCCESS;
}