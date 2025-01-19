// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "Windows.h"
#include "TlHelp32.h"
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
    inline_hook_t* created_hook = NULL;
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
    mid_hook_t* created_hook = NULL;
    CHECK_CALL(mmp_create_mid_hook, module, hook_identifier, source_address, target_handler, &created_hook);

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

int mmp_sigscan_region(const unsigned char* region_base, const size_t region_size, const unsigned char* pattern, const char* pattern_mask, uintptr_t* pattern_base)
{
    size_t pattern_size = strlen(pattern_mask);

    // Loop all bytes in the region
    for (size_t region_byte = 0; region_byte < region_size - pattern_size; region_byte++)
    {
        // Loop all bytes in the pattern and compare them to the bytes in the region
        bool pattern_matches = true;
        for (size_t in_pattern_byte = 0; in_pattern_byte < pattern_size; in_pattern_byte++)
        {
            pattern_matches &= 
                (pattern_mask[in_pattern_byte] == '?') || 
                (region_base[region_byte + in_pattern_byte] == pattern[in_pattern_byte]);

            // If it already doesn't match, we don't have to iterate anymore
            if (!pattern_matches)
                break;
        }

        if (pattern_matches)
        {
            *pattern_base = (uintptr_t)(region_base + region_byte);
            return MSL_SUCCESS;
        }
    }

    return MSL_OBJECT_NOT_FOUND;
}

int mmp_remove_allocations_from_table(module_t* owner_module, const void* allocation_base)
{
    int last_status = MSL_SUCCESS;

    size_t new_size = owner_module->memory_allocations.size;
    memory_allocation_t* mem_alloc;
    for (size_t i = 0; i < owner_module->memory_allocations.size; i++)
    {
        mem_alloc = &owner_module->memory_allocations.arr[i];
        if (mem_alloc->allocation_base == allocation_base)
        {
            owner_module->memory_allocations.arr[i] = owner_module->memory_allocations.arr[--new_size];
            if (!new_size) break;
        }
    }
    owner_module->memory_allocations.size = new_size;
    return last_status;
}

int mmp_add_inline_hook_to_table(module_t* owner_module, inline_hook_t* hook)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ADD_VECTOR(inline_hook_t), &owner_module->inline_hooks, hook);
    return last_status;
}

int mmp_add_mid_hook_to_table(module_t* owner_module, mid_hook_t* hook)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ADD_VECTOR(mid_hook_t), &owner_module->mid_hooks, hook);
    return last_status;
}

int mmp_remove_inline_hook(module_t* module, inline_hook_t* hook, bool remove_from_table)
{
    int last_status = MSL_SUCCESS;
    // Do the unhook atomically
    CHECK_CALL(mmp_freeze_current_process);
    hook->hook_instance = shi_init();
    CHECK_CALL(mmp_resume_current_process);

    if (remove_from_table)
    {
        CHECK_CALL(mmp_remove_inline_hook_from_table, module, hook);
    }

    return last_status;
}

int mmp_remove_mid_hook(module_t* module, mid_hook_t* hook, bool remove_from_table)
{
    int last_status = MSL_SUCCESS;
    // Do the unhook atomically
    CHECK_CALL(mmp_freeze_current_process);
    hook->hook_instance = shm_init();
    CHECK_CALL(mmp_resume_current_process);

    if (remove_from_table)
    {
        CHECK_CALL(mmp_remove_mid_hook_from_table, module, hook);
    }

    return last_status;
}

int mmp_remove_hook(module_t* module, char* hook_identifier, bool remove_from_table)
{
    int last_status = MSL_SUCCESS;

    // Try to look it up in the inline hook table
    inline_hook_t* inline_hook_object = NULL;
    CHECK_CALL(mmp_lookup_inline_hook_by_name, module, hook_identifier, &inline_hook_object);

    // If we found it, we can remove it
    if (inline_hook_object != NULL)
    {
        CHECK_CALL(mmp_remove_inline_hook, module, inline_hook_object, remove_from_table);
        return last_status;
    }

    // We know it's not an inline hook, so try searching for a midhook
    mid_hook_t* mid_hook_object = NULL;
    CHECK_CALL(mmp_lookup_mid_hook_by_name, module, hook_identifier, &mid_hook_object);

    // If we found it, remove it
    if (mid_hook_object != NULL)
    {
        CHECK_CALL(mmp_remove_mid_hook, module, mid_hook_object, remove_from_table);
        return last_status;
    }

    // Else it's a non-existent hook.
    return MSL_OBJECT_NOT_FOUND;
}

static int predicate_inline(inline_hook_t* inline_hook, void* context, bool* flag)
{
    int last_status = MSL_SUCCESS;
    inline_hook_t* hook = (inline_hook_t*)context;
    *flag = inline_hook == hook;
    return last_status;
}

static void destructor_inline(inline_hook_t* inline_hook)
{
    shi_destroy(inline_hook->hook_instance);
}

int mmp_remove_inline_hook_from_table(module_t* module, inline_hook_t* hook)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(REMOVE_VECTOR_IF(inline_hook_t), &module->inline_hooks, predicate_inline, hook, destructor_inline);
    return last_status;
}

static int predicate_mid(mid_hook_t* mid_hook, void* context, bool* flag)
{
    int last_status = MSL_SUCCESS;
    mid_hook_t* hook = (mid_hook_t*)context;
    *flag = mid_hook == hook;
    return last_status;
}

static void destructor_mid(mid_hook_t* mid_hook)
{
    shm_destroy(mid_hook->hook_instance);
}

int mmp_remove_mid_hook_from_table(module_t* module, mid_hook_t* hook)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(REMOVE_VECTOR_IF(mid_hook_t), &module->mid_hooks, predicate_mid, hook, destructor_mid);
    return last_status;
}

int mmp_lookup_inline_hook_by_name(module_t* module, char* hook_identifier, inline_hook_t** hook)
{
    inline_hook_t* inline_hook = NULL;
    for (size_t i = 0; i < module->inline_hooks.size; i++)
    {
        inline_hook = &module->inline_hooks.arr[i];
        if (!strcmp(inline_hook->identifier, hook_identifier))
        {
            *hook = inline_hook;
            return MSL_SUCCESS;
        }
    }
    return MSL_OBJECT_NOT_FOUND;
}

int mmp_lookup_mid_hook_by_name(module_t* module, char* hook_identifier, mid_hook_t** hook)
{
    mid_hook_t* mid_hook = NULL;
    for (size_t i = 0; i < module->mid_hooks.size; i++)
    {
        mid_hook = &module->mid_hooks.arr[i];
        if (!strcmp(mid_hook->identifier, hook_identifier))
        {
            *hook = mid_hook;
            return MSL_SUCCESS;
        }
    }
    return MSL_OBJECT_NOT_FOUND;
}

int mmp_create_inline_hook(module_t* module, char* hook_identifier, void* source_function, void* destination_function, inline_hook_t** hook)
{
    int last_status = MSL_SUCCESS;
    // Create the hook object
    (*hook)->owner = module;
    (*hook)->identifier = hook_identifier;
    (*hook)->hook_instance = shi_create_default_flag(source_function, destination_function);

    // Add the hook to the table
    CHECK_CALL(mmp_add_inline_hook_to_table, module, *hook);
    return last_status;
}

int mmp_create_mid_hook(module_t* module, char* hook_identifier, void* source_function, void* destination_function, mid_hook_t** hook)
{
    int last_status = MSL_SUCCESS;
    // Create the hook object
    (*hook)->owner = module;
    (*hook)->identifier = hook_identifier;
    (*hook)->hook_instance = shm_create_default_flag(source_function, destination_function);

    // Add the hook to the table
    CHECK_CALL(mmp_add_mid_hook_to_table, module, *hook);
    return last_status;
}

static int callback_freeze(const THREADENTRY32* entry, bool* flag)
{
    *flag = false;
    // Skip my thread
    if (GetCurrentThreadId() == entry->th32ThreadID) return MSL_SUCCESS;

    // Skip everything that's not my process
    if (GetCurrentProcessId() != entry->th32OwnerProcessID) return MSL_SUCCESS;

    HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, false, entry->th32ThreadID);

    if (!thread) return MSL_SUCCESS;

    SuspendThread(thread);
    CloseHandle(thread);

     return MSL_SUCCESS;
}

int mmp_freeze_current_process(void)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(el_for_each_thread, callback_freeze);
    return last_status;
}

static int callback_resume(const THREADENTRY32* entry, bool* flag)
{
    *flag = false;
    // Skip my thread
    if (GetCurrentThreadId() == entry->th32ThreadID) return MSL_SUCCESS;

    // Skip everything that's not my process
    if (GetCurrentProcessId() != entry->th32OwnerProcessID) return MSL_SUCCESS;

    HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, false, entry->th32ThreadID);

    if (!thread) return MSL_SUCCESS;

    ResumeThread(thread);
    CloseHandle(thread);

     return MSL_SUCCESS;
}


int mmp_resume_current_process(void)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(el_for_each_thread, callback_resume);
    return last_status;
}