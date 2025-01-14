// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/object.h"
#include "../include/interface.h"

VECTOR(module_t) global_module_list;

int obp_lookup_interface_owner(const char* interface_name, bool case_insensitive, module_t** module, msl_interface_table_entry_t** table_entry)
{
    module_t* loaded_module = NULL;
    // Loop every single module
    for (size_t i = 0; i < global_module_list.size; i++)
    {
        loaded_module = &global_module_list.arr[i];

        if (case_insensitive)
        {
            if(!stricmp(loaded_module->interface_table->interface_name, interface_name))
            {
                *module = loaded_module;
                *table_entry = loaded_module->interface_table;
                return MSL_SUCCESS;
            }
        }
        else
        {
            if(!strcmp(loaded_module->interface_table->interface_name, interface_name))
            {
                *module = loaded_module;
                *table_entry = loaded_module->interface_table;
                return MSL_SUCCESS;
            }
        }
    }
    // We didn't find any interface with that name.
    return MSL_OBJECT_NOT_FOUND;
}

int ob_get_interface(const char* interface_name, msl_interface_base_t** msl_interface)
{
    int last_status = MSL_SUCCESS;
    module_t* owner_module = NULL;
    msl_interface_table_entry_t* interface_entry = NULL;

    last_status = LOG_ON_ERR(obp_lookup_interface_owner, interface_name, true, &owner_module, &interface_entry);
    if (last_status) return last_status;

    *msl_interface = interface_entry->intf;
    return MSL_SUCCESS;
}