// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/error.h"
#include "../include/interface.h"
#include "../include/pe_parser.h"
#include "d3d11.h"

FUNC_HASH(str, TRoutine)
FUNC_HASH(str, size_t)
FUNC_VEC(module_callback_descriptor_t)
FUNC_VEC(module_t)
FUNC_VEC(msl_interface_table_entry_t) 

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
	last_status = GET_VALUE(str, TRoutine)(&msl_interface->builtin_function_cache, function_name, &function);
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
	last_status = INSERT(str, TRoutine)(&msl_interface->builtin_function_cache, function_name, function);
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

rvalue_t init_rvalue(void)
{
	rvalue_t rvalue;
	rvalue.real = 0;
	rvalue.flags = 0;
	rvalue.kind = VALUE_UNDEFINED;

    return rvalue;
}

rvalue_t init_rvalue_bool(bool value)
{
	rvalue_t rvalue;
	rvalue.real = (double)(value);
	rvalue.flags = 0;
	rvalue.kind = VALUE_BOOL;

    return rvalue;
}

rvalue_t init_rvalue_double(double value)
{
	rvalue_t rvalue;
	rvalue.real = value;
	rvalue.flags = 0;
	rvalue.kind = VALUE_REAL;

    return rvalue;
}

rvalue_t init_rvalue_i64(int64_t value)
{
	rvalue_t rvalue;
	rvalue.i64 = value;
	rvalue.flags = 0;
	rvalue.kind = VALUE_INT64;

    return rvalue;
}

rvalue_t init_rvalue_i32(int32_t value)
{
	rvalue_t rvalue;
	rvalue.i32 = value;
	rvalue.flags = 0;
	rvalue.kind = VALUE_INT32;

    return rvalue;
}

rvalue_t init_rvalue_instance(instance_t* object)
{
	rvalue_t rvalue;
	rvalue.object = object;
	rvalue.flags = 0;
	rvalue.kind = VALUE_OBJECT;

    return rvalue;
}

rvalue_t init_rvalue_str(const char* value)
{
	// Init to empty
	return *(rvalue_t*)value;
}

int init_rvalue_str_interface(rvalue_t* rvalue, const char* value, msl_interface_impl_t* msl_interface)
{
	// Initialize it to just empty stuff
	*rvalue = init_rvalue();

	// Let's not crash on invalid interfaces provided
	if (!msl_interface) return MSL_MODULE_INTERNAL_ERROR;

	// We can ignore this, because if it fails, we're just initialized to UNSET
	msl_interface->intf.string_to_rvalue(value, rvalue);

    return MSL_SUCCESS;
}

int descriptor_comparator(const void* first, const void* second) 
{
    int32_t first_priority = ((const module_callback_descriptor_t *)first)->priority;
    int32_t second_priority  = ((const module_callback_descriptor_t *)second)->priority;
    return first_priority  - second_priority ;
}

int sort_module_callbacks(msl_interface_impl_t* msl_interface)
{
	qsort(
		msl_interface->registered_callbacks.arr, 
		msl_interface->registered_callbacks.size, 
		sizeof(module_callback_descriptor_t), msl_interface->descriptor_comparator);
	return MSL_SUCCESS;
}

int find_descriptor(msl_interface_impl_t* msl_interface, module_callback_descriptor_t* descriptor, module_callback_descriptor_t* element)
{
	for(size_t i = 0; i < msl_interface->registered_callbacks.size; i++)
	{
		if (msl_interface->registered_callbacks.arr[i].routine == descriptor->routine && 
			msl_interface->registered_callbacks.arr[i].owner_module == descriptor->owner_module &&
			msl_interface->registered_callbacks.arr[i].trigger == descriptor->trigger &&
			msl_interface->registered_callbacks.arr[i].priority == descriptor->priority)
		{
			*element = msl_interface->registered_callbacks.arr[i];
			return MSL_SUCCESS;
		}
	}
	element = NULL;
	return MSL_OBJECT_NOT_IN_LIST;
}

int remove_callback_from_list(msl_interface_impl_t* msl_interface, module_t* module, void* routine)
{
	if (!msl_interface->registered_callbacks.size)
	{
		return MSL_NULL_BUFFER;
	}

	for(size_t i = 0; i < msl_interface->registered_callbacks.size; i++)
	{
		if (msl_interface->registered_callbacks.arr[i].routine == routine && msl_interface->registered_callbacks.arr[i].owner_module == module)
		{
			msl_interface->registered_callbacks.arr[i] = msl_interface->registered_callbacks.arr[msl_interface->registered_callbacks.size - 1];
			msl_interface->registered_callbacks.size--;

			return msl_interface->sort_module_callbacks(msl_interface);
		}
	}

	return MSL_SUCCESS;
}

int callback_exists(msl_interface_impl_t* msl_interface, module_t* module, void* routine)
{
	for(size_t i = 0; i < msl_interface->registered_callbacks.size; i++)
	{
		if (msl_interface->registered_callbacks.arr[i].routine == routine && msl_interface->registered_callbacks.arr[i].owner_module == module) 
			return MSL_SUCCESS;
	}
    return MSL_OBJECT_NOT_IN_LIST;
}

int create_callback_descriptor(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority, module_callback_descriptor_t* descriptor)
{
    descriptor->owner_module = module;
    descriptor->trigger = trigger;
    descriptor->routine = routine;
    descriptor->priority = priority;

    return MSL_SUCCESS;
}

int add_to_callback_list(msl_interface_impl_t* msl_interface, module_callback_descriptor_t* descriptor)
{
	int status = MSL_SUCCESS;

	status = ADD_VECTOR(module_callback_descriptor_t)(&msl_interface->registered_callbacks, descriptor);
	if (status) return status;

	return msl_interface->sort_module_callbacks(msl_interface);
}

int create_callback(msl_interface_impl_t* msl_interface, module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority)
{
    int status = MSL_SUCCESS;
    if (msl_interface->callback_exists(msl_interface, module, routine) == MSL_SUCCESS) 
    {
        status = MSL_OBJECT_ALREADY_EXISTS;
        return status;
    }

	module_callback_descriptor_t callback_descriptor;
    if(msl_interface->create_callback_descriptor(module, trigger, routine, priority, &callback_descriptor))
	{
		status = MSL_ALLOCATION_ERROR;
        return status;
	}

    return msl_interface->add_to_callback_list(msl_interface, &callback_descriptor);
}

int remove_callback(msl_interface_impl_t* msl_interface, module_t* module, void* routine)
{
	int status = MSL_SUCCESS;
	status = msl_interface->callback_exists(msl_interface, module, routine);
	if(status) return status;

	return msl_interface->remove_callback_from_list(msl_interface, module, routine);
}

int print_callback(msl_interface_impl_t* msl_interface)
{
	printf("Size: %lld\n", msl_interface->registered_callbacks.size);
	for(size_t i = 0; i < msl_interface->registered_callbacks.size; i++)
	{
		printf("Routine: %p\n", msl_interface->registered_callbacks.arr[i].routine);
	}
	return MSL_SUCCESS;
}