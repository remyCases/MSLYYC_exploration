// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdio.h>

// usefull macros
#define STR(A)                          #A
#define CAT(A, B)                       A ## B
#define S_CAT(A, B)                     CAT(A, B)
#define CAT_UND(A, B)                   S_CAT(S_CAT(A, _), B)

#define TO_ENUM(A, ...)                 A,
#define TO_CASE_STR(A, ...)             case A: return STR(A);
#define TO_CASE_PRINT(A, F, L, ...)     CAT_UND(TO_CASE_PRINT, __VA_ARGS__)(A, F, L)
#define TO_CASE_PRINT_SUCCESS(A, F, L)  case A: break;
#define TO_CASE_PRINT_(A, F, L)         case A: printf("[!] Line %d, in function %s: "STR(A)" (code error %d)\n", L, F, A); break;

// error enum
#define MACRO_ERROR(X, ...)                     \
    X(MSL_SUCCESS, __VA_ARGS__, SUCCESS)        \
    X(MSL_ACCESS_DENIED, __VA_ARGS__)           \
    X(MSL_INVALID_FILE_SIZE, __VA_ARGS__)       \
    X(MSL_INSUFFICIENT_MEMORY, __VA_ARGS__)     \
    X(MSL_UNREADABLE_FILE, __VA_ARGS__)         \
    X(MSL_INVALID_DOS_SIGNATURE, __VA_ARGS__)   \
    X(MSL_INVALID_NT_SIGNATURE, __VA_ARGS__)    \
    X(MSL_INVALID_FILE_TYPE, __VA_ARGS__)       \
    X(MSL_NULL_BUFFER, __VA_ARGS__)             \
    X(MSL_ALLOCATION_ERROR, __VA_ARGS__)        \
    X(MSL_OBJECT_ALREADY_EXISTS, __VA_ARGS__)   \
    X(MSL_OBJECT_NOT_IN_LIST, __VA_ARGS__)      \
    X(MSL_UNKNWON_ERROR, __VA_ARGS__)           \

#define LOG_ON_ERR(F, ...) error_print(STR(F), __LINE__, F(__VA_ARGS__))

typedef enum ERROR_MLS_ {
    MACRO_ERROR(TO_ENUM)
} ERROR_MLS;

char* error_str(int status);
int error_print(char* function_name, int nline, int status);