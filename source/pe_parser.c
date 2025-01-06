// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/pe_parser.h"
#include "../include/error.h"
#include "Windows.h"
#include "inttypes.h"

#define RVA2VA(TYPE, BASE, RVA) (TYPE)((ULONG_PTR)BASE + RVA)

int pe_load(char* file, char** buffer, size_t* buffer_size)
{
    *buffer_size = -1;
    FILE *fp = fopen(file, "rb");
    int err = MSL_SUCCESS;

    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) 
        {
            /* Get the size of the file. */
            *buffer_size = ftell(fp);
            if (*buffer_size == (uint64_t)(-1)) 
            { 
                err = MSL_INVALID_FILE_SIZE;
                goto clean_up;
            }

            /* Allocate our buffer to that size. */
            *buffer = (char*)malloc(sizeof(char) * (*buffer_size + 1));
            if (*buffer == NULL)
            {
                err = MSL_INSUFFICIENT_MEMORY;
                goto clean_up;
            }

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0)
            {
                err = MSL_UNKNWON_ERROR;
                goto clean_up;
            }

            /* Read the entire file into memory. */
            size_t new_len = fread(*buffer, sizeof(char), *buffer_size, fp);
            if (ferror(fp) != 0) 
            {
                err = MSL_UNREADABLE_FILE;
                goto clean_up;
            }
            else 
            {
                *(*buffer + new_len++) = '\0'; /* Just to be safe. */
            }
        }
    }
    else
    {
        err = MSL_ACCESS_DENIED;
        goto clean_up;
    }
    printf("[>] Loaded file %s with size %016" PRIX64 "\n", file, *buffer_size);

    clean_up:
    fclose(fp);
    return err;
}

int pe_parse(char *buffer)
{
    if (!buffer)
    {
        return MSL_NULL_BUFFER;
    }

    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)buffer;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return MSL_INVALID_DOS_SIGNATURE;
    }

    PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)(buffer + dos_header->e_lfanew);
    if (nt_header->Signature != IMAGE_NT_SIGNATURE)
    {
        return MSL_INVALID_NT_SIGNATURE;
    }

    return MSL_SUCCESS;
}
