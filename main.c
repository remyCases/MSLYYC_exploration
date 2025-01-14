// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <Zydis/Zydis.h>
#include "include/error.h"
#include "include/pe_parser.h"
#include "include/callback.h"

int main(int argc, char** argv)
{
    char* path = NULL;
    size_t buf_size = 0;
    char* buf = NULL;
    module_t module;
    int last_status = MSL_UNKNWON_ERROR;

    last_status = LOG_ON_ERR(init_module_callbacks);
    if (argc == 1) path = "data/StoneShard.exe";
    else path = argv[1];
 
    last_status = LOG_ON_ERR(pe_load, path, &buf, &buf_size);

    last_status = LOG_ON_ERR(create_callback, &module, EVENT_OBJECT_CALL, NULL, 0);
    last_status = LOG_ON_ERR(create_callback, &module, EVENT_OBJECT_CALL, (void*)1, 1);
    last_status = LOG_ON_ERR(create_callback, &module, EVENT_OBJECT_CALL, (void*)2, 2);
    print_callback();
    last_status = LOG_ON_ERR(remove_callback, &module, (void*)2);
    print_callback();
    last_status = LOG_ON_ERR(remove_callback, &module, (void*)0);
    print_callback();
    last_status = LOG_ON_ERR(remove_callback, &module, (void*)1);

	printf("[>] Execution complete\n");
    if (buf) free(buf);
    last_status = LOG_ON_ERR(free_module_callbacks);
    return 0;
}