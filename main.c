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
    int last_status = MSL_UNKNWON_ERROR;
    if (argc == 1) path = "data/StoneShard.exe";
    else path = argv[1];
 
    last_status = ERR(pe_load, path, &buf, &buf_size);
    last_status = ERR(pe_parse, buf);

    last_status = ERR(decompile, buf, buf_size);

	printf("[>] Execution complete\n");
    if (buf) free(buf);
    return 0;
}