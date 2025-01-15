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
int mdp_map_image(const char*, HMODULE**);
int mdp_get_module_base_address(module_t*, void**);

#endif  /* !MODULE_H_ */