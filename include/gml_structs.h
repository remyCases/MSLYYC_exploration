// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remy_Cases/MSLYYC_exploration.

#ifndef GML_STRUCTS_H_
#define GML_STRUCTS_H_

#include <stdint.h>
#include <stdbool.h>
#include "utils.h"

typedef enum EJSRetValBool EJSRetValBool;
typedef enum YYOBJECT_KIND YYOBJECT_KIND;
typedef enum RVALUE_TYPE RVALUE_TYPE;

typedef struct rvalue_s rvalue_t;
typedef struct weak_ref_s weak_ref_t;
typedef struct physics_object_s physics_object_t;
typedef struct skeleton_instance_s skeleton_instance_t;
typedef struct yygml_functions_s yygml_functions_t;
typedef struct code_s code_t;
typedef struct physics_data_gm_s physics_data_gm_t;
typedef struct event_s event_t;
typedef struct instance_s instance_t;
typedef struct rtoken_s rtoken_t;
typedef struct with_skeleton_mask_s with_skeleton_mask_t;
typedef struct sequence_instance_only_s sequence_instance_only_t;
typedef struct members_only_s members_only_t;
typedef struct yyobject_base_s yyobject_base_t;
typedef struct object_gm_s object_gm_t;
typedef struct members_only_s members_only_t;
typedef struct instance_internal_s instance_internal_t;
typedef struct yyrect_s yyrect_t;
typedef struct instance_base_s instance_base_t;

typedef void(*PFUNC_RAW)();
typedef rvalue_t(*PFUNC_YYGMLScript)(instance_t* self, instance_t* other, rvalue_t* result, int argument_count, rvalue_t** Arguments);
typedef void(*PFUNC_YYGML)(instance_t* self, instance_t* other);
typedef void(*FNGetOwnProperty)(yyobject_base_t* object, rvalue_t* result, const char* name);
typedef void(*FNDeleteProperty)(yyobject_base_t* object, rvalue_t* result, const char* name, bool throw_on_error);
typedef EJSRetValBool(*FNDefineOwnProperty)(yyobject_base_t* object, const char* name, rvalue_t* result, bool throw_on_error);

HASH_PTR(int32_t, rvalue_t)
HASH_PTR(int, object_gm_t)
HASH_PTR(int, event_t)

LINKEDLIST(instance_t)
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(LINKEDLIST_TYPE(instance_t)) == 0x18);
#endif // _WIN64

enum EJSRetValBool
{
    EJSRVB_FALSE,
    EJSRVB_TRUE,
    EJSRVB_TYPE_ERROR
};

enum YYOBJECT_KIND
{
    OBJECT_KIND_YYOBJECTBASE = 0,
    OBJECT_KIND_CINSTANCE,
    OBJECT_KIND_ACCESSOR,
    OBJECT_KIND_SCRIPTREF,
    OBJECT_KIND_PROPERTY,
    OBJECT_KIND_ARRAY,
    OBJECT_KIND_WEAKREF,
    OBJECT_KIND_CONTAINER,
    OBJECT_KIND_SEQUENCE,
    OBJECT_KIND_SEQUENCEINSTANCE,
    OBJECT_KIND_SEQUENCETRACK,
    OBJECT_KIND_SEQUENCECURVE,
    OBJECT_KIND_SEQUENCECURVECHANNEL,
    OBJECT_KIND_SEQUENCECURVEPOINT,
    OBJECT_KIND_SEQUENCEKEYFRAMESTORE,
    OBJECT_KIND_SEQUENCEKEYFRAME,
    OBJECT_KIND_SEQUENCEKEYFRAMEDATA,
    OBJECT_KIND_SEQUENCEEVALTREE,
    OBJECT_KIND_SEQUENCEEVALNODE,
    OBJECT_KIND_SEQUENCEEVENT,
    OBJECT_KIND_NINESLICE,
    OBJECT_KIND_FILTERHOST,
    OBJECT_KIND_EFFECTINSTANCE,
    OBJECT_KIND_SKELETON_SKIN,
    OBJECT_KIND_AUDIOBUS,
    OBJECT_KIND_AUDIOEFFECT,
    OBJECT_KIND_MAX
};

enum RVALUE_TYPE
{
    VALUE_REAL = 0,				// Real value
    VALUE_STRING = 1,			// String value
    VALUE_ARRAY = 2,			// Array value
    VALUE_PTR = 3,				// Ptr value
    VALUE_VEC3 = 4,				// Vec3 (x,y,z) value (within the RValue)
    VALUE_UNDEFINED = 5,		// Undefined value
    VALUE_OBJECT = 6,			// YYObjectBase* value 
    VALUE_INT32 = 7,			// Int32 value
    VALUE_VEC4 = 8,				// Vec4 (x,y,z,w) value (allocated from pool)
    VALUE_VEC44 = 9,			// Vec44 (matrix) value (allocated from pool)
    VALUE_INT64 = 10,			// Int64 value
    VALUE_ACCESSOR = 11,		// Actually an accessor
    VALUE_NULL = 12,			// JS Null
    VALUE_BOOL = 13,			// Bool value
    VALUE_ITERATOR = 14,		// JS For-in Iterator
    VALUE_REF = 15,				// Reference value (uses the ptr to point at a RefBase structure)
    VALUE_UNSET = 0x0ffffff		// Unset value (never initialized)
};

struct yygml_functions_s
{
    const char* name;
    union
    {
        PFUNC_YYGMLScript script_function;
        PFUNC_YYGML code_function;
        PFUNC_RAW raw_function;
    };
    void* function_variables; // YYVAR
};

struct rvalue_s
{
    union
    {
        int32_t i32;
        int64_t i64;
        double real;

        instance_t* object;
        void* pointer;
    };

    unsigned int flags;
    RVALUE_TYPE kind;
};

struct rtoken_s
{
    int kind;
    unsigned int type;
    int ind;
    int ind2;
    rvalue_t value;
    int item_number;
    rtoken_t* items;
    int position;
};

struct code_s
{
    int (**_vptr_code)(void);
    code_t* next;
    int kind;
    int compiled;
    const char* str;
    rtoken_t token;
    rvalue_t value;
    void* vm_instance;
    void* vm_debug_info;
    char* code;
    const char* name;
    int code_index;
    yygml_functions_t* functions;
    bool watch;
    int offset;
    int locals_count;
    int args_count;
    int flags;
    yyobject_base_t* prototype;
};

struct physics_data_gm_s
{
    float* physics_vertices;
    bool is_physics_object;
    bool is_physics_sensor;
    bool is_physics_awake;
    bool is_physics_kinematic;
    int physics_shape;
    int physics_group;
    float physics_density;
    float physics_restitution;
    float physics_linear_damping;
    float physics_angular_damping;
    float physics_friction;
    int physics_vertex_count;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(physics_data_gm_t) == 0x30);
#endif // _WIN64

struct event_s
{
    code_t* code;
    int32_t owner_object_id;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(event_t) == 0x10);
#endif // _WIN64

struct instance_base_s
{
    rvalue_t* yyvars;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(instance_base_t) == 0x08); // 8 bits less since no virtual function (8 bits on 64bits)
#endif // _WIN64

struct yyobject_base_s
{
    instance_base_t instance_base;
    yyobject_base_t* flink;
    yyobject_base_t* blink;
    yyobject_base_t* prototype;
    const char* class_name;
    FNGetOwnProperty get_own_property;
    FNDeleteProperty delete_property;
    FNDefineOwnProperty define_own_property;
    HASHMAP_TYPE_PTR(int32_t, rvalue_t)* yyvars_map;
    weak_ref_t** weak_ref;
    uint32_t weak_ref_count;
    uint32_t variable_count;
    uint32_t flags;
    uint32_t capacity;
    uint32_t visited;
    uint32_t visited_gc;
    int32_t gc_generation;
    int32_t gc_creation_frame;
    int32_t slot;
    YYOBJECT_KIND object_kind;
    int32_t rvalue_init_type;
    int32_t current_slot;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(yyobject_base_t) == 0x80); // 8 bits less since no virtual function (8 bits on 64bits)
#endif // _WIN64

struct object_gm_s
{
    const char* name;
    object_gm_t* parent_object;
    HASHMAP_TYPE_PTR(int, object_gm_t)* children_map;
    HASHMAP_TYPE_PTR(int, event_t)* events_map;
    physics_data_gm_t physics_data;
    LINKEDLIST_TYPE(instance_t) instances;
    LINKEDLIST_TYPE(instance_t) instances_recursive;
    uint32_t flags;
    int32_t sprite_index;
    int32_t depth;
    int32_t parent;
    int32_t mask;
    int32_t id;
};

struct yyrect_s
{
    float left;
    float top;
    float right;
    float bottom;
};

struct instance_internal_s
{
    uint32_t instance_flags;
    int32_t id;
    int32_t object_index;
    int32_t sprite_index;
    float sequence_position;
    float last_sequence_position;
    float sequence_direction;
    float image_index;
    float image_speed;
    float image_scale_x;
    float image_scale_y;
    float image_angle;
    float image_alpha;
    uint32_t image_blend;
    float x;
    float y;
    float x_start;
    float y_start;
    float x_previous;
    float y_previous;
    float direction;
    float speed;
    float friction;
    float gravity_direction;
    float gravity;
    float horizontal_speed;
    float vertical_speed;
    yyrect_t bounding_box;
    int timers[12];
    int64_t rollback_frame_killed;
    void* timeline_path;
    code_t* init_code;
    code_t* precreate_code;
    object_gm_t* old_object;
    int32_t layer_id;
    int32_t mask_index;
    int16_t mouse_over_count;
    instance_t* flink;
    instance_t* blink;
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(instance_internal_t) == 0xF8);
#endif // _WIN64

// Islets 1.0.0.3 Steam (x86), GM 2022.6
struct members_only_s
{
    instance_internal_t members;
};
#ifdef _WIN64
    COMPILE_TIME_ASSERT(sizeof(members_only_t) == 0xF8);
#endif // _WIN64

// 2023.x => 2023.8 (and presumably 2023.11)
struct sequence_instance_only_s
{
    void* sequence_instance;
    instance_internal_t members;
};
#ifdef _WIN64
    COMPILE_TIME_ASSERT(sizeof(sequence_instance_only_t) == 0x100);
#endif // _WIN64

// 2022.1 => 2023.1 (may be used later, haven't checked)
struct with_skeleton_mask_s
{
    void* skeleton_mask;
    void* sequence_instance;
    instance_internal_t members;
};
#ifdef _WIN64
    COMPILE_TIME_ASSERT(sizeof(with_skeleton_mask_t) == 0x108);
#endif // _WIN64

struct instance_s
{
    int64_t create_counter;
    object_gm_t* object;
    physics_object_t* physics_object;
    skeleton_instance_t* skeleton_animation;

    // Structs misalign between 2022.1 and 2023.8
    // Easy way to check which to use is to check id and compare
    // it to the result of GetBuiltin("id") on the same instance.
    // Use GetMembers() to get a CInstanceVariables reference.
    union
    {
        members_only_t members_only;
        sequence_instance_only_t sequence_instance_only;
        with_skeleton_mask_t with_skeleton_mask;
    };
};

FUNC_HASH_PTR(int32_t, rvalue_t)
FUNC_HASH_PTR(int, object_gm_t)
FUNC_HASH_PTR(int, event_t)

#endif  /* !GML_STRUCTS_H_ */