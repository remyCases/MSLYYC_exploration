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

#define COMPILE_TIME_ASSERT(e)          COMPILE_TIME_ASSERT_(e)
#define COMPILE_TIME_ASSERT_(e)         typedef char assertion_##__LINE__[(e) ? 1 : -1]

#endif  /* !UTILS_MACRO_H_ */