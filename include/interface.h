// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "Windows.h"
#include "dxgi.h"
#include "d3d11.h"
#include "gml_structs.h"
#include "tool.h"
#include "utils.h"
#include "module.h"
#include "callback.h"
#include "runner_interface.h"

typedef const char* str;
HASH(str, TRoutine)
HASH(str, size_t)

typedef enum CM_COLOR CM_COLOR;

typedef struct msl_interface_base_s msl_interface_base_t;
typedef struct msl_interface_s msl_interface_t;
typedef struct msl_interface_impl_s msl_interface_impl_t;

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
    int(*call_builtin_ex)(msl_interface_impl_t*, rvalue_t*, const char*, instance_t*, instance_t*, rvalue_t*, size_t);

    int(*create_callback)(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority);
    int(*remove_callback)(module_t* module, void* routine);
    
    int(*get_instance_member)(rvalue_t instance, const char* member_name, rvalue_t** member);
    int(*enum_instance_members)(rvalue_t instance, bool(*enum_function)(const char* member_name, rvalue_t* value));

    int(*rvalue_to_string)(rvalue_t* value, char** string);
    int(*string_to_rvalue)(const char* string, rvalue_t* value);

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

struct msl_interface_impl_s
{
    msl_interface_t intf;

    // Dictates whether the first stage of initializing completed already.
    bool first_init_complete;

    // Dictates whether the second stage of initializing completed already.
    bool second_init_complete;

    // The runner interface stolen by disassembling Extension_PrePrepare(),
    // alternatively found by reconstructing the stack (in older games).
    yyrunner_interface_t runner_interface;

    bool is_yyc_runner;

    // Set to true if the midfunction hook is used to get the runner interface.
    // If it is, the runner interface is unavailable during stage 1 init.
    bool is_using_mid_function_hook;

    // A handle to an event which is signaled by the mid-function hook.
    // Once signaled, the runner interface has been populated with values.
    HANDLE runner_interface_populated_event;

    // The instruction pointer of the Extension_PrePrepare breakpoint.
    void* exception_rip;

    // Original bytes of Extension_PrePrepare's JS instruction.
    // Only used if VEH is used.
    uint8_t* extension_patch_bytes;

    // The base address of the JS instruction.
    // Only used if VEH is used.
    void* extension_patch_base;

    // A pointer to the functions array in memory
    rfunction_t** functions_array;

    // A pointer to the Script_Data() engine function.
    FNScriptData get_script_data;

    // A pointer to the Room_Data() engine function.
    FNRoomData get_room_data;

    // A pointer to the pointer to the running room
    room_t** run_room;

    // Cache used for lookups of builtin functions (room_goto, etc.)
    // key = name, value = function pointer
    HASHMAP_TYPE(str, TRoutine) builtin_function_cache;

    // Cache used for lookups of builtin variables (xprevious, etc.)
    // key = name, value = index in the m_BuiltinArray
    HASHMAP_TYPE(str, size_t) builtin_variable_cache;

    // D3D11 stuff
    IDXGISwapChain* engine_swapchain;
    HWND window_handle;

    // The size of one entry in the RFunction array
    // GameMaker LTS still uses a 64-byte char array in the RFunction struct directly
    // New runners (2023.8) use a const char* in the array
    size_t function_entry_size;

    // Array of up to 500 builtins
    rvariable_routine_t* builtin_array;
    int* builtin_count;

    // Needed for RValue array access
    // RValue* actual_array = (RValue**)(RValue.m_Pointer)[this_value / sizeof(RValue*)];
    int64_t rvalue_array_offset;

    // Used to add or set a named value in a YYObjectBase structure.
    PFN_YYObjectBaseAdd add_to_yyobject_base;

    // Used to turn a name into a hash to use in lookups in the internal hashmap.
    // While accessible through variable_struct_get_hash in newer runners,
    // some don't have the function.
    // In case the name isn't in the hashmap, the function allocates a new slot for it,
    // effectively creating the variable inside the object.
    PFN_FindAllocSlot find_alloc_slot;

    // Stores plugin callbacks
    module_callback_descriptor_t* registered_callbacks;

    // === Internal functions ===
    int(*extract_function_entry)(msl_interface_impl_t*, size_t, char**, TRoutine*, int32_t*);
    int(*create_callback_descriptor)(module_t* module, EVENT_TRIGGERS trigger, void* routine, int32_t priority, module_callback_descriptor_t* descriptor);
    int(*add_to_callback_list)(module_callback_descriptor_t* descriptor);
    int(*find_descriptor)(module_callback_descriptor_t* descriptor, module_callback_descriptor_t* element);
    int(*remove_callback_from_list)(module_t* module, void* routine);
    int(*callback_exists)(module_t* module, void* routine);;

    // TODO: DISPATCH_CALLBACKS

    int (*fetch_D3D11_info)(ID3D11Device** device_object, IDXGISwapChain** swapchain);
    int (*determine_function_entry_size)(size_t* size);
};

extern msl_interface_impl_t global_module_interface;

int init_rvalue(rvalue_t*);
int init_rvalue_bool(rvalue_t*, bool);
int init_rvalue_double(rvalue_t*, double value);
int init_rvalue_i64(rvalue_t*, int64_t);
int init_rvalue_i32(rvalue_t*, int32_t);
int init_rvalue_instance(rvalue_t*, instance_t*);
int init_rvalue_str(rvalue_t**, const char*);
int init_rvalue_str_interface(rvalue_t*, const char*, msl_interface_impl_t*);
#endif  /* !INTERFACE_H_ */