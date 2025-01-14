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
#include "callback.h"
#include "runner_interface.h"
#include "../safety_hook_wrapper/include/wrapper.h"

typedef const char* str;

typedef enum EVENT_TRIGGERS EVENT_TRIGGERS;
typedef enum CM_COLOR CM_COLOR;
typedef enum MODULE_OPERATION_TYPE MODULE_OPERATION_TYPE;

typedef struct module_callback_descriptor_s module_callback_descriptor_t;
typedef struct operation_info_s operation_info_t;
typedef struct msl_inline_hook_s msl_inline_hook_t;
typedef struct msl_mid_hook_s msl_mid_hook_t;
typedef struct msl_memory_allocation_s msl_memory_allocation_t;
typedef struct module_s module_t;
typedef struct msl_interface_base_s msl_interface_base_t;
typedef struct msl_interface_s msl_interface_t;
typedef struct msl_interface_impl_s msl_interface_impl_t;
typedef struct msl_interface_table_entry_s msl_interface_table_entry_t;

typedef int(*Entry)(module_t*,const char*);
typedef int(*LoaderEntry)(module_t*, void*(*pp_get_framework_routine)(const char*), Entry, const char*, module_t*);	
typedef void(*ModuleCallback)(module_t*, MODULE_OPERATION_TYPE, operation_info_t*);

DEF_HASHMAP(str, TRoutine)
DEF_HASHMAP(str, size_t)
DEF_VECTOR(module_callback_descriptor_t)
DEF_VECTOR(module_t)

enum EVENT_TRIGGERS
{
    EVENT_OBJECT_CALL = 1,	// The event represents a Code_Execute() call.
    EVENT_FRAME = 2,		// The event represents an IDXGISwapChain::Present() call.
    EVENT_RESIZE = 3,		// The event represents an IDXGISwapChain::ResizeBuffers() call.
    EVENT_UNUSED = 4,		// This value is unused.
    EVENT_WNDPROC = 5		// The event represents a WndProc() call.
};

enum MODULE_OPERATION_TYPE
{
    OPERATION_UNKNOWN = 0,
    // The call is a ModulePreinitialize call
    OPERATION_PREINITIALIZE = 1,
    // The call is a ModuleInitialize call
    OPERATION_INITIALIZE = 2,
    // The call is a ModuleUnload call
    OPERATION_UNLOAD = 3
};

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

struct module_callback_descriptor_s
{
    module_t* owner_module;
    EVENT_TRIGGERS trigger;
    int32_t priority;
    void* routine;
};

struct operation_info_s
{
    union
    {
        uint8_t flags;
        struct
        {
            bool is_future_call;
            bool reserved;
        };
    };

    void* module_base_address;
};

struct msl_inline_hook_s
{
    module_t* owner;
    const char* identifier;
    safety_hook_inline_t hook_instance;
};

struct msl_mid_hook_s
{
    module_t* owner;
    const char* identifier;
    safety_hook_mid_t hook_instance;
};

struct msl_memory_allocation_s
{
    void* allocation_base;
    size_t allocation_size;
    module_t* owner_module;
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
    
    void(*print_warning)(const char*);

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
    HASHMAP(str, TRoutine) builtin_function_cache;

    // Cache used for lookups of builtin variables (xprevious, etc.)
    // key = name, value = index in the m_BuiltinArray
    HASHMAP(str, size_t) builtin_variable_cache;

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
    VECTOR(module_callback_descriptor_t) registered_callbacks;

    // === Internal functions ===
    int(*extract_function_entry)(msl_interface_impl_t*, size_t, char**, TRoutine*, int32_t*);
    int(*descriptor_comparator)(const void*, const void*);
    int(*sort_module_callbacks)(msl_interface_impl_t*);
    int(*create_callback_descriptor)(module_t*, EVENT_TRIGGERS, void*, int32_t, module_callback_descriptor_t*);
    int(*add_to_callback_list)(msl_interface_impl_t*, module_callback_descriptor_t*);
    int(*find_descriptor)(module_callback_descriptor_t*, module_callback_descriptor_t*);
    int(*remove_callback_from_list)(msl_interface_impl_t*, module_t*, void*);
    int(*callback_exists)(msl_interface_impl_t*, module_t*, void*);

    // TODO: DISPATCH_CALLBACKS

    int (*fetch_D3D11_info)(ID3D11Device**, IDXGISwapChain**);
    int (*determine_function_entry_size)(size_t*);
};

extern msl_interface_impl_t global_module_interface;

struct msl_interface_table_entry_s
{
    module_t* owner_module;
    const char* interface_name;
    msl_interface_base_t* intf;
};
typedef struct module_s 
{
    union
    {
        uint8_t bitfield;
        struct
        {
            // If this bit is set, the module's Initialize function has been called.
            bool is_initialized : 1;

            // If this bit is set, the module's Preload function has been called.
            // This call to Preload happens before the call to Initialize.
            // 
            // If the Aurie Framework is injected into a running process, this function is called
            // right before the call to Initialize.
            // Otherwise, this function is guaranteed to run before the main process's entrypoint.
            bool is_preloaded : 1;

            // If this bit is set, the module is marked for deletion and will be unloaded by the next
            // call to Aurie::Internal::MdpPurgeMarkedModules
            bool marked_for_purge : 1;

            // If this bit is set, the module was loaded by a MdMapImage call from another module.
            // This makes it such that its ModulePreload function never gets called.
            bool is_runtime_loaded : 1;
        };
    } flags;

    // Describes the image base (and by extent the module).
    union
    {
        HMODULE module;
        void* pointer;
        unsigned long long address;
    } image_base;

    // Specifies the image size in memory.
    uint32_t image_size;

    // The path of the loaded image.
    char* image_path;

    // The address of the Windows entrypoint of the image.
    union
    {
        void* pointer;
        unsigned long long address;
    } image_entrypoint;

    // The initialize routine for the module
    Entry module_initialize;

    // The optional preinitialize routine for the module
    Entry module_preinitialize;

    // An unload routine for the module
    Entry module_unload;

    // The __AurieFrameworkInit function
    LoaderEntry framework_initialize;

    // Interfaces exposed by the module
    msl_interface_table_entry_t* interface_table;

    // Memory allocated by the module
    // 
    // If the allocation is made in the global context (i.e. by MmAllocatePersistentMemory)
    // the allocation is put into g_ArInitialImage of the framework module.
    msl_memory_allocation_t* memory_allocations;

    // Functions hooked by the module by Mm*Hook functions
    msl_inline_hook_t* inline_hooks;
    msl_mid_hook_t* mid_hooks;

    // If set, notifies the plugin of any module actions
    ModuleCallback module_operation_callback;
};

extern VECTOR(module_t) global_module_list;

rvalue_t init_rvalue(void);
rvalue_t init_rvalue_bool(bool);
rvalue_t init_rvalue_double(double);
rvalue_t init_rvalue_i64(int64_t);
rvalue_t init_rvalue_i32(int32_t);
rvalue_t init_rvalue_instance(instance_t*);
rvalue_t init_rvalue_str(const char*);
int init_rvalue_str_interface(rvalue_t*, const char*, msl_interface_impl_t*);
#endif  /* !INTERFACE_H_ */