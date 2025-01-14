// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/object.h"

int obp_lookup_interface_owner(const char* interface_name, bool case_insensitive, module_t** module, msl_interface_table_entry_t* table_entry)
{
    // Loop every single module
    for (auto& loaded_module : global_module_list)
    {
        // Check if we found it in this module
        auto iterator = std::find_if(
            loaded_module.InterfaceTable.begin(),
            loaded_module.InterfaceTable.end(),
            [case_insensitive, InterfaceName](const AurieInterfaceTableEntry& entry) -> bool
            {
                // Do a case insensitive comparison if needed
                if (case_insensitive)
                {
                    return !stricmp(entry.InterfaceName, InterfaceName);
                }

                return !strcmp(entry.InterfaceName, InterfaceName);
            }
        );

        // We found the interface in the current module!
        if (iterator != std::end(loaded_module.InterfaceTable))
        {
            module = &loaded_module;
            table_entry = &(*iterator);
            return MSL_SUCCESS;
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

    last_status = obp_lookup_interface_owner(interface_name, true, owner_module, interface_entry);
    if (last_status) return last_status;

    *msl_interface = interface_entry->intf;
    return MSL_SUCCESS;
}