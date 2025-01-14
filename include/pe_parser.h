// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef PE_PARSER_H_
#define PE_PARSER_H_

#include <stddef.h>

int pe_load(char*, char**, size_t*);
int ppi_get_module_section_bounds(void*, const char*, uint64_t*, size_t*);
#endif  /* !PE_PARSER_H_ */