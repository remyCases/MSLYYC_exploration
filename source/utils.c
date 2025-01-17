// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/utils.h"
#include "../include/error.h"
#include "../include/gml_structs.h"

int iterator_create_alloc(const char* path, const char* pattern, directory_iterator_t** directory_iterator) 
{
    int last_status = MSL_SUCCESS;
    directory_iterator_t* iter = NULL;
    if (*directory_iterator) 
    {
        last_status = MSL_POINTER_NON_NULL;
        goto cleanup;
    }

    iter = (directory_iterator_t*)malloc(sizeof(directory_iterator_t));
    if (!iter) 
    {
        last_status = MSL_ALLOCATION_ERROR;
        goto cleanup;
    }

    size_t path_len = strlen(path);
    iter->current_path = (char*)malloc(MAX_PATH);
    if (!iter->current_path) 
    {
        last_status = MSL_ALLOCATION_ERROR;
        goto cleanup;
    }

    strcpy(iter->current_path, path);
    if (path[path_len - 1] != '\\') 
    {
        strcat(iter->current_path, "\\");
    }

    iter->pattern = strdup(pattern ? pattern : "*");
    iter->stack = NULL;

    // Create the initial search pattern
    char search_path[MAX_PATH];
    strcpy(search_path, iter->current_path);
    search_path[MAX_PATH-1] = 0;
    strcat(search_path, iter->pattern);
    search_path[MAX_PATH-1] = 0;
    
    iter->find_handle = FindFirstFile(search_path, &iter->find_data);
    if (iter->find_handle == INVALID_HANDLE_VALUE)
    {
        
        last_status = MSL_EXTERNAL_ERROR;
        goto cleanup;
    }

    ret:
    *directory_iterator = iter;
    return last_status;

    cleanup:
    if(iter && iter->pattern) free(iter->pattern);
    if(iter && iter->current_path) free(iter->current_path);
    if(iter) free(iter);
    goto ret;
}

int iterator_next(directory_iterator_t* iter) 
{
    while (1) 
    {
        // Try to get next file in current directory
        if (FindNextFile(iter->find_handle, &iter->find_data)) 
        {
            return 1;
        }

        // If no more files in current directory
        FindClose(iter->find_handle);

        // If we have directories in stack, pop and continue
        if (iter->stack) 
        {
            directory_stack_node_t* top = iter->stack;
            iter->stack = top->next;
            
            strcpy(iter->current_path, top->path);
            iter->find_handle = top->find_handle;
            
            free(top->path);
            free(top);
            continue;
        }

        return 0;  // No more files
    }
}

int iterator_enter_directory_alloc(directory_iterator_t* iter) 
{
    if (!(iter->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
        strcmp(iter->find_data.cFileName, ".") == 0 ||
        strcmp(iter->find_data.cFileName, "..") == 0) {
        return 0;
    }

    // Push current state to stack
    directory_stack_node_t* node = (directory_stack_node_t*)malloc(sizeof(directory_stack_node_t));
    if (!node) return 0;

    node->path = strdup(iter->current_path);
    node->find_handle = iter->find_handle;
    node->next = iter->stack;
    iter->stack = node;

    // Update current path
    char newPath[MAX_PATH];
    strcpy(newPath, iter->current_path);
    newPath[MAX_PATH-1] = 0;
    strcat(newPath, iter->find_data.cFileName);
    newPath[MAX_PATH-1] = 0;
    strcat(newPath, "\\");
    newPath[MAX_PATH-1] = 0;

    strcpy(iter->current_path, newPath);

    // Start new directory search
    char search_path[MAX_PATH];
    strcpy(search_path, iter->current_path);
    search_path[MAX_PATH-1] = 0;
    strcat(search_path, iter->pattern);
    search_path[MAX_PATH-1] = 0;
    
    iter->find_handle = FindFirstFile(search_path, &iter->find_data);
    if (iter->find_handle == INVALID_HANDLE_VALUE) 
    {
        return iterator_next(iter);  // Skip empty directories
    }

    return 1;
}

int iterator_destroy(directory_iterator_t* iter) 
{
    int last_status = MSL_SUCCESS;
    if (!iter) return MSL_POINTER_NON_NULL;

    // Close current find handle
    if (iter->find_handle != INVALID_HANDLE_VALUE) {
        FindClose(iter->find_handle);
    }

    directory_stack_node_t* top = NULL;
    // Clean up stack
    while (iter->stack) 
    {
        top = iter->stack;
        iter->stack = top->next;
        FindClose(top->find_handle);
        free(top->path);
        free(top);
    }

    free(iter->current_path);
    free(iter->pattern);
    free(iter);
    return last_status;
}

// Returns 1 if the path has a parent path component
int has_parent_path(const char* path, bool* parent_path) 
{
    if (!path || !*path) // Empty path
    {
        *parent_path = false;
        goto ret;
    }

    size_t len = strlen(path);
    if (len == 0)
    {
        *parent_path = false;
        goto ret;
    }

    // Handle root paths
    if (len == 2 && path[1] == ':') // Drive letter only (e.g., "C:")
    {
        *parent_path = false;
        goto ret;
    }
    if (len == 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) // Root directory (e.g., "C:\")
    {
        *parent_path = false;
        goto ret;
    }

    // Remove trailing slashes
    while (len > 0 && (path[len - 1] == '\\' || path[len - 1] == '/')) 
    {
        len--;
    }
    if (len == 0)
    {
        *parent_path = false;
        goto ret;
    }

    // Look for last separator
    for (size_t i = len - 1; i > 0; i--) 
    {
        if (path[i] == '\\' || path[i] == '/') 
        {
            *parent_path = false;
            goto ret;
        }
    }

    // Check if we have a drive letter prefix
    if (len > 1 && path[1] == ':') 
    {
        *parent_path = len > 2;  // Has something after drive letter
        goto ret;
    }

    *parent_path = false;
    ret:
    return MSL_SUCCESS;
}

// Caller must free the returned string.
int parent_path_alloc(const char* path, char** parent)
{
    int last_status = MSL_SUCCESS;
    if (!path || !*path) 
    {
        last_status = MSL_INVALID_PARAMETER;
        goto cleanup;
    }
    bool parent;
    CALL(has_parent_path, path, &parent);

    if (!parent) 
    {
        last_status = MSL_INVALID_PARAMETER;
        goto cleanup;
    }

    size_t len = strlen(path);
    char* result = (char*)malloc(len + 1);
    if (!result) return MSL_ALLOCATION_ERROR;
    
    strcpy(result, path);

    // Remove trailing slashes
    while (len > 0 && (result[len - 1] == '\\' || result[len - 1] == '/')) 
    {
        result[--len] = '\0';
    }
    if (len == 0) 
    {
        last_status = MSL_INVALID_PARAMETER;
        goto cleanup;
    }

    // Find last separator
    size_t last_sep = 0;
    for (size_t i = len - 1; i > 0; i--) 
    {
        if (result[i] == '\\' || result[i] == '/') 
        {
            last_sep = i;
            break;
        }
    }

    // Handle special cases
    if (last_sep == 0) 
    {
        // Check if we have a drive letter
        if (len > 1 && result[1] == ':') 
        {
            if (len > 2) 
            {
                result[2] = '\0';  // Keep drive letter and colon
                *parent = result;
                goto ret;
            }
            last_status = MSL_INVALID_PARAMETER;
            goto cleanup;
        }
        last_status = MSL_INVALID_PARAMETER;
        goto cleanup;
    }

    // Handle root directory of a drive
    if (last_sep == 2 && result[1] == ':') 
    {
        result[3] = '\0';  // Keep "C:\"
        *parent = result;
        goto ret;
    }

    // Normal case - truncate at last separator
    result[last_sep] = '\0';
    *parent = result;

    ret:
    return last_status;

    cleanup:
    if (result) free(result);
    goto ret;
}

int is_regular_file(const char* path, bool* regular_file) 
{
    if (!path || !*path)
    {
        *regular_file = false;
        return MSL_SUCCESS;
    }
    
    unsigned long attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        *regular_file = false;
        return MSL_INVALID_FILE_ATTRIBUTE;
    }
    
    *regular_file = !(attrs & FILE_ATTRIBUTE_DIRECTORY) && 
                    !(attrs & FILE_ATTRIBUTE_DEVICE) &&
                    !(attrs & FILE_ATTRIBUTE_REPARSE_POINT);
    
    return MSL_SUCCESS;
}

int has_filename(const char* path, bool* filename) 
{
    if (!path || !*path)
    {
        *filename = false;
        goto ret;
    }
    
    size_t len = strlen(path);
    if (len == 0)
    {
        *filename = false;
        goto ret;
    }
    
    // Skip trailing slashes
    while (len > 0 && (path[len-1] == '\\' || path[len-1] == '/')) 
    {
        len--;
    }
    if (len == 0)
    {
        *filename = false;
        goto ret;
    }
    
    // Check for special cases
    if (len == 2 && path[1] == ':')  // Drive letter only
    {
        *filename = false;
        goto ret;
    }
    if (len == 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) // Root dir
    {
        *filename = false;
        goto ret;
    }  
    
    // Look for last separator
    for (size_t i = len-1; i > 0; i--) 
    {
        if (path[i] == '\\' || path[i] == '/') 
        {
            *filename = path[i+1] != '\0';  // Has something after separator
            goto ret;
        }
    }
    

    *filename = true;  // No separators found, entire path is filename
    ret:
    return MSL_SUCCESS;
}

// Caller must free the returned string.
int filename_alloc(const char* path, char** filename) 
{
    int last_status = MSL_SUCCESS;
    bool flag;
    CALL(has_filename, path, &flag);
    if (!path || !flag) 
    {
        last_status = MSL_SUCCESS;
        *filename = NULL;
        goto ret;
    }
    
    size_t len = strlen(path);
    // Skip trailing slashes
    while (len > 0 && (path[len-1] == '\\' || path[len-1] == '/')) 
    {
        len--;
    }
    
    // Find last separator
    const char* last_sep = NULL;
    for (size_t i = 0; i < len; i++) 
    {
        if (path[i] == '\\' || path[i] == '/') 
        {
            last_sep = &path[i];
        }
    }
    
    // Extract filename
    const char* fname_start = last_sep ? last_sep + 1 : path;
    size_t fname_len = &path[len] - fname_start;
    
    char* result = (char*)malloc(fname_len + 1);
    if (!result) 
    {
        last_status = MSL_ALLOCATION_ERROR;
        *filename = NULL;
        goto ret;
    }
    
    memcpy(result, fname_start, fname_len);
    result[fname_len] = '\0';
    *filename = result;

    ret:
    return last_status;
}

int has_extension(const char* path, bool* extension) 
{
    int last_status = MSL_SUCCESS;
    bool flag;
    CALL(has_filename, path, &flag);
    if (!path || !flag)
    {
        *extension = false;
        goto ret;
    } 
    
    char* fname;
    CALL(filename, path, fname);
    if (!fname)
    {
        *extension = false;
        goto ret;
    }
    
    size_t len = strlen(fname);
    // Look for last dot after last separator
    for (size_t i = 0; i < len; i++) 
    {
        if (fname[i] == '.') 
        {
            *extension = (i < len - 1);  // Must have something after dot
            break;
        }
    }
    
    free(fname);
    ret:
    return last_status;
}

static int get_file_info(const char* path, BY_HANDLE_FILE_INFORMATION* result) 
{
    // Get file attributes first
    unsigned long attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) 
    {
        return MSL_INVALID_FILE_ATTRIBUTE;
    }

    // Open file/directory handle with minimal permissions needed
    unsigned long access = 0; // We don't need any access rights for this
    unsigned long share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    unsigned long flags = FILE_FLAG_BACKUP_SEMANTICS; // Needed for directories
    
    HANDLE handle = CreateFileA(path, access, share, NULL, OPEN_EXISTING, flags, NULL);
    if (handle == INVALID_HANDLE_VALUE) 
    {
        return MSL_INVALID_HANDLE_VALUE;
    }

    // Get file information
    if (!GetFileInformationByHandle(handle, result)) 
    {
        CloseHandle(handle);
        return MSL_EXTERNAL_ERROR;
    }

    CloseHandle(handle);
    return MSL_SUCCESS;
}

int paths_are_equivalent(const char* path1, const char* path2, int* equivalent) 
{
    int last_status = MSL_SUCCESS;
    *equivalent = 0;
    if (!path1 || !path2) 
    {
        return MSL_INVALID_PARAMETER;
    }

    // Get full paths first
    char full_path1[MAX_PATH];
    char full_path2[MAX_PATH];

    if (!GetFullPathNameA(path1, MAX_PATH, full_path1, NULL) ||
        !GetFullPathNameA(path2, MAX_PATH, full_path2, NULL)) 
    {
        return MSL_INVALID_PARAMETER;
    }

    // If paths are identical after normalization, they're equivalent
    if (stricmp(full_path1, full_path2) == 0) 
    {
        return MSL_SUCCESS;
    }

    BY_HANDLE_FILE_INFORMATION info1;
    BY_HANDLE_FILE_INFORMATION info2;
    // Get file information for both paths
    CALL(get_file_info, full_path1, &info1);
    CALL(get_file_info, full_path2, &info2);

    // Compare volume serial numbers and file IDs
    if (info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
        info1.nFileIndexHigh == info2.nFileIndexHigh &&
        info1.nFileIndexLow == info2.nFileIndexLow) 
    {
        *equivalent = 1;
    }

    return MSL_SUCCESS;
}

hash_t hash_key_int(int key)
{
    return (key * -0x61c8864f + 1) & INT_MAX;
}

hash_t hash_key_yyobject_base_t(yyobject_base_t* key)
{
    return (((unsigned long long)((uintptr_t)(key)) >> 6) * 7 + 1) & INT_MAX;
}

hash_t hash_key_ptr(void* key)
{
    return (((unsigned long long)((uintptr_t)(key)) >> 8) + 1) & INT_MAX;
};

hash_t hash_key_str(const char* key)
{
    // https://github.com/jwerle/murmurhash.c - Licensed under MIT
    size_t len = strlen(key);
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;
    uint32_t h = 0;
    uint32_t k = 0;
    uint8_t* d = (uint8_t*)key; // 32 bit extract from 'key'
    const uint32_t* chunks = NULL;
    const uint8_t* tail = NULL; // tail - last 8 bytes
    int i = 0;
    int l = len / 4; // chunk length

    chunks = (const uint32_t*)(d + l * 4); // body
    tail = (const uint8_t*)(d + l * 4); // last 8 byte chunk of `key'

    // for each 4 byte chunk of `key'
    for (i = -l; i != 0; ++i) 
    {
        // next 4 byte chunk of `key'
        k = chunks[i];

        // encode next 4 byte chunk of `key'
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        // append to hash
        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }

    k = 0;

    // remainder
    switch (len & 3) // `len % 4'
    {
        case 3: k ^= (tail[2] << 16);
        case 2: k ^= (tail[1] << 8);

        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;
            h ^= k;
    }

    h ^= len;

    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;
}