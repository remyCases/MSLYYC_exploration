// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef PE_PARSER_H_
#define PE_PARSER_H_

#include "interface.h"

int pp_query_image_architecture(const char*, unsigned short*);
int pp_find_file_export_by_name(const char*, const char*, uintptr_t*);
int pp_get_framework_routine(const char*, void**);
int pp_get_image_subsystem(void*, unsigned short*);
int pp_get_current_architecture(unsigned short*);
int ppi_map_file_to_memory_alloc(const char*, void**, size_t*);
int ppi_query_image_architecture(void*, unsigned short*);
int ppi_find_module_export_by_name(const module_t*, const char*, void**);
int ppi_get_nt_header(void*, void**);
int ppi_get_module_section_bounds(void*, const char*, uint64_t*, size_t*);
int ppi_get_export_offset(void*, const char*, uintptr_t*);
int ppi_rva_to_file_offset(PIMAGE_NT_HEADERS, uint32_t, uint32_t*);
int ppi_rva_to_file_offset64(PIMAGE_NT_HEADERS64, uint32_t, uint32_t*);
int ppi_rva_to_file_offset32(PIMAGE_NT_HEADERS32, uint32_t, uint32_t*);

#endif  /* !PE_PARSER_H_ */