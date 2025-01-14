// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef OBJECT_H_
#define OBJECT_H_

#include "error.h"
#include "interface.h"

int ob_get_interface(const char* interface_name, msl_interface_base_t** msl_interface);

#endif  /* !OBJECT_H_ */