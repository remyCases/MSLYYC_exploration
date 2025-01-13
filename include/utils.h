// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "utils_macro.h"

typedef uint32_t hash_t;

#define HASHMAP_ELMT_TYPE(K, V) SS_CAT_UND(hmel, K, V, t)
#define HASHMAP_ELMT_TYPE_PTR(K, V) SS_CAT_UND(hmel, K, V, ptr_t)
#define HASHMAP_ELMT(K, V)                      \
typedef struct SS_CAT_UND(hmel, K, V, s) {      \
    V value;                                    \
    K key;                                      \
    hash_t hash;                                \
} HASHMAP_ELMT_TYPE(K, V);

#define HASHMAP_ELMT_PTR(K, V)                  \
typedef struct SS_CAT_UND(hmel, K, V, ptr_s) {  \
    V* value;                                   \
    K key;                                      \
    hash_t hash;                                \
} HASHMAP_ELMT_TYPE_PTR(K, V);

#define HASHMAP_TYPE(K, V) SS_CAT_UND(hm, K, V, t)
#define HASHMAP_TYPE_PTR(K, V) SS_CAT_UND(hm, K, V, ptr_t)
#define HASHMAP(K, V)                           \
typedef struct SS_CAT_UND(hm, K, V, s) {        \
    int32_t current_size;                       \
    int32_t used_count;                         \
    int32_t current_mask;                       \
    int32_t grow_threshold;                     \
    HASHMAP_ELMT_TYPE(K, V)* elements;          \
} HASHMAP_TYPE(K, V);

#define HASHMAP_PTR(K, V)                       \
typedef struct SS_CAT_UND(hm, K, V, ptr_s) {    \
    int32_t current_size;                       \
    int32_t used_count;                         \
    int32_t current_mask;                       \
    int32_t grow_threshold;                     \
    HASHMAP_ELMT_TYPE_PTR(K, V)* elements;      \
} HASHMAP_TYPE_PTR(K, V);

#define GET_CONTAINER_N(K, V) S_CAT_UND(get_container, K, V)
#define GET_CONTAINER(K, V)                                                                         \
ERROR_MLS GET_CONTAINER_N(K, V)(HASHMAP_TYPE(K, V)* hashmap, K key, HASHMAP_ELMT_TYPE(K, V)* value) \
{                                                                                                   \
    hash_t value_hash = hash_key_int(key);                                                          \
    int32_t ideal_position = (int)(value_hash & hashmap->current_mask);                             \
    for (HASHMAP_ELMT_TYPE(K, V) current_element = hashmap->elements[ideal_position];               \
        current_element.hash != 0;                                                                  \
        current_element = hashmap->elements[(++ideal_position) & hashmap->current_mask]) {          \
        if (current_element.key != key) continue;                                                   \
        *value = current_element;                                                                   \
        return MSL_SUCCESS;                                                                         \
    }                                                                                               \
    return MSL_OBJECT_NOT_IN_LIST;                                                                  \
}

#define GET_CONTAINER_PTR(K, V)                                                                             \
ERROR_MLS GET_CONTAINER_N(K, V)(HASHMAP_TYPE_PTR(K, V)* hashmap, K key, HASHMAP_ELMT_TYPE_PTR(K, V)* value) \
{                                                                                                           \
    hash_t value_hash = hash_key_int(key);                                                                  \
    int32_t ideal_position = (int)(value_hash & hashmap->current_mask);                                     \
    for (HASHMAP_ELMT_TYPE_PTR(K, V) current_element = hashmap->elements[ideal_position];                   \
        current_element.hash != 0;                                                                          \
        current_element = hashmap->elements[(++ideal_position) & hashmap->current_mask]) {                  \
        if (current_element.key != key) continue;                                                           \
        *value = current_element;                                                                           \
        return MSL_SUCCESS;                                                                                 \
    }                                                                                                       \
    return MSL_OBJECT_NOT_IN_LIST;                                                                          \
}

#define GET_VALUE_N(K, V) S_CAT_UND(get_value, K, V)
#define GET_VALUE(K, V)                                                                     \
ERROR_MLS GET_VALUE_N(K, V)(HASHMAP_TYPE(K, V)* hashmap, K key, V* value)                   \
{                                                                                           \
    HASHMAP_ELMT_TYPE(K, V)* object_container = NULL;                                       \
    if (GET_CONTAINER_N(K, V)(hashmap, key, object_container) == MSL_OBJECT_NOT_IN_LIST)    \
        return MSL_OBJECT_NOT_IN_LIST;                                                      \
    *value = object_container->value;                                                       \
    return MSL_SUCCESS;                                                                     \
}

#define GET_VALUE_PTR(K, V)                                                                 \
ERROR_MLS GET_VALUE_N(K, V)(HASHMAP_TYPE_PTR(K, V)* hashmap, K key, V** value)              \
{                                                                                           \
    HASHMAP_ELMT_TYPE_PTR(K, V)* object_container = NULL;                                   \
    if (GET_CONTAINER_N(K, V)(hashmap, key, object_container) == MSL_OBJECT_NOT_IN_LIST)    \
        return MSL_OBJECT_NOT_IN_LIST;                                                      \
    *value = object_container->value;                                                       \
    return MSL_SUCCESS;                                                                     \
}

#define HASH(K, V)      \
    HASHMAP_ELMT(K, V)  \
    HASHMAP(K, V)       \

#define FUNC_HASH(K, V) \
    GET_CONTAINER(K, V) \
    GET_VALUE(K, V)

#define HASH_PTR(K, V)      \
    HASHMAP_ELMT_PTR(K, V)  \
    HASHMAP_PTR(K, V)       

#define FUNC_HASH_PTR(K, V) \
    GET_CONTAINER_PTR(K, V) \
    GET_VALUE_PTR(K, V)

#define LINKEDLIST_TYPE(T) S_CAT_UND(ll, T, t)
#define LINKEDLIST(T)                   \
typedef struct S_CAT_UND(ll, T, s) {    \
    T* first;                           \
    T* last;                            \
    int32_t count;                      \
    int32_t delete_type;                \
} LINKEDLIST_TYPE(T);

hash_t hash_key_int(int key);
hash_t hash_key_ptr(void* key);
hash_t hash_key_char(const char* key);
#endif  /* !UTILS_H_ */