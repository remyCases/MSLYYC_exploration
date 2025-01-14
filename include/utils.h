// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "utils_macro.h"

typedef uint32_t hash_t;

#define HASHMAP_ELMT_TYPE(K, V) SS_CAT_UND(hmel, K, V, t)
#define HASHMAP_ELMT(K, V)                      \
typedef struct SS_CAT_UND(hmel, K, V, s) {      \
    V value;                                    \
    K key;                                      \
    hash_t hash;                                \
} HASHMAP_ELMT_TYPE(K, V);

#define HASHMAP_TYPE(K, V) SS_CAT_UND(hm, K, V, t)
#define HASHMAP(K, V)                           \
typedef struct SS_CAT_UND(hm, K, V, s) {        \
    int32_t current_size;                       \
    int32_t used_count;                         \
    int32_t current_mask;                       \
    int32_t grow_threshold;                     \
    HASHMAP_ELMT_TYPE(K, V)* elements;          \
	void(*delete_value)(K* key, V* value);      \
} HASHMAP_TYPE(K, V);

#define GET_CONTAINER_N(K, V) S_CAT_UND(get_container, K, V)
#define GET_CONTAINER(K, V)                                                                         \
ERROR_MLS GET_CONTAINER_N(K, V)(HASHMAP_TYPE(K, V)* hashmap, K key, HASHMAP_ELMT_TYPE(K, V)* value) \
{                                                                                                   \
    hash_t value_hash = HASH_KEY(K)(key);                                                          \
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

#define INSERT_N(K, V) S_CAT_UND(insert, K, V)
#define INSERT(K, V)                                                                                            \
ERROR_MLS INSERT_N(K, V)(HASHMAP_TYPE(K, V)* hashmap, K key, V value)                                           \
{                                                                                                               \
    /* Check if we need to grow the hashmap */                                                                  \
    if (hashmap->used_count >= hashmap->grow_threshold) {                                                       \
        /* TODO: Implement grow function if needed */                                                           \
        return MSL_INSUFFICIENT_MEMORY;                                                                         \
    }                                                                                                           \
    hash_t value_hash = HASH_KEY(K)(key);                                                                      \
    if (value_hash == 0) value_hash = 1; /* Reserve 0 for empty slots */                                        \
    int32_t ideal_position = (int)(value_hash & hashmap->current_mask);                                         \
    int32_t position = ideal_position;                                                                          \
    /* Find a slot using linear probing */                                                                      \
    while (hashmap->elements[position].hash != 0) {                                                             \
        /* Check if key already exists */                                                                       \
        if (hashmap->elements[position].key == key) {                                                           \
            /* Handle value replacement */                                                                      \
            if (hashmap->delete_value) {                                                                        \
                hashmap->delete_value(&hashmap->elements[position].key, &hashmap->elements[position].value);    \
            }                                                                                                   \
            hashmap->elements[position].value = value;                                                          \
            return MSL_SUCCESS;                                                                                 \
        }                                                                                                       \
        position = (int)((position + 1) & hashmap->current_mask);                                               \
        if (position == ideal_position) {                                                                       \
            return MSL_INSUFFICIENT_MEMORY; /* Map is full */                                                   \
        }                                                                                                       \
    }                                                                                                           \
    /* Insert new element */                                                                                    \
    hashmap->elements[position].hash = value_hash;                                                              \
    hashmap->elements[position].key = key;                                                                      \
    hashmap->elements[position].value = value;                                                                  \
    hashmap->used_count++;                                                                                      \
    return MSL_SUCCESS;                                                                                         \
}

#define GROW_N(K, V) S_CAT_UND(grow, K, V)
#define GROW(K, V)                                                                              \
ERROR_MLS GROW_N(K, V)(HASHMAP_TYPE(K, V)* hashmap)                                             \
{                                                                                               \
    int32_t new_size = hashmap->current_size * 2;                                               \
    HASHMAP_ELMT_TYPE(K, V)* new_elements = calloc(new_size, sizeof(HASHMAP_ELMT_TYPE(K, V)));  \
    if (!new_elements) return MSL_INSUFFICIENT_MEMORY;                                          \
    /* Save old values */                                                                       \
    HASHMAP_ELMT_TYPE(K, V)* old_elements = hashmap->elements;                                  \
    int32_t old_size = hashmap->current_size;                                                   \
    /* Update hashmap with new array */                                                         \
    hashmap->elements = new_elements;                                                           \
    hashmap->current_size = new_size;                                                           \
    hashmap->current_mask = new_size - 1;                                                       \
    hashmap->used_count = 0;                                                                    \
    hashmap->grow_threshold = (new_size * 3) / 4; /* 75% load factor */                         \
    /* Reinsert all elements */                                                                 \
    for (int32_t i = 0; i < old_size; i++) {                                                    \
        if (old_elements[i].hash != 0) {                                                        \
            INSERT_N(K, V)(hashmap, old_elements[i].key, old_elements[i].value);                \
        }                                                                                       \
    }                                                                                           \
    free(old_elements);                                                                         \
    return MSL_SUCCESS;                                                                         \
}

#define HASH(K, V)      \
    HASHMAP_ELMT(K, V)  \
    HASHMAP(K, V)       \

#define FUNC_HASH(K, V) \
    GET_CONTAINER(K, V) \
    GET_VALUE(K, V)     \
    INSERT(K, V)        \
    GROW(K, V)   

#define LINKEDLIST_TYPE(T) S_CAT_UND(ll, T, t)
#define LINKEDLIST(T)                   \
typedef struct S_CAT_UND(ll, T, s) {    \
    T* first;                           \
    T* last;                            \
    int32_t count;                      \
    int32_t delete_type;                \
} LINKEDLIST_TYPE(T);

#define OLINKEDLIST_TYPE(T) S_CAT_UND(oll, T, t)
#define OLINKEDLIST(T)                  \
typedef struct S_CAT_UND(oll, T, s) {   \
    T* first;                           \
    T* last;                            \
    int32_t count;                      \
} OLINKEDLIST_TYPE(T);

#define ARRAY_STRUCTURE_TYPE(T) S_CAT_UND(as, T, t)
#define ARRAY_STRUCTURE(T)               \
typedef struct S_CAT_UND(as, T, s) {     \
    int32_t length;                      \
    T* array;                            \
} ARRAY_STRUCTURE_TYPE(T);

#define HASH_KEY(K) CAT_UND(HASH_KEY, K)
#define HASH_KEY_str hash_key_str
#define HASH_KEY_int hash_key_int
#define HASH_KEY_int32_t hash_key_int

hash_t hash_key_int(int key);
hash_t hash_key_ptr(void* key);
hash_t hash_key_str(const char* key);
#endif  /* !UTILS_H_ */