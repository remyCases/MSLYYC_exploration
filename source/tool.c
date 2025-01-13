// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/tool.h"

rfunction_string_full_t get_index_full(rfunction_t* function, size_t index)
{
    return *(rfunction_string_full_t*)((char*)(function) + (sizeof(rfunction_string_full_t) * index));
}

rfunction_string_ref_t get_index_ref(rfunction_t* function, size_t index)
{
    return *(rfunction_string_ref_t*)((char*)(function) + (sizeof(rfunction_string_ref_t) * index));
}