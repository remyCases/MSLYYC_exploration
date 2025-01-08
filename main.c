// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/error.h"
#include "include/pe_parser.h"
#include "include/decompiler.h"
#include <inttypes.h>

int main(int argc, char** argv) 
{
    char* path = NULL;
    size_t buf_size = 0;
    char* buf = NULL;
    pe_file_t pe_file;

    int last_status = MSL_UNKNWON_ERROR;
    if (argc == 1) path = "data/StoneShard.exe";
    else path = argv[1];
 
    last_status = LOG_ERR(pe_load, path, &buf, &buf_size);
    last_status = LOG_ERR(pe_parse, &pe_file, buf, buf_size);

    last_status = LOG_ERR(decompile, pe_file);

	printf("[>] Execution complete\n");
    if (buf) free(buf);
    return 0;
}