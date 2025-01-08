// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

#include <Zydis/Zydis.h>
#include <inttypes.h>
#include <stdio.h>
#include "../include/decompiler.h"
#include "../include/error.h"

int decompile(char* buffer, size_t buffer_size)
{
    int err = MSL_SUCCESS;
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
    const ZyanUSize length = buffer_size;
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
    while (ZYAN_TRUE)
    {
        ZyanStatus status = ZydisDecoderDecodeFull(&decoder, buffer + offset, length - offset, &instruction, operands);
        if (!ZYAN_SUCCESS(status))
        {
            printf("[>] Error %" PRIX32 " failed to decode %lld\n", status, offset);
            printf("[>] Module %" PRIX32 "\n", ZYAN_STATUS_MODULE(status));
            printf("[>] Code %" PRIX32 "\n", ZYAN_STATUS_CODE(status));
            err = MSL_UNKNWON_ERROR;
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

    return err;
}
