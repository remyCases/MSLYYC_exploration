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

#define ARG_LENGTH(...) ARG_LENGTH__(__VA_ARGS__)
#define ARG_LENGTH__(...) ARG_LENGTH_(,##__VA_ARGS__,                           \
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, \
    44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, \
    25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, \
    5, 4, 3, 2, 1, 0)
#define ARG_LENGTH_(_, _63, _62, _61, _60, _59, _58, _57, _56, _55, _54, _53,   \
    _52, _51, _50, _49, _48, _47, _46, _45, _44, _43, _42, _41, _40, _39, _38,  \
    _37, _36, _35, _34, _33, _32, _31, _30, _29, _28, _27, _26, _25, _24, _23,  \
    _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8,    \
    _7, _6, _5, _4, _3, _2, _1, Count, ...) Count

#define ARG_COUNT(X, ...) ARG_COUNT__(X, __VA_ARGS__)
#define ARG_COUNT__(X, ...) ARG_COUNT_(,##__VA_ARGS__,                          \
    X(63, __VA_ARGS__), X(62, __VA_ARGS__), X(61, __VA_ARGS__), X(60, __VA_ARGS__), X(59, __VA_ARGS__), X(58, __VA_ARGS__), X(57, __VA_ARGS__), X(56, __VA_ARGS__), X(55, __VA_ARGS__), X(54, __VA_ARGS__), X(53, __VA_ARGS__), X(52, __VA_ARGS__), X(51, __VA_ARGS__), X(50, __VA_ARGS__), X(49, __VA_ARGS__), X(48, __VA_ARGS__), X(47, __VA_ARGS__), X(46, __VA_ARGS__), X(45, __VA_ARGS__), \
    X(44, __VA_ARGS__), X(43, __VA_ARGS__), X(42, __VA_ARGS__), X(41, __VA_ARGS__), X(40, __VA_ARGS__), X(39, __VA_ARGS__), X(38, __VA_ARGS__), X(37, __VA_ARGS__), X(36, __VA_ARGS__), X(35, __VA_ARGS__), X(34, __VA_ARGS__), X(33, __VA_ARGS__), X(32, __VA_ARGS__), X(31, __VA_ARGS__), X(30, __VA_ARGS__), X(29, __VA_ARGS__), X(28, __VA_ARGS__), X(27, __VA_ARGS__), X(26, __VA_ARGS__), \
    X(25, __VA_ARGS__), X(24, __VA_ARGS__), X(23, __VA_ARGS__), X(22, __VA_ARGS__), X(21, __VA_ARGS__), X(20, __VA_ARGS__), X(19, __VA_ARGS__), X(18, __VA_ARGS__), X(17, __VA_ARGS__), X(16, __VA_ARGS__), X(15, __VA_ARGS__), X(14, __VA_ARGS__), X(13, __VA_ARGS__), X(12, __VA_ARGS__), X(11, __VA_ARGS__), X(10, __VA_ARGS__), X(9, __VA_ARGS__), X(8, __VA_ARGS__), X(7, __VA_ARGS__), X(6, __VA_ARGS__), \
    X(5, __VA_ARGS__), X(4, __VA_ARGS__), X(3, __VA_ARGS__), X(2, __VA_ARGS__), X(1, __VA_ARGS__), X(0, __VA_ARGS__))
#define ARG_COUNT_(_, _63, _62, _61, _60, _59, _58, _57, _56, _55, _54, _53,    \
    _52, _51, _50, _49, _48, _47, _46, _45, _44, _43, _42, _41, _40, _39, _38,  \
    _37, _36, _35, _34, _33, _32, _31, _30, _29, _28, _27, _26, _25, _24, _23,  \
    _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8,    \
    _7, _6, _5, _4, _3, _2, _1, Count, ...) __VA_ARGS__

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