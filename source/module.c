// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/module.h"
#include "../include/error.h"
#include "../include/object.h"
#include "Psapi.h"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

VECTOR(module_t) global_module_list;

int mdp_create_module(const char* image_path, HMODULE image_module, bool process_exports, uint8_t bit_flags, module_t* module)
{
    int last_status = MSL_SUCCESS;
    module_t temp_module;

    // Populate known fields first
    temp_module.flags.bitfield = bit_flags;
    temp_module.image_path = image_path;

    if (process_exports)
    {
        last_status = LOG_ON_ERR(mdp_process_image_exports, image_path, image_module, &temp_module);
        if (last_status) return last_status;
    }

    last_status = LOG_ON_ERR(mdp_query_module_information, image_module, &temp_module.image_base.pointer, &temp_module.image_size, &temp_module.image_entrypoint.pointer);
    if (last_status) return last_status;

    *module = temp_module;

    return last_status;
}

int mdp_is_module_marked_for_purge(module_t* module, bool* flag)
{
    int last_status = MSL_SUCCESS;
    *flag = module->flags.marked_for_purge;
    return last_status;
}

int mdp_mark_module_for_purge(module_t* module)
{
    int last_status = MSL_SUCCESS;
    module->flags.marked_for_purge = true;
    return last_status;
}

int mdp_purge_marked_modules(void)
{
    int last_status = MSL_SUCCESS;
    module_t* module = NULL;
    bool purge_flag = false;
    // Loop through all the modules marked for purge
    for(size_t i = 0; i < global_module_list.size; i++)
    {
        module = &global_module_list.arr[i];
        last_status = LOG_ON_ERR(mdp_is_module_marked_for_purge, module, &purge_flag);
        if (last_status) return last_status;

        if (purge_flag)
        {
            // Unmap the module, but don't call the unload routine, and don't remove it from the list
            last_status = LOG_ON_ERR(mdp_unmap_image, module, false, false);
            if (last_status) return last_status;
        }
    }

    // Remove the now unloaded modules from our list
    // Note we can't do this in the for loop, since that'd invalidate the iterators
    size_t new_size = global_module_list.size;
    for(size_t i = 0; i < global_module_list.size; i++)
    {
        module = &global_module_list.arr[i];
        last_status = LOG_ON_ERR(mdp_is_module_marked_for_purge, module, &purge_flag);
        if (last_status) return last_status;

        if (purge_flag)
        {
            global_module_list.arr[i] = global_module_list.arr[new_size - 1];
            new_size--;

            if (!new_size) break;
        }
    }
    return last_status;
}

int mdp_map_image(const char* image_path, HMODULE* image_base)
{
    // If the file doesn't exist, we have nothing to map
    if (access(image_path, F_OK)) return MSL_FILE_NOT_FOUND;

    int last_status = MSL_SUCCESS;
    unsigned short target_arch = 0;
    unsigned short self_arch = 0;
    
    // Query the target image architecture
    last_status = LOG_ON_ERR(pp_query_image_architecture, image_path, target_arch);
    if (last_status) return last_status;

    // Query the current architecture
    last_status = LOG_ON_ERR(pp_get_current_architecture, self_arch);
    if (last_status) return last_status;

    // Don't try to load modules which are the wrong architecture
    if (target_arch != self_arch) return MSL_INVALID_ARCH;

    // Make sure the image has the required exports
    int framework_init;
    int module_entry;
    int module_preinit;

    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "__AurieFrameworkInit", &framework_init);
    if (last_status) return last_status;
    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "ModuleInitialize", &module_entry);
    if (last_status) return last_status;
    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "ModulePreinitialize", &module_preinit);
    if (last_status) return last_status;

    // If the image doesn't have a framework init function, we can't load it.
    if (framework_init) return MSL_INVALID_SIGNATURE;

    // If we don't have a module entry OR a module preinitialize function, we can't load.
    bool has_either_entry = module_entry || module_preinit;
    if (!has_either_entry) return MSL_INVALID_SIGNATURE;

    module_t* potential_loaded_copy = NULL;
    last_status = LOG_ON_ERR(mdp_lookup_module_by_path, image_path, &potential_loaded_copy);
    
    // If there's a module that's already loaded from the same path, deny loading it twice
    if (last_status == MSL_SUCCESS) return MSL_OBJECT_ALREADY_EXISTS;

    // Load the image into memory and make sure we loaded it
    HMODULE image_module = LoadLibraryW((LPCWSTR)image_path);
    if (!image_module) return MSL_EXTERNAL_ERROR;

    *image_base = image_module;
    return MSL_SUCCESS;
}

int mdp_build_module_list(const char* base_folder, bool recursive, bool(*predicate)(const char*), VECTOR(char)* files)
{
    int last_status = MSL_SUCCESS;
    files->size= 0;
    char tmp_path[MAX_PATH];
    tmp_path[0] = 0;

    directory_iterator_t* iter = NULL;
    last_status = LOG_ON_ERR(iterator_create, base_folder, "*", &iter);
    if (last_status) return last_status;

    do
    {
        strcpy(tmp_path, iter->current_path);
        tmp_path[MAX_PATH-1] = 0;
        strcat(tmp_path, iter->find_data.cFileName);
        tmp_path[MAX_PATH-1] = 0;
        if (predicate(tmp_path))
        {
            last_status = LOG_ON_ERR(ADD_VECTOR(char), files, tmp_path);
            if (last_status) return last_status;
        }

        if (iter->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            if (iterator_enter_directory(iter)) 
            {
                continue;  // Successfully entered directory
            }
        }
    } while (iterator_next(iter));

    last_status = LOG_ON_ERR(iterator_destroy, iter);
    return last_status;
}

int mdp_add_module_to_list(module_t* module)
{
    int last_status = MSL_SUCCESS;
    last_status = LOG_ON_ERR(ADD_VECTOR(module_t), &global_module_list, module);
    return last_status;
}

int mdp_query_module_information(HMODULE hmodule, void** module_base, uint32_t* size_of_module, void** entry_point)
{
    // Query the information by asking Windows
    MODULEINFO module_info;
    if (!GetModuleInformation(GetCurrentProcess(), hmodule, &module_info, sizeof(module_info)))
    {
        return MSL_EXTERNAL_ERROR;
    }

    // Fill in what the caller wants
    if (module_base)
        *module_base = module_info.lpBaseOfDll;

    if (size_of_module)
        *size_of_module = module_info.SizeOfImage;

    if (entry_point)
        *entry_point = module_info.EntryPoint;

    return MSL_SUCCESS;
}

int mdp_get_image_path(module_t* module, char** path)
{
    int last_status = MSL_SUCCESS;
    *path = module->image_path;
    return last_status;
}

int mdp_get_image_folder(module_t* module, char** path)
{
    int last_status = MSL_SUCCESS;
    char* module_path;
    last_status = LOG_ON_ERR(mdp_get_image_path, module, &module_path);
    if (last_status) return last_status;

    if (!has_parent_path(module_path)) return MSL_INVALID_PARAMETER;

    last_status = LOG_ON_ERR(parent_path, module_path, path);
    return last_status;
}

int mdp_get_next_module(module_t* module, module_t** next_module)
{
    // Find the module in our list (gets an iterator)
    for (size_t i = 0; i < global_module_list.size; i++)
    {
        if (&global_module_list.arr[i] == module)
        {
            *next_module = &global_module_list.arr[(i + 1) % global_module_list.size];    
            return MSL_SUCCESS;
        }
    }

    return MSL_INVALID_PARAMETER;
}

int mdp_get_module_base_address(module_t* module, void** ptr)
{
    int last_status = MSL_SUCCESS;
    *ptr = module->image_base.pointer;
    return last_status;
}

int mdp_lookup_module_by_path(const char* module_path, module_t** module)
{
    int last_status = MSL_SUCCESS;
    int equivalent;
    for (size_t i = 0; i < global_module_list.size; i++)
    {
        last_status = LOG_ON_ERR(paths_are_equivalent, module_path, global_module_list.arr[i].image_path, &equivalent);
        if (last_status) return last_status;

        if (equivalent)
        {
            *module = &global_module_list.arr[i];
            return MSL_SUCCESS;
        }
    }

    return MSL_INVALID_PARAMETER;
}

int mdp_process_image_exports(const char* image_path, HMODULE image_base_address, module_t* module_image)
{
    // Find all the required functions
    int last_status = MSL_SUCCESS;
    uintptr_t framework_init_offset;
    uintptr_t module_init_offset;
    uintptr_t module_preload_offset;
    uintptr_t module_callback_offset;
    uintptr_t module_unload_offset;

    // We always need __AurieFrameworkInit to exist.
    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "__AurieFrameworkInit", &framework_init_offset);
    if (last_status) return last_status;
    if (framework_init_offset) return MSL_FILE_PART_NOT_FOUND;

    // We also need either a ModuleInitialize or a ModulePreinitialize function.
    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "ModuleInitialize", &module_init_offset);
    if (last_status) return last_status;
    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "ModulePreinitialize", &module_preload_offset);
    if (last_status) return last_status;
    if (module_init_offset || module_preload_offset) return MSL_FILE_PART_NOT_FOUND;

    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "ModuleOperationCallback", &module_callback_offset);
    if (last_status) return last_status;
    last_status = LOG_ON_ERR(pp_find_file_export_by_name, image_path, "ModuleUnload", &module_unload_offset);
    if (last_status) return last_status;

    // Cast the problems away
    char* image_base = (char*)(image_base_address);

    Entry module_init = (Entry)(image_base + module_init_offset);
    Entry module_preload = (Entry)(image_base + module_preload_offset);
    Entry module_unload = (Entry)(image_base + module_unload_offset);
    LoaderEntry framework_init = (LoaderEntry)(image_base + framework_init_offset);
    ModuleCallback module_callback = (ModuleCallback)(image_base + module_callback_offset);

    // If the offsets are zero, the function wasn't found, which means we shouldn't populate the field.
    if (module_init_offset)
        module_image->module_initialize = module_init;

    if (module_preload_offset)
        module_image->module_preinitialize = module_preload;

    if (framework_init_offset)
        module_image->framework_initialize = framework_init;

    if (module_callback_offset)
        module_image->module_operation_callback = module_callback;

    if (module_unload_offset)
        module_image->module_unload = module_unload;

    return last_status;
}

int mdp_unmap_image(module_t* module, bool remove_from_list, bool call_unload_routine)
{
    int last_status = MSL_SUCCESS;

    // We don't have to do anything else, since SafetyHook will handle everything for us.
    // Truly a GOATed library, thank you @localcc for telling me about it love ya
    // C note: inline_hook_t and mid_hooks_t need a custom destructor to call the destructor from SafetyHook
    last_status = LOG_ON_ERR(CLEAR_VECTOR(inline_hook_t), &module->inline_hooks, destructor_inline_hook_t);
    if (last_status) return last_status;
    last_status = LOG_ON_ERR(CLEAR_VECTOR(mid_hook_t), &module->mid_hooks, destructor_mid_hook_t);
    if (last_status) return last_status;

    // Call the unload entry if needed
    if (call_unload_routine)
    {
        last_status = LOG_ON_ERR(mdp_dispatch_entry, module, module->module_unload);
        if (last_status) return last_status;
    }

    // Remove the module's operation callback
    module->module_operation_callback = NULL;

    // Destory all interfaces created by the module
    for (size_t i = 0; i < module->interface_table.size; i++)
    {
        last_status = LOG_ON_ERR(module->interface_table.arr[i].intf->destroy);
        if (last_status) return last_status;
    }

    // Wipe them off the interface table
    // Note these can't be freed, they're allocated by the owner module
    // C note: interface_table_entry_t has only ptr, so no need for a custom destructor here
    last_status = LOG_ON_ERR(CLEAR_VECTOR(interface_table_entry_t), &module->interface_table, NULL);
    if (last_status) return last_status;

    memory_allocation_t* mem_alloc = NULL;
    // Free all memory allocated by the module (except persistent memory)
    for (size_t i = 0; i < module->memory_allocations.size; i++)
    {
        mem_alloc = &module->memory_allocations.arr[i];
        last_status = LOG_ON_ERR(mmp_free_memory, mem_alloc->owner_module, mem_alloc->allocation_base, false);
        if (last_status) return last_status;
    }

    // Remove all the allocation entries, they're now invalid
    // C note: memory_allocation_t has only ptr, so no need for a custom destructor here
    last_status = LOG_ON_ERR(CLEAR_VECTOR(memory_allocation_t), &module->memory_allocations, NULL);
    if (last_status) return last_status;

    // Free the module
    FreeLibrary(module->image_base.hmodule);

    // Remove the module from our list if needed
    if (remove_from_list)
    {   
        last_status = LOG_ON_ERR(REMOVE_VECTOR(module_t), &global_module_list, module);
        if (last_status) return last_status;
    }

    return last_status;
}

int mdp_dispatch_entry(module_t* module, Entry entry)
{
    int last_status = MSL_SUCCESS;
    // Ignore dispatch attempts for the initial module
    if (module == global_initial_image) return MSL_SUCCESS;

    last_status = LOG_ON_ERR(obp_dispatch_module_operation_callbacks, module, entry, true);
    if (last_status) return last_status;

    char* path;
    last_status = LOG_ON_ERR(mdp_get_image_path, module, &path);
    if (last_status) return last_status;
    last_status = LOG_ON_ERR(module->framework_initialize, global_initial_image, pp_get_framework_routine, entry, path, module);
    if (last_status) return last_status;

    last_status = LOG_ON_ERR(obp_dispatch_module_operation_callbacks, module, entry, false);
    return last_status;
}

int md_is_image_preinitialized(module_t* module, bool* preinitialized)
{
    int last_status = MSL_SUCCESS;
    *preinitialized = module->flags.is_preloaded;
    return last_status;
}

int md_unmap_image(module_t* module)
{
    int last_status = MSL_SUCCESS;
    if (module == global_initial_image) return MSL_ACCESS_DENIED;

    last_status = LOG_ON_ERR(mdp_unmap_image, module, true, true);
    return last_status;
}