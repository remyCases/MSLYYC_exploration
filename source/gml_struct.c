// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdbool.h>
#include "../include/gml_structs.h"
#include "../include/interface.h"

int init_rvalue(rvalue_t* rvalue)
{
	rvalue->real = 0;
	rvalue->flags = 0;
	rvalue->kind = VALUE_UNDEFINED;

    return MSL_SUCCESS;
}

int init_rvalue_bool(rvalue_t* rvalue, bool value)
{
	rvalue->real = (double)(value);
	rvalue->flags = 0;
	rvalue->kind = VALUE_BOOL;

    return MSL_SUCCESS;
}

int init_rvalue_double(rvalue_t* rvalue, double value)
{
	rvalue->real = value;
	rvalue->flags = 0;
	rvalue->kind = VALUE_REAL;

    return MSL_SUCCESS;
}

int init_rvalue_i64(rvalue_t* rvalue, int64_t value)
{
	rvalue->i64 = value;
	rvalue->flags = 0;
	rvalue->kind = VALUE_INT64;

    return MSL_SUCCESS;
}

int init_rvalue_i32(rvalue_t* rvalue, int32_t value)
{
	rvalue->i32 = value;
	rvalue->flags = 0;
	rvalue->kind = VALUE_INT32;

    return MSL_SUCCESS;
}

int init_rvalue_instance(rvalue_t* rvalue, instance_t* object)
{
	rvalue->object = object;
	rvalue->flags = 0;
	rvalue->kind = VALUE_OBJECT;

    return MSL_SUCCESS;
}

int init_rvalue_str(rvalue_t** rvalue, const char* value)
{
	// Init to empty
	*rvalue = (rvalue_t*)value;

    return MSL_SUCCESS;
}

int init_rvalue_str_interface(rvalue_t* rvalue, const char* value, msl_interface_t* msl_interface)
{
	// Initialize it to just empty stuff
	init_rvalue(rvalue);

	// Let's not crash on invalid interfaces provided
	if (!msl_interface) return;

	// We can ignore this, because if it fails, we're just initialized to UNSET
	msl_interface->string_to_rvalue(value, rvalue);

    return MSL_SUCCESS;
}