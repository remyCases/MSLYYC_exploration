// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/error.h"

char* error_str(int status) 
{
    switch(status) 
    {
        MACRO_ERROR(TO_CASE_STR)
        default: return "";
    }
}

int error_print(char* function_name, char* nfile, int nline, int status)
{
    switch(status) 
    {
        MACRO_ERROR(TO_CASE_PRINT, function_name, nfile, nline)
        default:;
    }

    return status;
}