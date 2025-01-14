// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/error.h"
#include "../include/interface.h"

#define DEFAULT_CAPACITY 2
static module_callback_descriptor_array_t registered_callbacks;

int init_module_callbacks()
{
	registered_callbacks.size = 0;
	registered_callbacks.capacity = DEFAULT_CAPACITY;
	registered_callbacks.arr = (module_callback_descriptor_t*)malloc(sizeof(module_callback_descriptor_t)*registered_callbacks.capacity);

	if (!registered_callbacks.arr) return MSL_ALLOCATION_ERROR;

	return MSL_SUCCESS;
}

int free_module_callbacks()
{
	free(registered_callbacks.arr);
	return MSL_SUCCESS;
}

static int resize_module_callbacks()
{
	registered_callbacks.capacity += DEFAULT_CAPACITY;
	registered_callbacks.arr = (module_callback_descriptor_t*)realloc(registered_callbacks.arr, sizeof(module_callback_descriptor_t)*registered_callbacks.capacity);

	if (!registered_callbacks.arr) return MSL_ALLOCATION_ERROR;

	return MSL_SUCCESS;
}

static int add_module_callbacks(module_callback_descriptor_t* descriptor)
{
	int status = MSL_SUCCESS;
	if (registered_callbacks.size >= registered_callbacks.capacity)
	{
		status = resize_module_callbacks();
		if (status) return status;
	}

	registered_callbacks.arr[registered_callbacks.size] = *descriptor;
	registered_callbacks.size++;

	return status;
}

static int descriptor_comparator(const void* first, const void* second) 
{
    int32_t first_priority = ((const module_callback_descriptor_t *)first)->priority;
    int32_t second_priority  = ((const module_callback_descriptor_t *)second)->priority;
    return first_priority  - second_priority ;
}

static int sort_module_callbacks()
{
	qsort(registered_callbacks.arr, registered_callbacks.size, sizeof(module_callback_descriptor_t), descriptor_comparator);
	return MSL_SUCCESS;
}

static int find_descriptor(module_callback_descriptor_t* descriptor, module_callback_descriptor_t* element)
{
	for(size_t i = 0; i < registered_callbacks.size; i++)
	{
		if (registered_callbacks.arr[i].routine == descriptor->routine && 
			registered_callbacks.arr[i].owner_module == descriptor->owner_module &&
			registered_callbacks.arr[i].trigger == descriptor->trigger &&
			registered_callbacks.arr[i].priority == descriptor->priority)
		{
			*element = registered_callbacks.arr[i];
			return MSL_SUCCESS;
		}
	}
	element = NULL;
	return MSL_OBJECT_NOT_IN_LIST;
}

static int remove_callback_from_list(module_t* module, void* routine)
{
	if (!registered_callbacks.size)
	{
		return MSL_NULL_BUFFER;
	}

	for(size_t i = 0; i < registered_callbacks.size; i++)
	{
		if (registered_callbacks.arr[i].routine == routine && registered_callbacks.arr[i].owner_module == module)
		{
			registered_callbacks.arr[i] = registered_callbacks.arr[registered_callbacks.size - 1];
			registered_callbacks.size--;

			return sort_module_callbacks();
		}
	}

	return MSL_SUCCESS;
}

static int callback_exists(module_t* module, void* routine)
{
	for(size_t i = 0; i < registered_callbacks.size; i++)
	{
		if (registered_callbacks.arr[i].routine == routine && registered_callbacks.arr[i].owner_module == module) 
			return MSL_SUCCESS;
	}
    return MSL_OBJECT_NOT_IN_LIST;
}

static int create_callback_descriptor(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority, module_callback_descriptor_t* descriptor)
{
    descriptor->owner_module = module;
    descriptor->trigger = trigger;
    descriptor->routine = routine;
    descriptor->priority = priority;

    return MSL_SUCCESS;
}

static int add_to_callback_list(module_callback_descriptor_t* descriptor)
{
	int status = MSL_SUCCESS;

	status = add_module_callbacks(descriptor);
	if (status) return status;

	return sort_module_callbacks();
}

int create_callback(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority)
{
    int status = MSL_SUCCESS;
    if (callback_exists(module, routine) == MSL_SUCCESS) 
    {
        status = MSL_OBJECT_ALREADY_EXISTS;
        return status;
    }

	module_callback_descriptor_t callback_descriptor;
    if(create_callback_descriptor(module, trigger, routine, priority, &callback_descriptor))
	{
		status = MSL_ALLOCATION_ERROR;
        return status;
	}

    return add_to_callback_list(&callback_descriptor);
}

int remove_callback(module_t* module, void* routine)
{
	int status = MSL_SUCCESS;
	status = callback_exists(module, routine);
	if(status) return status;

	return remove_callback_from_list(module, routine);
}

int print_callback()
{
	printf("Size: %lld\n", registered_callbacks.size);
	for(size_t i = 0; i < registered_callbacks.size; i++)
	{
		printf("Routine: %p\n", registered_callbacks.arr[i].routine);
	}
	return MSL_SUCCESS;
}