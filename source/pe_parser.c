// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "Windows.h"
#include <stdint.h>
#include "inttypes.h"
#include "../include/pe_parser.h"
#include "../include/error.h"

#define RVA2VA(TYPE, BASE, RVA) (TYPE)((ULONG_PTR)BASE + RVA)

int pe_load(char* file, char** buffer, size_t* buffer_size)
{
    *buffer_size = -1;
    FILE *fp = fopen(file, "rb");
    int status = MSL_SUCCESS;

    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) 
        {
            /* Get the size of the file. */
            *buffer_size = ftell(fp);
            if (*buffer_size == (uint64_t)(-1)) 
            { 
                status = MSL_INVALID_FILE_SIZE;
                goto clean_up;
            }

            /* Allocate our buffer to that size. */
            *buffer = (char*)malloc(sizeof(char) * (*buffer_size + 1));
            if (*buffer == NULL)
            {
                status = MSL_INSUFFICIENT_MEMORY;
                goto clean_up;
            }

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0)
            {
                status = MSL_UNKNWON_ERROR;
                goto clean_up;
            }

            /* Read the entire file into memory. */
            size_t new_len = fread(*buffer, sizeof(char), *buffer_size, fp);
            if (ferror(fp) != 0) 
            {
                status = MSL_UNREADABLE_FILE;
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
        status = MSL_ACCESS_DENIED;
        goto clean_up;
    }
    printf("[>] Loaded file %s with size %016" PRIX64 "\n", file, *buffer_size);

    clean_up:
    fclose(fp);
    return status;
}

int ppi_get_nt_header(void* image, void** header)
{
    // Check the DOS header to be sure we mapped an actual PE file
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)(image);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return MSL_INVALID_DOS_SIGNATURE;
    }

    // Get the NT headers
    PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)((char*)image + dos_header->e_lfanew);

    // The signature field is the same offset and size for both x86 and x64.
    if (nt_header->Signature != IMAGE_NT_SIGNATURE)
    {
        return MSL_INVALID_NT_SIGNATURE;
    }

    *header = nt_header;
    return MSL_SUCCESS;
}

// https://github.com/Archie-osu/HattieDrv/blob/main/Driver/source/ioctl/pe/pe.cpp
// See HTGetSectionBoundsByName
int ppi_get_module_section_bounds(void* image, const char* section_name, uint64_t* section_offset, size_t* section_size)
{
    int last_status = MSL_SUCCESS;

    PIMAGE_NT_HEADERS nt_header = NULL;
    // NT Header query failed, not a valid image?
    CALL(ppi_get_nt_header, image, (void*)(nt_header));

    PIMAGE_SECTION_HEADER first_section = (PIMAGE_SECTION_HEADER)(nt_header + 1);

    for (
        PIMAGE_SECTION_HEADER current_section = first_section;
        current_section < (first_section + nt_header->FileHeader.NumberOfSections);
        current_section++
        )
    {
        // current_section->Name is not null terminated, we null terminate it.
        // This might not be an issue in normal PE files
        // but while parsing ntoskrnl.exe, this issue became apparent.
        char name_buffer[16] = { 0 };
        memcpy(name_buffer, current_section->Name, 8);

        // Actually we didn't need to null-terminate the string here, since we only compare up to 8 chars anyway
        if (!strnicmp(name_buffer, section_name, 8))
        {
            *section_offset = current_section->VirtualAddress;
            *section_size = current_section->Misc.VirtualSize;

            return MSL_SUCCESS;
        }
    }

    return MSL_FILE_PART_NOT_FOUND;
}