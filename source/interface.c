// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/error.h"
#include "../include/interface.h"
#include "../include/pe_parser.h"
#include "d3d11.h"

FUNC_HASH(str, TRoutine)
FUNC_HASH(str, size_t)

int extract_function_entry(msl_interface_impl_t* msl_interface, size_t index, const char** function_name, TRoutine* function_routine, int32_t* argument_count)
{
    rfunction_t* functions = *msl_interface->functions_array;

    if (msl_interface->function_entry_size == sizeof(rfunction_string_ref_t))
    {
        rfunction_string_ref_t function_entry = get_index_ref(functions, index);

        // Save stuff into the variables
        *function_name = function_entry.name;
        *function_routine = function_entry.routine;
        *argument_count = function_entry.argument_count;
    }
    else if (msl_interface->function_entry_size == sizeof(rfunction_string_full_t))
    {
        rfunction_string_full_t function_entry = get_index_full(functions, index);
        
        // Properly null-terminate the string
        char string_buffer[70] = { 0 };
        strncpy(string_buffer, function_entry.name, 64);

        // You know the rest...
        *function_name = string_buffer;
        *function_routine = function_entry.routine;
        *argument_count = function_entry.argument_count;
    }

	return MSL_SUCCESS;
}

int fetch_D3D11_info(msl_interface_impl_t* msl_interface, ID3D11Device** device_object, IDXGISwapChain** swapchain)
{
	int last_status = MSL_SUCCESS;

	// Call os_get_info, which gets us the necessary info
	rvalue_t os_info_ds_map;
	last_status = LOG_ON_ERR(msl_interface->intf.call_builtin_ex, msl_interface, &os_info_ds_map, "os_get_info", NULL, NULL, NULL, 0);

	// This is not checking the return value of os_get_info,
	// instead checking if we even called the function successfully.
	if (last_status) return last_status;

	// Pull everything needed from the DS List
	// We need to pass the pointer to the interface into the RValue initializer
	// here, because Aurie didn't yet put our interface in its array (we're being called from stage II init), 
	// therefore the hidden ObGetInterface calls within the RValue would fail.
	rvalue_t arg;
	init_rvalue_str_interface(&arg, "video_d3d11_device", msl_interface);
	rvalue_t args[2] = { os_info_ds_map, arg };
	rvalue_t dx_device;
	last_status = LOG_ON_ERR(msl_interface->intf.call_builtin_ex, msl_interface, &dx_device, "ds_map_find_value", NULL, NULL, args, 2);

	// This is not checking the return value of ds_map_find_value,
	// instead checking if we even called the function successfully.
	if (last_status) return MSL_OBJECT_NOT_FOUND;

	init_rvalue_str_interface(&arg, "video_d3d11_swapchain", msl_interface);
	args[1] = arg;
	rvalue_t dx_swapchain;
	last_status = LOG_ON_ERR(msl_interface->intf.call_builtin_ex, msl_interface, &dx_swapchain, "ds_map_find_value", NULL, NULL, args, 2);

	// This is not checking the return value of ds_map_find_value,
	// instead checking if we even called the function successfully.
	if (last_status) return MSL_OBJECT_NOT_FOUND;

	if (device_object)
		*device_object = (ID3D11Device*)(dx_device.pointer);

	if (swapchain)
		*swapchain = (IDXGISwapChain*)(dx_swapchain.pointer);

	return MSL_SUCCESS;
}

int determine_function_entry_size(msl_interface_impl_t* msl_interface, size_t* size)
{
	int last_status = MSL_SUCCESS;
	if (!msl_interface->functions_array) 
	{
		*size = 0;
		return MSL_SUCCESS;
	}

	rfunction_t* first_entry = *msl_interface->functions_array;
	if (!first_entry) 
	{
		*size = 0;
		return MSL_SUCCESS;
	}

	// This is either an ASCII string that I just interpreted as a pointer,
	// or it's a real  pointer into somewhere in the main executable's .rdata section.
	// If it's the latter, we know sizeof(RFunction) == 24.
	const char* potential_reference = first_entry->referential_entry.name;
	if (!potential_reference) 
	{
		*size = 0;
		return MSL_SUCCESS;
	}

	// Get the offset and size of the .rdata section
	uint64_t rdata_offset = 0;
	size_t rdata_size = 0;
	last_status = ppi_get_module_section_bounds(GetModuleHandleA(NULL), ".rdata", &rdata_offset, &rdata_size);

	if (last_status) 
	{
		*size = 0;
		return last_status;
	}

	// Get the offset and size of the .text section
	uint64_t text_offset = 0;
	size_t text_size = 0;
	last_status = ppi_get_module_section_bounds(GetModuleHandleA(NULL), ".text", &text_offset, &text_size);

	if (last_status) 
	{
		*size = 0;
		return last_status;
	}

	// The section base is returned relative to the module base
	text_offset += (uint64_t)(GetModuleHandleA(NULL));
	rdata_offset += (uint64_t)(GetModuleHandleA(NULL));

	char* rdata_section_start = (char*)(rdata_offset);
	char* rdata_section_end = (char*)(rdata_offset + rdata_size);

	// The string should be somewhere in the .rdata section
	// If it's not, it's definitely not RFunctionStringRef (or the structure is corrupt)
	if (potential_reference >= rdata_section_start && potential_reference <= rdata_section_end)
	{
		*size = sizeof(rfunction_string_ref_t);
		return MSL_SUCCESS;
	}

	char* text_section_start = (char*)(text_offset);
	char* text_section_end = (char*)(text_offset + text_size);
	char* routine = (char*)(first_entry->full_entry.routine);

	// The routine should be somewhere in the .text section
	// If it's not, it's definitely not RFunctionStringFull (or the structure is corrupt)
	
	if (routine >= text_section_start && routine <= text_section_end)
	{
		*size = sizeof(rfunction_string_full_t);
		return MSL_SUCCESS;
	}

	// Unknown size - possibly wrong runner architecture?
	*size = 0;
	return MSL_UNKNWON_ERROR;
}

int get_named_routine_pointer(msl_interface_impl_t* msl_interface, const char* function_name, void** function_pointer)
{
	// Make sure we have what we need
	if (!msl_interface->runner_interface.Code_Function_Find)
		return MSL_MODULE_INTERNAL_ERROR;

	if (!msl_interface->functions_array)
		return MSL_MODULE_INTERNAL_ERROR;

	int last_status = MSL_SUCCESS;

	// Get the index for the function
	int function_index = -1;
	last_status = msl_interface->intf.get_named_routine_index(function_name, &function_index);

	// Make sure we got one
	if (last_status) return last_status;

	// Values greater or equal to 100k are reserved for scripts.
	// Values greater or equal to 500k are reserved for extension functions.
	// Until we can deal with those, just deny access.

	if (function_index >= 100000)
	{
		// If we don't have access to scripts, deny access to both scripts and Extension functions
		// If we do have access to scripts, we deny access only to Extension Functions
		if (function_index >= 500000 || !msl_interface->get_script_data)
		{
			return MSL_ACCESS_DENIED;
		}
		
		// Get the script
		*function_pointer = msl_interface->get_script_data(function_index - 100000);
		RUNTIME_ASSERT(*function_pointer);

		return MSL_SUCCESS;
	}

	// Previous check should've tripped if the value is -1
	RUNTIME_ASSERT(function_index > 0);

	char* func_name;
	int32_t function_argument_count = 0;
	TRoutine function_routine = NULL;

	msl_interface->extract_function_entry(msl_interface, function_index, &func_name, &function_routine, &function_argument_count);

	// Something's wrong, hard to say what...
	RUNTIME_ASSERT(function_routine != NULL);

	// Function name should match? Most likely an issue with YYRunnerInterface?
	// Check Extension_PrePrepare in faulty runner
	RUNTIME_ASSERT(!stricmp(func_name, function_name));

	// Get the pointer to the function from the game array
	*function_pointer = function_routine;
	return MSL_SUCCESS;
}

int call_builtin_ex(msl_interface_impl_t* msl_interface, rvalue_t* result, const char* function_name, instance_t* self_instance, instance_t* other_instance, rvalue_t* arguments, size_t arguments_size)
{
	// Use the cached result if possible
	TRoutine function = NULL;
	int last_status = MSL_SUCCESS;
	last_status = GET_VALUE_N(str, TRoutine)(&msl_interface->builtin_function_cache, function_name, &function);
	if (last_status == MSL_SUCCESS)
	{
		function(
			result,
			self_instance,
			other_instance,
			arguments_size,
			arguments
		);

		return MSL_SUCCESS;
	}

	// Query for the function pointer
	last_status = msl_interface->intf.get_named_routine_pointer(function_name, (void*)function);

	// Make sure we found a function
	if (last_status) return last_status;

	// Previous check should've fired
	RUNTIME_ASSERT(function != NULL);

	// Cache the result
	last_status = INSERT_N(str, TRoutine)(&msl_interface->builtin_function_cache, function_name, function);
	if (last_status) return last_status;
	
	function(
		result,
		self_instance,
		other_instance,
		arguments_size,
		arguments
	);

	return MSL_SUCCESS;
}

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

int init_rvalue_str_interface(rvalue_t* rvalue, const char* value, msl_interface_impl_t* msl_interface)
{
	// Initialize it to just empty stuff
	init_rvalue(rvalue);

	// Let's not crash on invalid interfaces provided
	if (!msl_interface) return MSL_MODULE_INTERNAL_ERROR;

	// We can ignore this, because if it fails, we're just initialized to UNSET
	msl_interface->intf.string_to_rvalue(value, rvalue);

    return MSL_SUCCESS;
}