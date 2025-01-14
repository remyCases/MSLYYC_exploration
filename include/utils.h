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

#define HASHMAP_ELMT(K, V) SS_CAT_UND(hmel, K, V, t)
#define _HASHMAP_ELMT(K, V)                      \
typedef struct SS_CAT_UND(hmel, K, V, s) {      \
    V value;                                    \
    K key;                                      \
    hash_t hash;                                \
} HASHMAP_ELMT(K, V);

#define HASHMAP(K, V) SS_CAT_UND(hm, K, V, t)
#define _HASHMAP(K, V)                           \
typedef struct SS_CAT_UND(hm, K, V, s) {        \
    int32_t current_size;                       \
    int32_t used_count;                         \
    int32_t current_mask;                       \
    int32_t grow_threshold;                     \
    HASHMAP_ELMT(K, V)* elements;               \
	void(*delete_value)(K* key, V* value);      \
} HASHMAP(K, V);

#define GET_CONTAINER(K, V) S_CAT_UND(get_container, K, V)
#define _GET_CONTAINER(K, V)                                                                        \
int GET_CONTAINER(K, V)(HASHMAP(K, V)* hashmap, K key, HASHMAP_ELMT(K, V)* value)                   \
{                                                                                                   \
    hash_t value_hash = HASH_KEY(K)(key);                                                           \
    int32_t ideal_position = (int)(value_hash & hashmap->current_mask);                             \
    for (HASHMAP_ELMT(K, V) current_element = hashmap->elements[ideal_position];                    \
        current_element.hash != 0;                                                                  \
        current_element = hashmap->elements[(++ideal_position) & hashmap->current_mask]) {          \
        if (current_element.key != key) continue;                                                   \
        *value = current_element;                                                                   \
        return MSL_SUCCESS;                                                                         \
    }                                                                                               \
    return MSL_OBJECT_NOT_IN_LIST;                                                                  \
}

#define GET_VALUE(K, V) S_CAT_UND(get_value, K, V)
#define _GET_VALUE(K, V)                                                                    \
int GET_VALUE(K, V)(HASHMAP(K, V)* hashmap, K key, V* value)                                \
{                                                                                           \
    HASHMAP_ELMT(K, V)* object_container = NULL;                                            \
    if (GET_CONTAINER(K, V)(hashmap, key, object_container) == MSL_OBJECT_NOT_IN_LIST)      \
        return MSL_OBJECT_NOT_IN_LIST;                                                      \
    *value = object_container->value;                                                       \
    return MSL_SUCCESS;                                                                     \
}

#define INSERT(K, V) S_CAT_UND(insert, K, V)
#define _INSERT(K, V)                                                                                           \
int INSERT(K, V)(HASHMAP(K, V)* hashmap, K key, V value)                                                        \
{                                                                                                               \
    /* Check if we need to grow the hashmap */                                                                  \
    if (hashmap->used_count >= hashmap->grow_threshold) {                                                       \
        /* TODO: Implement grow function if needed */                                                           \
        return MSL_INSUFFICIENT_MEMORY;                                                                         \
    }                                                                                                           \
    hash_t value_hash = HASH_KEY(K)(key);                                                                       \
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

#define GROW(K, V) S_CAT_UND(grow, K, V)
#define _GROW(K, V)                                                                             \
int GROW(K, V)(HASHMAP(K, V)* hashmap)                                                          \
{                                                                                               \
    int32_t new_size = hashmap->current_size * 2;                                               \
    HASHMAP_ELMT(K, V)* new_elements = calloc(new_size, sizeof(HASHMAP_ELMT(K, V)));            \
    if (!new_elements) return MSL_INSUFFICIENT_MEMORY;                                          \
    /* Save old values */                                                                       \
    HASHMAP_ELMT(K, V)* old_elements = hashmap->elements;                                       \
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
            INSERT(K, V)(hashmap, old_elements[i].key, old_elements[i].value);                  \
        }                                                                                       \
    }                                                                                           \
    free(old_elements);                                                                         \
    return MSL_SUCCESS;                                                                         \
}

#define DEF_HASHMAP(K, V)   \
    _HASHMAP_ELMT(K, V)     \
    _HASHMAP(K, V)          \

#define DEF_FUNC_HASH(K, V) \
    int GET_CONTAINER(K, V)(HASHMAP(K, V)*, K, HASHMAP_ELMT(K, V)*);  \
    int GET_VALUE(K, V)(HASHMAP(K, V)*, K, V*);                       \
    int INSERT(K, V)(HASHMAP(K, V)*, K, V);                           \
    int GROW(K, V)(HASHMAP(K, V)*);   

#define FUNC_HASH(K, V) \
    _GET_CONTAINER(K, V) \
    _GET_VALUE(K, V)     \
    _INSERT(K, V)        \
    _GROW(K, V)   

#define LINKEDLIST(T) S_CAT_UND(ll, T, t)
#define _LINKEDLIST(T)                  \
typedef struct S_CAT_UND(ll, T, s) {    \
    T* first;                           \
    T* last;                            \
    int32_t count;                      \
    int32_t delete_type;                \
} LINKEDLIST(T);

#define DEF_LINKEDLIST(T)   \
    _LINKEDLIST(T)   

#define OLINKEDLIST(T) S_CAT_UND(oll, T, t)
#define _OLINKEDLIST(T)                 \
typedef struct S_CAT_UND(oll, T, s) {   \
    T* first;                           \
    T* last;                            \
    int32_t count;                      \
} OLINKEDLIST(T);

#define DEF_OLINKEDLIST(T)   \
    _OLINKEDLIST(T)   

#define ARRAY_STRUCTURE(T) S_CAT_UND(as, T, t)
#define _ARRAY_STRUCTURE(T)              \
typedef struct S_CAT_UND(as, T, s) {     \
    int32_t length;                      \
    T* array;                            \
} ARRAY_STRUCTURE(T);

#define DEF_ARRAY_STRUCTURE(T)   \
    _ARRAY_STRUCTURE(T)   

#define VECTOR(T) S_CAT_UND(v, T, t)
#define _VECTOR(T)                      \
typedef struct S_CAT_UND(v, T, s) {     \
    size_t size;                        \
    size_t capacity;                    \
    T* arr;                             \
} VECTOR(T);

#define DEF_VECTOR(T)   \
    _VECTOR(T) 

#define DEFAULT_CAPACITY 2
#define INIT_VECTOR(T) S_CAT_UND(init, vec, T)
#define _INIT_VECTOR(T)                             \
int INIT_VECTOR(T)(VECTOR(T)* vec) {                \
	vec->size = 0;                                  \
	vec->capacity = DEFAULT_CAPACITY;               \
	vec->arr = (T*)malloc(sizeof(T)*vec->capacity); \
	if (!vec->arr) return MSL_ALLOCATION_ERROR;     \
	return MSL_SUCCESS;                             \
} 

#define FREE_VECTOR(T) S_CAT_UND(free, vec, T)
#define _FREE_VECTOR(T)                     \
int FREE_VECTOR(T)(VECTOR(T)* vec) {        \
	free(vec->arr);                         \
	return MSL_SUCCESS;                     \
}

#define RESIZE_VECTOR(T) S_CAT_UND(resize, vec, T)
#define _RESIZE_VECTOR(T)                                       \
int RESIZE_VECTOR(T)(VECTOR(T)* vec) {                          \
	vec->capacity += DEFAULT_CAPACITY;                          \
    vec->arr = (T*)realloc(vec->arr, sizeof(T)*vec->capacity);  \
    if (!vec->arr) return MSL_ALLOCATION_ERROR;                 \
	return MSL_SUCCESS;                                         \
}

#define ADD_VECTOR(T) S_CAT_UND(add, vec, T)
#define _ADD_VECTOR(T)                              \
int ADD_VECTOR(T)(VECTOR(T)* vec, T* elmt) {        \
	int status = MSL_SUCCESS;                       \
	if (vec->size >= vec->capacity) {               \
		status = RESIZE_VECTOR(T)(vec);             \
		if (status) return status;                  \
	}                                               \
	vec->arr[vec->size] = *elmt;                    \
	vec->size++;                                    \
	return status;                                  \
}

#define FUNC_VEC(T)     \
    _INIT_VECTOR(T)     \
    _FREE_VECTOR(T)     \
    _RESIZE_VECTOR(T)   \
    _ADD_VECTOR(T)

#define DEF_FUNC_VEC(T)     \
    int INIT_VECTOR(T)(VECTOR(T)*);     \
    int FREE_VECTOR(T)(VECTOR(T)*);     \
    int RESIZE_VECTOR(T)(VECTOR(T)*);   \
    int ADD_VECTOR(T)(VECTOR(T)*, T*);

#define HASH_KEY(K) CAT_UND(HASH_KEY, K)
#define HASH_KEY_str hash_key_str
#define HASH_KEY_int hash_key_int
#define HASH_KEY_int32_t hash_key_int

hash_t hash_key_int(int key);
hash_t hash_key_ptr(void* key);
hash_t hash_key_str(const char* key);
#endif  /* !UTILS_H_ */