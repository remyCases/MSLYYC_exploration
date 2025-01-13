// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "gml_structs.h"
#include "tool.h"
#include "utils.h"
#include "module.h"
#include "callback.h"
#include "runner_interface.h"

HASH_STR(TRoutine)

typedef enum CM_COLOR CM_COLOR;

typedef struct msl_interface_base_s msl_interface_base_t;
typedef struct msl_interface_s msl_interface_t;

enum CM_COLOR
{
    CM_BLACK = 0,
    CM_BLUE,
    CM_GREEN,
    CM_AQUA,
    CM_RED,
    CM_PURPLE,
    CM_YELLOW,
    CM_WHITE,
    CM_GRAY,
    CM_LIGHTBLUE,
    CM_LIGHTGREEN,
    CM_LIGHTAQUA,
    CM_LIGHTRED,
    CM_LIGHTPURPLE,
    CM_LIGHTYELLOW,
    CM_BRIGHTWHITE
};

struct msl_interface_base_s
{
    // Interface "constructor"
    int(*create)();
    // Interface "destructor"
    int(*destroy)();
    // Query interface version
    int(*query_version)(short* major, short* minor, short* patch);
};

struct msl_interface_s
{
    // Interface "constructor"
    int(*create)();
    // Interface "destructor"
    int(*destroy)();
    // Query interface version
    int(*query_version)(short* major, short* minor, short* patch);

    int(*get_named_routine_index)(const char* function_name, int* function_index);
    int(*get_named_routine_pointer)(const char* function_name, void** function_pointer);
    int(*get_global_instance)(instance_t** instance);
    int(*call_builtin)(const char* function_name, rvalue_t* args, size_t arg_size, rvalue_t* out);
    int(*call_builtin_ex)(rvalue_t* result, const char* function_name, instance_t* self, instance_t* other, rvalue_t* args, size_t arg_size);

    int(*create_callback)(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority);
    int(*remove_callback)(module_t* module, void* routine);
    
    int(*get_instance_member)(rvalue_t instance, const char* member_name, rvalue_t** member);
    int(*enum_instance_members)(rvalue_t instance, bool(*enum_function)(const char* member_name, rvalue_t* value));

    int(*rvalue_to_string)(rvalue_t* value, char** string);
    int(*string_to_rvalue)(char* string, rvalue_t* value);

    int(*get_runner_interface)(yyrunner_interface_t* yyrunner_interface);
    void (*invalidate_all_caches)();

    int(*get_script_data)(int index, script_t** script);
    int(*get_builtin_variable_index)(char* name, size_t* index);
    int(*get_builtin_variable_information)(size_t index, rvariable_routine_t** variable_information);
    int(*get_builtin)(char* name, instance_t* target_instance, int array_index, rvalue_t* value);
    int(*set_builtin)(char* name, instance_t* target_instance, int array_index, rvalue_t* value);
    int(*get_array_entry)(rvalue_t* value, size_t array_index, rvalue_t** array_element);
    int(*get_array_size)(rvalue_t* value, size_t* size);
    int(*get_room_data)(int32_t room_id, room_t** room);
    int(*get_current_room_data)(room_t** current_room);
    int(*get_instance_object)(int32_t instance_id, instance_t** instance);
    int(*invoke_with_object)(const rvalue_t* object, void(*method)(instance_t* self, instance_t* other));
    int(*get_variable_slot)(const rvalue_t* object, const char* variable_name, int32_t* hash);
};

FUNC_HASH_STR(TRoutine)
#endif  /* !INTERFACE_H_ */