// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdio.h>

// usefull macros
#define STR(A)                      #A
#define CAT(A, B)                   A ## B
#define S_CAT(A, B)                 CAT(A, B)
#define CAT_UND(A, B)               S_CAT(S_CAT(A, _), B)

#define TO_ENUM(A, ...)             A,
#define TO_CASE_STR(A, ...)         case A: return STR(A);
#define TO_CASE_PRINT(A, F, ...)    CAT_UND(TO_CASE_PRINT, __VA_ARGS__)(A, F)
#define TO_CASE_PRINT_SUCCESS(A, F) case A: break;
#define TO_CASE_PRINT_(A, F)        case A: printf("[!] In function %s: "STR(A)" (code error %d)\n", F, A); break;

// error enum
#define MACRO_ERROR(X, ...)                     \
    X(MSL_SUCCESS, __VA_ARGS__, SUCCESS)        \
    X(MSL_ACCESS_DENIED, __VA_ARGS__)           \
    X(MSL_INVALID_FILE_SIZE, __VA_ARGS__)       \
    X(MSL_INSUFFICIENT_MEMORY, __VA_ARGS__)     \
    X(MSL_UNREADABLE_FILE, __VA_ARGS__)         \
    X(MSL_INVALID_DOS_SIGNATURE, __VA_ARGS__)   \
    X(MSL_INVALID_NT_SIGNATURE, __VA_ARGS__)    \
    X(MSL_NULL_BUFFER, __VA_ARGS__)             \
    X(MSL_UNKNWON_ERROR, __VA_ARGS__)           \

#define ERR(F, ...) error_print(STR(F), F(__VA_ARGS__))

typedef enum ERROR_MLS_ {
    MACRO_ERROR(TO_ENUM)
} ERROR_MLS;

char* error_str(ERROR_MLS err);
int error_print(char* func, ERROR_MLS err);
