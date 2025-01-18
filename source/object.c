// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/object.h"
#include "../include/interface.h"
#include "../include/module.h"

VECTOR(module_t) global_module_list;

int obp_destroy_interface_by_name(const char*);
int obp_lookup_interface_owner_export(const char*, const char*, void**);
int obp_get_object_type(base_object_t*, int*);
int obp_set_module_operation_callback(module_t*, ModuleCallback);
int obp_add_interface_to_table(module_t*, interface_table_entry_t*);
int obp_create_operation_info(module_t* module, bool is_future_call, operation_info_t* operation_information);
int obp_destroy_interface(module_t*, interface_base_t*, bool, bool);
int obp_lookup_interface_owner(const char*, bool, module_t**, interface_table_entry_t**);

int ob_create_interface(module_t* module, interface_base_t* interface_base, const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL_CUSTOM_ERROR(ob_interface_exists, MSL_OBJECT_ALREADY_EXISTS, interface_name);

    interface_table_entry_t table_entry = {
        .intf = interface_base,
        .interface_name = interface_name,
        .owner_module = module,
    };

    // Make sure the interface knows it's being set up,
    // and that it succeeds at doing so. We don't want an
    // uninitialized, half-broken interface exposed!

    CHECK_CALL(interface_base->create);

    CHECK_CALL(obp_add_interface_to_table, module, &table_entry);
    return last_status;
}

int ob_interface_exists(const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    module_t* containing_module = NULL;
    interface_table_entry_t* table_entry = NULL;
    
    // If we find a module containing the interface, that means the interface exists!
    // ObpLookupInterfaceOwner will return AURIE_INTERFACE_NOT_FOUND if it doesn't exist.
    CHECK_CALL(obp_lookup_interface_owner, interface_name, true, &containing_module, &table_entry);
    return last_status;
}

int obp_destroy_interface_by_name(const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    module_t* owner_module = NULL;
    interface_table_entry_t* table_entry = NULL;

    CHECK_CALL(obp_lookup_interface_owner, interface_name, true, &owner_module, &table_entry);
    CHECK_CALL(obp_destroy_interface, owner_module, table_entry->intf, true, true);
    return last_status;
}

int obp_lookup_interface_owner_export(const char* interface_name, const char* export_name, void** export_address)
{
    int last_status = MSL_SUCCESS;
    module_t* interface_owner = NULL;
    interface_table_entry_t* table_entry = NULL;

    // First, look up the interface owner AurieModule
    CHECK_CALL(obp_lookup_interface_owner, interface_name, true, &interface_owner, &table_entry);

    // Now, get the module base address
    void* module_base_address = NULL;
    // Module has no base address?
    CHECK_CALL_CUSTOM_ERROR(mdp_get_module_base_address, MSL_FILE_PART_NOT_FOUND, interface_owner, module_base_address);

    // Get the thing
    void* procedure_address = GetProcAddress((HMODULE)(module_base_address), export_name);

    // No export with that name...
    if (!procedure_address) return MSL_OBJECT_NOT_FOUND;

    *export_address = procedure_address;
    return last_status;
}

int obp_get_object_type(base_object_t* base_object, int* object_type)
{
    int last_status = MSL_SUCCESS;
    *object_type = base_object->get_object_type();
    return last_status;
}

int obp_set_module_operation_callback(module_t* module, ModuleCallback callback_routine)
{
    int last_status = MSL_SUCCESS;
    module->module_operation_callback = callback_routine;
    return last_status;
}

int obp_dispatch_module_operation_callbacks(module_t* affected_module, Entry routine, bool is_future_call)
{
    // Determine the operation type
    // Yes I know, this is ugly, if you know a better solution
    // feel free to PR / tell me.
    int last_status = MSL_SUCCESS;
    MODULE_OPERATION_TYPE current_operation_type = OPERATION_UNKNOWN;

    if (routine == affected_module->module_preinitialize)
        current_operation_type = OPERATION_PREINITIALIZE;
    else if (routine == affected_module->module_initialize)
        current_operation_type = OPERATION_INITIALIZE;
    else if (routine == affected_module->module_unload)
        current_operation_type = OPERATION_UNLOAD;
    
    operation_info_t operation_information;
    CHECK_CALL(obp_create_operation_info, affected_module, is_future_call, &operation_information);

    module_t* loaded_module;
    for (size_t i = 0; i < global_module_list.size; i++)
    {
        loaded_module = &global_module_list.arr[i];
        if (!loaded_module->module_operation_callback) continue;

        CHECK_CALL(loaded_module->module_operation_callback, affected_module, current_operation_type, &operation_information);
    }
    return last_status;
}

int obp_add_interface_to_table(module_t* module, interface_table_entry_t* entry)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ADD_VECTOR(interface_table_entry_t), &module->interface_table, entry);
    return last_status;
}

int obp_create_operation_info(module_t* module, bool is_future_call, operation_info_t* operation_information)
{
    int last_status = MSL_SUCCESS;
    operation_information->is_future_call = is_future_call;
    CHECK_CALL(mdp_get_module_base_address, module, &operation_information->module_base_address);
    return last_status;
}

int obp_destroy_interface(module_t* module, interface_base_t* interface_base, bool notify, bool remove_from_list)
{
    int last_status = MSL_SUCCESS;
    if (notify)
    {
        CHECK_CALL(interface_base->destroy);
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

int obp_lookup_interface_owner(const char* interface_name, bool case_insensitive, module_t** module, interface_table_entry_t** table_entry)
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

int ob_get_interface(const char* interface_name, interface_base_t** interface_base)
{
    int last_status = MSL_SUCCESS;
    module_t* owner_module = NULL;
    interface_table_entry_t* interface_entry = NULL;

    CHECK_CALL(obp_lookup_interface_owner, interface_name, true, &owner_module, &interface_entry);

    *interface_base = interface_entry->intf;
    return last_status;
}

int ob_destroy_interface(module_t* module, const char* interface_name)
{
    int last_status = MSL_SUCCESS;
    module_t* owner_module = NULL;
    interface_table_entry_t* table_entry = NULL;

    CHECK_CALL(obp_lookup_interface_owner, interface_name, true, &owner_module, &table_entry);

    if (owner_module != module) return MSL_ACCESS_DENIED;

    CHECK_CALL(obp_destroy_interface, module, table_entry->intf, true, true);
    return last_status;
}