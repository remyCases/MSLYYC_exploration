// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

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

int pe_load(char* file, char** buffer, size_t* buffer_size);
int pe_parse(pe_file_t* pe_file, char *buffer, size_t buffer_size);