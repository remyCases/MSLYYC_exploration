// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/object.h"
#include "../include/interface.h"

VECTOR(module_t) global_module_list;

int ob_create_interface(module_t*, msl_interface_base_t*, const char*);
int ob_interface_exists(const char*);
int obp_destroy_interface_by_name(const char*);
int obp_add_interface_to_table(module_t*, msl_interface_table_entry_t*);
int obp_destroy_interface(module_t*, msl_interface_base_t*, bool, bool);
int obp_lookup_interface_owner(const char*, bool, module_t**, msl_interface_table_entry_t**);

int ob_create_interface(module_t* module, msl_interface_base_t* interface_base, const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    last_status = LOG_ON_ERR(ob_interface_exists, interface_name);
    if (last_status) return MSL_OBJECT_ALREADY_EXISTS;

    msl_interface_table_entry_t table_entry = {
        .intf = interface_base,
        .interface_name = interface_name,
        .owner_module = module,
    };

    // Make sure the interface knows it's being set up,
    // and that it succeeds at doing so. We don't want an
    // uninitialized, half-broken interface exposed!

    last_status = LOG_ON_ERR(interface_base->create);
    if (last_status) return last_status;

    last_status = LOG_ON_ERR(obp_add_interface_to_table, module, &table_entry);
    return last_status;
}

int ob_interface_exists(const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    module_t* containing_module = NULL;
    msl_interface_table_entry_t* table_entry = NULL;
    
    // If we find a module containing the interface, that means the interface exists!
    // ObpLookupInterfaceOwner will return AURIE_INTERFACE_NOT_FOUND if it doesn't exist.
    last_status = LOG_ON_ERR(obp_lookup_interface_owner, interface_name, true, &containing_module, &table_entry);
    return last_status;
}

int obp_destroy_interface_by_name(const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    module_t* owner_module = NULL;
    msl_interface_table_entry_t* table_entry = NULL;

    last_status = LOG_ON_ERR(obp_lookup_interface_owner, interface_name, true, &owner_module, &table_entry);
    if (last_status) return last_status;

    last_status = LOG_ON_ERR(obp_destroy_interface, owner_module, table_entry->intf, true, true);
    return last_status;
}

int obp_add_interface_to_table(module_t* module, msl_interface_table_entry_t* entry)
{
    int last_status = MSL_SUCCESS;
    last_status = LOG_ON_ERR(ADD_VECTOR(msl_interface_table_entry_t), &module->interface_table, entry);
    return last_status;
}

int obp_destroy_interface(module_t* module, msl_interface_base_t* interface_base, bool notify, bool remove_from_list)
{
    int last_status = MSL_SUCCESS;
    if (notify)
    {
        last_status = LOG_ON_ERR(interface_base->destroy);
        if (last_status) return last_status;
    }

    if (remove_from_list)
    {
        size_t new_size = module->interface_table.size;
        for(size_t i = 0; i < module->interface_table.size; i++)
        {
            if (module->interface_table.arr[i].intf == interface_base)
            {
                module->interface_table.arr[i] = module->interface_table.arr[new_size - 1];
                new_size--;

                if (!new_size) break;
            }
        }
        module->interface_table.size = new_size;
    }

    return last_status;
}

int obp_lookup_interface_owner(const char* interface_name, bool case_insensitive, module_t** module, msl_interface_table_entry_t** table_entry)
{
    module_t* loaded_module = NULL;
    // Loop every single module
    for (size_t i = 0; i < global_module_list.size; i++)
    {
        loaded_module = &global_module_list.arr[i];

        if (case_insensitive)
        {
            if(!stricmp(loaded_module->interface_table.arr->interface_name, interface_name))
            {
                *module = loaded_module;
                *table_entry = loaded_module->interface_table.arr;
                return MSL_SUCCESS;
            }
        }
        else
        {
            if(!strcmp(loaded_module->interface_table.arr->interface_name, interface_name))
            {
                *module = loaded_module;
                *table_entry = loaded_module->interface_table.arr;
                return MSL_SUCCESS;
            }
        }
    }
    // We didn't find any interface with that name.
    return MSL_OBJECT_NOT_FOUND;
}

int ob_get_interface(const char* interface_name, msl_interface_base_t** interface_base)
{
    int last_status = MSL_SUCCESS;
    module_t* owner_module = NULL;
    msl_interface_table_entry_t* interface_entry = NULL;

    last_status = LOG_ON_ERR(obp_lookup_interface_owner, interface_name, true, &owner_module, &interface_entry);
    if (last_status) return last_status;

    *interface_base = interface_entry->intf;
    return MSL_SUCCESS;
}