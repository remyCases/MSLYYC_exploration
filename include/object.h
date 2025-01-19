// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef OBJECT_H_
#define OBJECT_H_

#include "interface.h"

int ob_create_interface(module_t*, interface_base_t*, const char*);
int ob_interface_exists(const char*);
int obp_destroy_interface_by_name(const char*);
int obp_lookup_interface_owner_export(const char*, const char*, void**);
int obp_get_object_type(base_object_t*, int*);
int obp_set_module_operation_callback(module_t*, ModuleCallback);
int obp_dispatch_module_operation_callbacks(module_t*, Entry, bool);
int obp_add_interface_to_table(module_t*, interface_table_entry_t*);
int obp_create_operation_info(module_t*, bool, operation_info_t*);
int obp_destroy_interface(module_t*, interface_base_t*, bool, bool);
int obp_lookup_interface_owner(const char*, bool, module_t**, interface_table_entry_t**);
int ob_get_interface(const char*, interface_base_t**);
int ob_destroy_interface(module_t*, const char*);

#endif  /* !OBJECT_H_ */