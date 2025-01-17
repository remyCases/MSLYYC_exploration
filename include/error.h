// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include "utils_macro.h"

// error enum
#define MACRO_ERROR(X, ...)                     \
    X(MSL_SUCCESS, __VA_ARGS__, SUCCESS)        \
    X(MSL_INVALID_PARAMETER, __VA_ARGS__)       \
    X(MSL_ACCESS_DENIED, __VA_ARGS__)           \
    X(MSL_FILE_NOT_FOUND, __VA_ARGS__)          \
    X(MSL_INVALID_FILE_ATTRIBUTE, __VA_ARGS__)  \
    X(MSL_INVALID_FILE_SIZE, __VA_ARGS__)       \
    X(MSL_INSUFFICIENT_MEMORY, __VA_ARGS__)     \
    X(MSL_UNREADABLE_FILE, __VA_ARGS__)         \
    X(MSL_INVALID_SIGNATURE, __VA_ARGS__)       \
    X(MSL_INVALID_DOS_SIGNATURE, __VA_ARGS__)   \
    X(MSL_INVALID_NT_SIGNATURE, __VA_ARGS__)    \
    X(MSL_INVALID_ARCH, __VA_ARGS__)            \
    X(MSL_INVALID_HANDLE_VALUE, __VA_ARGS__)    \
    X(MSL_FILE_PART_NOT_FOUND, __VA_ARGS__)     \
    X(MSL_NULL_BUFFER, __VA_ARGS__)             \
    X(MSL_ALLOCATION_ERROR, __VA_ARGS__)        \
    X(MSL_OBJECT_ALREADY_EXISTS, __VA_ARGS__)   \
    X(MSL_OBJECT_NOT_FOUND, __VA_ARGS__)        \
    X(MSL_OBJECT_NOT_IN_LIST, __VA_ARGS__)      \
    X(MSL_MODULE_INTERNAL_ERROR, __VA_ARGS__)   \
    X(MSL_MODULE_DEPENDENCY_NOT_RESOLVED, __VA_ARGS__)   \
    X(MSL_EXTERNAL_ERROR, __VA_ARGS__)          \
    X(MSL_POINTER_NON_NULL, __VA_ARGS__)        \
    X(MSL_FAIL, __VA_ARGS__)                    \
    X(MSL_UNKNWON_ERROR, __VA_ARGS__)           \

#define LOG_ON_ERR(F, ...) error_print(STR(F), __FILE__, __LINE__, F(__VA_ARGS__))
#define CHECK_CALL(F, ...)                            \
    last_status = LOG_ON_ERR(F, __VA_ARGS__);   \
    if (last_status) return last_status;

#define CHECK_CALL_CUSTOM_ERROR(F, E, ...)            \
    last_status = LOG_ON_ERR(F, __VA_ARGS__);   \
    if (last_status) return E;

#define CHECK_CALL_GOTO_ERROR(F, E, ...)            \
    last_status = LOG_ON_ERR(F, __VA_ARGS__);   \
    if (last_status) goto E;

typedef enum ERROR_MLS_ {
    MACRO_ERROR(TO_ENUM)
} ERROR_MLS;

char* error_str(int status);
int error_print(char* function_name, char* nfile, int nline, int status);

#endif  /* !ERROR_H_ */