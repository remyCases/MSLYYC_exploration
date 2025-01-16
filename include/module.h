// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef MODULE_H_
#define MODULE_H_

#include "../include/interface.h"

int mdp_create_module(const char*, HMODULE, bool, uint8_t, module_t*);
int mdp_is_module_marked_for_purge(module_t*, bool*);
int mdp_mark_module_for_purge(module_t*);
int mdp_purge_marked_modules(void);
int mdp_map_image(const char*, HMODULE*);
int mdp_build_module_list(const char*, bool, bool(*predicate)(const char*), VECTOR(char)*);
int mdp_add_module_to_list(module_t*);
int mdp_query_module_information(HMODULE, void**, uint32_t*, void**);
int mdp_get_image_path(module_t*, char**);
int mdp_get_image_folder(module_t*, char**);
int mdp_get_next_module(module_t*, module_t**);
int mdp_get_module_base_address(module_t*, void**);
int mdp_lookup_module_by_path(const char*, module_t**);
int mdp_process_image_exports(const char*, HMODULE, module_t*);

#endif  /* !MODULE_H_ */