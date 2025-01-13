// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef PE_PARSER_H_
#define PE_PARSER_H_

#include <stddef.h>

typedef enum FILE_TYPE_ {
    PE_FILE,
    OTHER,
} FILE_TYPE;
typedef struct pe_file_s {
    char* buf;
    size_t buf_size;
    FILE_TYPE file_type;
} pe_file_t;

int pe_load(char*, char**, size_t*);
int ppi_get_module_section_bounds(void*, const char*, uint64_t*, size_t*);
#endif  /* !PE_PARSER_H_ */