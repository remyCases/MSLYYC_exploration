// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef OBJECT_H_
#define OBJECT_H_

#include "error.h"
#include "interface.h"

int ob_create_interface(module_t*, interface_base_t*, const char*);
int ob_interface_exists(const char*);
int obp_dispatch_module_operation_callbacks(module_t*, Entry, bool);
int ob_get_interface(const char*, interface_base_t**);
int ob_destroy_interface(module_t*, const char*);
#endif  /* !OBJECT_H_ */