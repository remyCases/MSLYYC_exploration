// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef UTILS_MACRO_H_
#define UTILS_MACRO_H_

// usefull macros
#define STR(A)                          #A
#define CAT(A, B)                       A ## B
#define S_CAT(A, B)                     CAT(A, B)
#define CAT_UND(A, B)                   S_CAT(S_CAT(A, _), B)
#define S_CAT_UND(A, B, C)              CAT_UND(CAT_UND(A, B), C)
#define SS_CAT_UND(A, B, C, D)          CAT_UND(S_CAT_UND(A, B, C), D)

#define TO_ENUM(A, ...)                     A,
#define TO_CASE_STR(A, ...)                 case A: return STR(A);
#define TO_CASE_PRINT(A, F, NF, NL, ...)    CAT_UND(TO_CASE_PRINT, __VA_ARGS__)(A, F, NF, NL)
#define TO_CASE_PRINT_SUCCESS(A, F, NF, NL) case A: break;
#define TO_CASE_PRINT_(A, F, NF, NL)        case A: printf("[!] File %s line %d, in function %s: "STR(A)" (code error %d)\n", NF, NL, F, A); break;

# define ARG_LENGTH(...) ARG_LENGTH__(__VA_ARGS__)
# define ARG_LENGTH__(...) ARG_LENGTH_(,##__VA_ARGS__,                         \
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45,\
    44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26,\
    25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6,\
    5, 4, 3, 2, 1, 0)
# define ARG_LENGTH_(_, _63, _62, _61, _60, _59, _58, _57, _56, _55, _54, _53, \
    _52, _51, _50, _49, _48, _47, _46, _45, _44, _43, _42, _41, _40, _39, _38, \
    _37, _36, _35, _34, _33, _32, _31, _30, _29, _28, _27, _26, _25, _24, _23, \
    _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8,   \
    _7, _6, _5, _4, _3, _2, _1, Count, ...) Count

#define COMPILE_TIME_ASSERT(e)          COMPILE_TIME_ASSERT_(e)
#define COMPILE_TIME_ASSERT_(e)         typedef char assertion_##__LINE__[(e) ? 1 : -1]
#define RUNTIME_ASSERT(e)                                           \
    do {                                                            \
        if (!(e)) {                                                 \
            printf("Assertion failed: %s\nFile: %s\nLine: %d\n",    \
                   STR(e), __FILE__, __LINE__);                     \
            abort();                                                \
        }                                                           \
    } while(0)
#endif  /* !UTILS_MACRO_H_ */