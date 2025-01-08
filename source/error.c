// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/error.h"

char* error_str(ERROR_MLS err) 
{
    switch(err) 
    {
        MACRO_ERROR(TO_CASE_STR)
        default: return "";
    }
}

int error_print(char* func, ERROR_MLS err)
{
    switch(err) 
    {
        MACRO_ERROR(TO_CASE_PRINT, func)
        default:;
    }

    return err;
}