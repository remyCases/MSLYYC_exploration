// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/module.h"
#include "../include/error.h"
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
            last_status = LOG_ON_ERR(mdp_unmap_image, &module, false, false);
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
    last_status = MdpLookupModuleByPath(image_path, potential_loaded_copy);
    
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

int mdp_get_module_base_address(module_t* module, void** ptr)
{
    int last_status = MSL_SUCCESS;
    *ptr = module->image_base.pointer;
    return last_status;
}