// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "Windows.h"
#include <stdint.h>
#include "inttypes.h"
#include "../include/pe_parser.h"
#include "../include/error.h"

module_t* global_initial_image;

int pp_query_image_architecture(const char* path, unsigned short* image_architecture)
{
    int last_status = MSL_SUCCESS;

    void* image_base = NULL;
    size_t image_size = 0;
    unsigned short image_arch = 0;

    // Allocate the file into memory
    CHECK_CALL_GOTO_ERROR(ppi_map_file_to_memory_alloc, ret, path, &image_base, &image_size);
    
    // Query the image architecture
    CHECK_CALL_GOTO_ERROR(ppi_query_image_architecture, ret, image_base, &image_arch);

    // Save the image architecture we got
    *image_architecture = image_arch;

    // Free the file, we don't care if we errored on PpiQueryImageArchitecture
    ret:
    if (image_base) free(image_base);
    return last_status;
}

int pp_find_file_export_by_name(const char* image_path, const char* image_export_name, uintptr_t* offset)
{
    int last_status = MSL_SUCCESS;
    *offset = 0;
    void* image_base = NULL;
    size_t image_size = 0;

    // Map the file into memory, remember to free!
    CHECK_CALL_GOTO_ERROR(ppi_map_file_to_memory_alloc, ret, image_path, image_base, &image_size);
    CHECK_CALL_GOTO_ERROR(ppi_get_export_offset, ret, image_base,image_export_name, offset);

    ret:
    if (image_base) free(image_base);
    return last_status;
}

int pp_get_framework_routine(const char* export_name, void** routine)
{
    int last_status = MSL_SUCCESS;
    FARPROC framework_routine = GetProcAddress(global_initial_image->image_base.hmodule, export_name);

    *routine = (void*)(framework_routine);
    return last_status;
}

int pp_get_image_subsystem(void* image, unsigned short* image_subsystem)
{
    int last_status = MSL_SUCCESS;
    void* nt_header = NULL;

    CHECK_CALL(ppi_get_nt_header, image, &nt_header);
    *image_subsystem = ((PIMAGE_NT_HEADERS)nt_header)->OptionalHeader.Subsystem;
    return last_status;
}

int pp_get_current_architecture(unsigned short* image_architecture)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ppi_query_image_architecture, global_initial_image->image_base.pointer, image_architecture);
    return last_status;
}

int ppi_map_file_to_memory_alloc(const char* file_path, void** base_of_file, size_t* size_of_file)
{
    int last_status = MSL_SUCCESS;
    // Try to open the file
    FILE *fp = fopen(file_path, "rb");
    size_t file_size = 0;
    char* file_in_memory = NULL;

    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) 
        {
            /* Get the size of the file. */
            file_size = ftell(fp);
            if (file_size == (uint64_t)(-1)) 
            { 
                last_status = MSL_INVALID_FILE_SIZE;
                goto clean_up;
            }

            /* Allocate our buffer to that size. */
            file_in_memory = (char*)malloc(sizeof(char) * (file_size + 1));
            if (file_in_memory == NULL)
            {
                last_status = MSL_ALLOCATION_ERROR;
                goto clean_up;
            }

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0)
            {
                last_status = MSL_UNKNWON_ERROR;
                goto clean_up;
            }

            /* Read the entire file into memory. */
            size_t new_len = fread(file_in_memory, sizeof(char), file_size, fp);
            if (ferror(fp) != 0) 
            {
                last_status = MSL_UNREADABLE_FILE;
                goto clean_up;
            }
            else 
            {
                file_in_memory[new_len++] = '\0'; /* Just to be safe. */
            }
        }
    }
    else
    {
        last_status = MSL_ACCESS_DENIED;
        goto clean_up;
    }
    printf("[>] Loaded file %s with size %016" PRIX64 "\n", file_path, file_size);

    *size_of_file = file_size;
    *base_of_file = file_in_memory;

    clean_up:
    fclose(fp);
    return last_status;
}

int ppi_query_image_architecture(void* image, unsigned short* image_architecture)
{
    int last_status = MSL_SUCCESS;
    void* nt_header = NULL;

    CHECK_CALL(ppi_get_nt_header, image, nt_header);
    *image_architecture = ((PIMAGE_NT_HEADERS)nt_header)->FileHeader.Machine;

    return last_status;
}

int ppi_find_module_export_by_name(const module_t* image, const char* image_export_name, void** export)
{
    int last_status = MSL_SUCCESS;
    *export = NULL;
    uintptr_t export_offset = 0;
    CHECK_CALL(ppi_get_export_offset, image->image_base.pointer, image_export_name, &export_offset);

    *export = (void*)(image->image_base.address + export_offset);
    return last_status;
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
    CHECK_CALL(ppi_get_nt_header, image, (void*)(nt_header));

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

int ppi_get_export_offset(void* image, const char* image_export_name, uintptr_t* export_offset)
{
    int last_status = MSL_SUCCESS;
    unsigned short target_image_arch = 0;

    CHECK_CALL(ppi_query_image_architecture, image, &target_image_arch);

    void* nt_headers = NULL;
    CHECK_CALL(ppi_get_nt_header, image, &nt_headers);

    PIMAGE_EXPORT_DIRECTORY export_directory = NULL;
    uint32_t offset = 0;

    if (target_image_arch == IMAGE_FILE_MACHINE_I386)
    {
        // This NT_HEADERS object has the correct bitness, it is now safe to access the optional header
        PIMAGE_NT_HEADERS32 nt_headers_x86 = (PIMAGE_NT_HEADERS32)(nt_headers);

        // Handle object files
        if (!nt_headers_x86->FileHeader.SizeOfOptionalHeader)
        {
            return MSL_FILE_PART_NOT_FOUND;
        }

        // In case our file doesn't have an export header
        if (nt_headers_x86->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT)
        {
            return MSL_FILE_PART_NOT_FOUND;
        }

        // Get the RVA - we can't just add this to file_in_memory because of section alignment and stuff...
        // We could if we had the file already LLA'd into memory, but that's impossible, since the file's a different arch.
        DWORD export_dir_address = nt_headers_x86->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (!export_dir_address)
        {
            return MSL_FILE_PART_NOT_FOUND;
        }

        // Get the export directory from the VA 
        CHECK_CALL(ppi_rva_to_file_offset32, nt_headers_x86, export_dir_address, &offset);
        export_directory = (PIMAGE_EXPORT_DIRECTORY)((char*)(image) + offset);
    }
    else if (target_image_arch == IMAGE_FILE_MACHINE_AMD64)
    {
        PIMAGE_NT_HEADERS64 nt_headers_x64 = (PIMAGE_NT_HEADERS64)(nt_headers);

        // Handle object files
        if (!nt_headers_x64->FileHeader.SizeOfOptionalHeader)
        {
            return MSL_FILE_PART_NOT_FOUND;
        }

        // In case our file doesn't have an export header
        if (nt_headers_x64->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT)
        {
            return MSL_FILE_PART_NOT_FOUND;
        }

        // Get the RVA - we can't just add this to file_in_memory because of section alignment and stuff...
        // We could if we had the file already LLA'd into memory, but that's impossible, since  the file's a different arch.
        DWORD export_dir_address = nt_headers_x64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (!export_dir_address)
        {
            return MSL_FILE_PART_NOT_FOUND;
        }

        // Get the export directory from the VA 
        CHECK_CALL(ppi_rva_to_file_offset64, nt_headers_x64, export_dir_address, &offset);
        export_directory = (PIMAGE_EXPORT_DIRECTORY)((char*)(image) + offset);
    }
    else
    {
        // Unsupported architecture
        return MSL_INVALID_ARCH;
    }

    // The file doesn't have an export directory (aka. doesn't export anything)
    if (!export_directory)
    {
        return MSL_FILE_PART_NOT_FOUND;
    }

    uint32_t function_names_offset = 0;
    CHECK_CALL(ppi_rva_to_file_offset, (PIMAGE_NT_HEADERS)(nt_headers), export_directory->AddressOfNames, &function_names_offset);
    // Get all our required arrays
    DWORD* function_names = (DWORD*)((char*)(image) + function_names_offset);

    uint32_t function_name_ordinals_offset = 0;
    CHECK_CALL(ppi_rva_to_file_offset, (PIMAGE_NT_HEADERS)(nt_headers), export_directory->AddressOfNameOrdinals, &function_name_ordinals_offset);
    WORD* function_name_ordinals = (WORD*)((char*)(image) + function_name_ordinals_offset);

    uint32_t function_addresses_offset = 0;
    CHECK_CALL(ppi_rva_to_file_offset, (PIMAGE_NT_HEADERS)(nt_headers), export_directory->AddressOfFunctions, &function_addresses_offset);
    DWORD* function_addresses = (DWORD*)((char*)(image) + function_addresses_offset);

    uint32_t export_name_offset = 0;
    // Loop over all the named exports
    for (DWORD n = 0; n < export_directory->NumberOfNames; n++)
    {
        CHECK_CALL(ppi_rva_to_file_offset, (PIMAGE_NT_HEADERS)(nt_headers), function_names[n], &export_name_offset);
        // Get the name of the export
        const char* export_name = (char*)(image) + export_name_offset;

        // Get the function ordinal for array access
        short function_ordinal = function_name_ordinals[n];

        // Get the function offset
        uint32_t function_offset = function_addresses[function_ordinal];

        // If it's our target export
        if (!stricmp(image_export_name, export_name))
        {
            *export_offset = function_offset;
            return MSL_SUCCESS;
        }
    }

    return MSL_OBJECT_NOT_FOUND;
}

int ppi_rva_to_file_offset(PIMAGE_NT_HEADERS image_headers, uint32_t rva, uint32_t* offset)
{
    int last_status = MSL_SUCCESS;
    PIMAGE_SECTION_HEADER pHeaders = IMAGE_FIRST_SECTION(image_headers);

    // Loop over all the sections of the file
    for (short n = 0; n < image_headers->FileHeader.NumberOfSections; n++)
    {
        // ... to check if the RVA points to within that section
        // the section begins at pHeaders[n].VirtualAddress and ends at pHeaders[n].VirtualAddress + pHeaders[n].SizeOfRawData
        if (rva >= pHeaders[n].VirtualAddress && rva < (pHeaders[n].VirtualAddress + pHeaders[n].SizeOfRawData))
        {
            // The RVA points into this section, so return the offset inside the section's data.
            *offset = (rva - pHeaders[n].VirtualAddress) + pHeaders[n].PointerToRawData;
            return last_status;
        }
    }

    *offset = 0;
    return last_status;
}

int ppi_rva_to_file_offset64(PIMAGE_NT_HEADERS64 image_headers, uint32_t rva, uint32_t* offset)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ppi_rva_to_file_offset, (PIMAGE_NT_HEADERS)(image_headers), rva, offset);
    return last_status;
}

int ppi_rva_to_file_offset32(PIMAGE_NT_HEADERS32 image_headers, uint32_t rva, uint32_t* offset)
{
    int last_status = MSL_SUCCESS;
    CHECK_CALL(ppi_rva_to_file_offset, (PIMAGE_NT_HEADERS)(image_headers), rva, offset);
    return last_status;
}