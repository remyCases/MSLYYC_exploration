// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef TOOL_H_
#define TOOL_H_

#include <stdint.h>
#include <stdbool.h>
#include "utils.h"
#include "gml_structs.h"

typedef struct rfunction_s rfunction_t;
typedef struct rfunction_string_ref_s rfunction_string_ref_t;
typedef struct rfunction_string_full_s rfunction_string_full_t;

struct rfunction_string_full_s
{
    char name[64];
    TRoutine routine;
    int32_t argument_count;
    int32_t usage_count;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(rfunction_string_full_t) == 80);
#endif // _WIN64

struct rfunction_string_ref_s
{
    const char* name;
    TRoutine routine;
    int32_t argument_count;
    int32_t usage_count;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(rfunction_string_ref_t) == 24);
#endif // _WIN64

struct rfunction_s
{
    union
    {
        rfunction_string_ref_t referential_entry;
        rfunction_string_full_t full_entry;
    };
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(rfunction_t) == 80);
#endif // _WIN64

rfunction_string_full_t get_index_full(rfunction_t* function, size_t index);
rfunction_string_ref_t get_index_ref(rfunction_t* function, size_t index);
#endif  /* !TOOL_H_ */