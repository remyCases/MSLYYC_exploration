// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/Zydis.h"
#include "include/error.h"
#include "include/pe_parser.h"
#include <inttypes.h>

#if defined(_MSC_VER)
#define MSL_COMPILER_MSVC 1
#define MSL_COMPILER_GCC 0
#define MSL_COMPILER_CLANG 0
#elif defined(__GNUC__)
#define MSL_COMPILER_MSVC 0
#define MSL_COMPILER_GCC 1
#define MSL_COMPILER_CLANG 0
#elif defined(__clang__)
#define MSL_COMPILER_MSVC 0
#define MSL_COMPILER_GCC 0
#define MSL_COMPILER_CLANG 1
#else
#error "Unsupported compiler"
#endif

#if MSL_COMPILER_MSVC
#if defined(_M_IX86)
#define MSL_ARCH_X86_32 1
#define MSL_ARCH_X86_64 0
#elif defined(_M_X64)
#define MSL_ARCH_X86_32 0
#define MSL_ARCH_X86_64 1
#else
#error "Unsupported architecture"
#endif
#elif MSL_COMPILER_GCC || MSL_COMPILER_CLANG
#if defined(__i386__)
#define MSL_ARCH_X86_32 1
#define MSL_ARCH_X86_64 0
#elif defined(__x86_64__)
#define MSL_ARCH_X86_32 0
#define MSL_ARCH_X86_64 1
#else
#error "Unsupported architecture"
#endif
#endif

#if defined(_WIN32)
#define MSL_OS_WINDOWS 1
#define MSL_OS_LINUX 0
#elif defined(__linux__)
#define MSL_OS_WINDOWS 0
#define MSL_OS_LINUX 1
#else
#error "Unsupported OS"
#endif

#if MSL_OS_WINDOWS
#if MSL_COMPILER_MSVC
#define MSL_CCALL __cdecl
#define MSL_STDCALL __stdcall
#define MSL_FASTCALL __fastcall
#define MSL_THISCALL __thiscall
#elif MSL_COMPILER_GCC || MSL_COMPILER_CLANG
#define MSL_CCALL __attribute__((cdecl))
#define MSL_STDCALL __attribute__((stdcall))
#define MSL_FASTCALL __attribute__((fastcall))
#define MSL_THISCALL __attribute__((thiscall))
#endif
#else
#define MSL_CCALL
#define MSL_STDCALL
#define MSL_FASTCALL
#define MSL_THISCALL
#endif

#if MSL_COMPILER_MSVC
#define MSL_NOINLINE __declspec(noinline)
#elif MSL_COMPILER_GCC || MSL_COMPILER_CLANG
#define MSL_NOINLINE __attribute__((noinline))
#endif

int main(int argc, char** argv) 
{
    for (int i = 1; i < argc; i++)
	{
		printf("[>] Args[%d] %s\n", i, argv[i]);
    }

    size_t buf_win_size = 0;
    size_t buf_exe_size = 0;
    char* buf_win = NULL;
    char* buf_exe = NULL;

    ERR(pe_load, "data/gog_0.9.1.3.win", &buf_win, &buf_win_size);
    ERR(pe_load, "data/StoneShard.exe", &buf_exe, &buf_exe_size);
    ERR(pe_parse, buf_win);
    ERR(pe_parse, buf_exe);

    // Initialize decoder context
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
    // visualize relative addressing
    ZyanU64 runtime_address = 0x0;
    ZyanUSize offset = 0;
    const ZyanUSize length = buf_exe_size;
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
    while (ZYAN_TRUE)
    {
        ZyanStatus status = ZydisDecoderDecodeFull(&decoder, buf_exe + offset, length - offset, &instruction, operands);
        if (!ZYAN_SUCCESS(status))
        {
            printf("[>] Error %" PRIX32 " failed to decode %lld\n", status, offset);
            printf("[>] Module %" PRIX32 "\n", ZYAN_STATUS_MODULE(status));
            printf("[>] Code %" PRIX32 "\n", ZYAN_STATUS_CODE(status));
            break;
        }

        // Print current instruction pointer.
        printf("%016" PRIX64 "  ", runtime_address);

        // Format & print the binary instruction structure to human-readable format
        char buffer[256];
        ZydisFormatterFormatInstruction(
            &formatter, 
            &instruction, operands,
            instruction.operand_count_visible, 
            buffer, 
            sizeof(buffer), 
            runtime_address, 
            ZYAN_NULL
        );
        puts(buffer);

        offset += instruction.length;
        runtime_address += instruction.length;
    }

	printf("[>] Execution complete\n");
    if (buf_exe) free(buf_exe);
    if (buf_win) free(buf_win);
    return 0;
}