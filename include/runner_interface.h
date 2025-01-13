	// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "gml_structs.h"
#ifndef RUNNER_INTERFACE_H_
#define RUNNER_INTERFACE_H_

typedef enum EBUFFER_TYPE EBUFFER_TYPE;
typedef enum EBUFFER_SEEK EBUFFER_SEEK;

typedef void* HYYMUTEX;
typedef void* HSPRITEASYNC;
typedef struct http_req_context_s http_req_context_t;
typedef struct ibuffer_vtable_s ibuffer_vtable_t;
typedef struct ibuffer_s ibuffer_t;

typedef int (*PFUNC_async)(http_req_context_t* _p_context, void* _p_payload, int* _p_map);
typedef void (*PFUNC_cleanup)(http_req_context_t* _p_context);
typedef void (*PFUNC_process)(http_req_context_t* _p_context);

enum EBUFFER_TYPE
{
    EBUFFER_None = 0x0,
    EBUFFER_U8 = 0x1,
    EBUFFER_S8 = 0x2,
    EBUFFER_U16 = 0x3,
    EBUFFER_S16 = 0x4,
    EBUFFER_U32 = 0x5,
    EBUFFER_S32 = 0x6,
    EBUFFER_F16 = 0x7,
    EBUFFER_F32 = 0x8,
    EBUFFER_D64 = 0x9,
    EBUFFER_Bool = 0xA,
    EBUFFER_String = 0xB,
    EBUFFER_U64 = 0xC,
    EBUFFER_Text = 0xD,
};

enum EBUFFER_SEEK
{
    EBUFFER_Start = 0x0,
    EBUFFER_Relative = 0x1,
    EBUFFER_End = 0x2,
};

struct ibuffer_vtable_s
{
    void (*Destroy)(ibuffer_t* self);
    int (*Write)(EBUFFER_TYPE _type, rvalue_t* _p_in);
    int (*WriteArray)(EBUFFER_TYPE _type, uint8_t* _pSrc, int size);
    int (*Read)(EBUFFER_TYPE _type, rvalue_t* _p_out);
    int (*Seek)(EBUFFER_SEEK _type, int _val);
    void (*Peek)(int _offset, EBUFFER_TYPE _type, rvalue_t* _p_out);
    void (*Poke)(int _offset, EBUFFER_TYPE _type, rvalue_t* _p_in);
    int (*Save)(const char* _p_uRI, int _offset, int _size);
    int (*Load)(const char* _p_uRI, int _src_offset, int _src_size, int _dest_offset);
    void (*Base64Encode)(rvalue_t* _p_out, int _offset, int _size);
    void (*Base64Decode)(const char* _pBASE64, int _offset, int _size);
    void (*MD5)(rvalue_t* _p_out, int _offset, int _size);
    void (*SHA1)(rvalue_t* _p_out, int _offset, int _size);
    void (*Resize)(int _newsize);
    void (*Copy)(int _src_offset, int _src_size, ibuffer_t* _pDest, int _dest_off);
    void (*Fill)(int _offset, int _size, EBUFFER_TYPE _type, rvalue_t* _p_in, int _stride, bool fill_gaps);
    void (*GetSurface)(int _surface);
    void (*SetSurface)(int _surface, int _offset);
    uint8_t* (*Compress)(int _offset, int _size, uint32_t& resultSize);
    uint8_t* (*Decompress)(uint32_t& resultSize);
};

struct ibuffer_s {
    const ibuffer_vtable_s* vt;  // Same size as C++ vtable pointer
};
#ifdef _WIN64
	COMPILE_TIME_ASSERT(sizeof(ibuffer_t) == 0x8);
#endif // _WIN64

typedef struct yyrunner_interface_s
{
    // ########################################################################
    // BASIC INTERACTION WITH THE USER
    // ######################################################################## 

    /**
     * @brief Writes a formatted string to the debug console output, analogous to printf.
     *
     * @param fmt A string that contains the text to be written to the output. It can include embedded format tags.
     * @param ... (Variadic arguments) A variable number of arguments to be embedded in the format string.
     *
     * Usage example:
     *
     *		// Writes to the debug console
        *		debug_console_output("This is a number: %d", a_number);
        */
    void (*debug_console_output)(const char* fmt, ...);

    /**
     * @brief Writes a formatted string to the release console output, analogous to printf.
     *
     * @param fmt A string that contains the text to be written to the output. It can include embedded format tags.
     * @param ... (Variadic arguments) A variable number of arguments to be embedded in the format string.
     *
     * Usage example:
     *
     *		// Writes to the release console
        *		release_console_output("This is a number: %d", a_number);
        */
    void (*release_console_output)(const char* fmt, ...);

    /**
     * @brief Displays a popup message on the runner side, typically to convey information or warnings to the user.
     *
     * @param msg The string message to be displayed in the popup.
     *
     * Usage example:
     *
     *		// Displays a message to the user
        *		show_message("Hello from the other side!");
        */
    void (*show_message)(const char* msg);

    /**
     * @brief Writes a formatted error message to the error console and triggers a runtime error.
     *
     * @param _error A string that contains the text of the error message. It can include embedded format tags.
     * @param ... (Variadic arguments) A variable number of arguments to be embedded in the error string.
     *
     * Usage example:
     *
     *		// Triggers a runtime error with a custom message
        *		YYError("MyFunction :: incorrect number of arguments");
        */
    void (*YYError)(const char* _error, ...);


    // ########################################################################
    // MEMORY MANAGEMENT
    // ######################################################################## 

    /**
     * @brief Allocates a block of memory of size _size.
     *
     * @param _size The size of the memory block to allocate, in bytes.
     *
     * @return A pointer to the allocated memory block. The pointer is
     * null if the function fails to allocate memory.
     *
     * Usage example:
     *
     *		// Allocates a block of memory of size 10 bytes.
        *		void* pMemoryBlock = YYAlloc(10);
        */
    void* (*YYAlloc)(int _size);

    /**
     * @brief Reallocates a block of memory to a new size.
     *
     * @param pOriginal A pointer to the memory block originally allocated
     * with YYAlloc or YYRealloc.
     * @param _newSize The new size of the memory block, in bytes.
     *
     * @return A pointer to the reallocated memory block. This might be
     * different from pOriginal if the function had to move the memory
     * block to enlarge it. Returns null if the function fails to reallocate
     * memory.
     *
     * Usage example:
     *
     *		// Reallocate pMemoryBlock to 20 bytes.
        *		void* pNewMemoryBlock = YYRealloc(pMemoryBlock, 20);
        */
    void* (*YYRealloc)(void* pOriginal, int _newSize);

    /**
     * @brief Frees a block of memory that was previously allocated.
     *
     * @param p A pointer to the memory block to be freed. This should have been
     * returned by a previous call to YYAlloc or YYRealloc.
     *
     * Usage example:
     *
     *		// Frees the allocated memory block.
        *		YYFree(pMemoryBlock);
        */
    void  (*YYFree)(const void* p);

    /**
     * @brief Duplicates a string by allocating memory for a new string and copying the
     * content.
     *
     * @param _pS A pointer to the null-terminated string to duplicate.
     *
     * @return A pointer to the newly allocated string with the same content as _pS.
     * Returns null if the function fails to allocate memory. The returned string must be
     * freed using YYFree when it is no longer needed.
     *
     * Usage example:
     *
     *		// Duplicates the string.
        *		const char* duplicatedString = YYStrDup(originalString);
        */
    const char* (*YYStrDup)(const char* _pS);


    // ########################################################################
    // ARGUMENT PARSING
    // ######################################################################## 

    /**
     * @brief Parses and retrieves a boolean value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the boolean value.
     *
     * @return The boolean value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		bool value = YYGetBool(pArgs, 0);
        */
    bool (*YYGetBool)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a float value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the float value.
     *
     * @return The float value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		float value = YYGetFloat(pArgs, 1);
        */
    float (*YYGetFloat)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a double-precision floating-point value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the double value.
     *
     * @return The double-precision floating-point value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		double value = YYGetReal(pArgs, 2);
        */
    double (*YYGetReal)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a 32-bit integer value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the 32-bit integer value.
     *
     * @return The 32-bit integer value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		int32_t value = YYGetInt32(pArgs, 3);
        */
    int32_t(*YYGetInt32)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a 32-bit unsigned integer value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the 32-bit unsigned integer value.
     *
     * @return The 32-bit unsigned integer value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		uint32_t value = YYGetUint32(pArgs, 4);
        */
    uint32_t(*YYGetUint32)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a 64-bit integer value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the 64-bit integer value.
     *
     * @return The 64-bit integer value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		int64 value = YYGetInt64(pArgs, 5);
        */
    int64_t(*YYGetInt64)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a pointer value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the pointer value.
     *
     * @return The pointer value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		void* value = YYGetPtr(pArgs, 6);
        */
    void* (*YYGetPtr)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves either a pointer value or an integer value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the pointer or integer value.
     *
     * @return The pointer or integer value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		intptr_t value = YYGetPtrOrInt(pArgs, 7);
        */
    intptr_t(*YYGetPtrOrInt)(const rvalue_t* _pBase, int _index);

    /**
     * @brief Parses and retrieves a string value from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the string value.
     *
     * @return The string value present at the specified index in the argument array.
     *
     * Usage example:
     *
     *		const char* value = YYGetString(pArgs, 8);
        *
        * @note The user should not free the returned const char*, as it is managed by the internal memory management system.
        *       However, it is essential to duplicate it if you need to store it since the runner invalidates these strings at
        *       the end of each step.
        */
    const char* (*YYGetString)(const rvalue_t* _pBase, int _index);


    // ########################################################################
    // rvalue_t PARSING
    // ######################################################################## 

    /**
     * @brief Attempts to retrieve a boolean value by parsing|casting from a single rvalue_t.
     *
     * @param _pValue A pointer to the rvalue_t.
     *
     * @return The boolean value obtained by casting the specified rvalue_t.
     *
     * Usage example:
     *
     *		bool value = BOOL_rvalue_t(pValue);
        */
    bool (*BOOL_rvalue_t)(const rvalue_t* _pValue);

    /**
     * @brief Attempts to retrieve a double-precision floating-point value by parsing|casting from a single rvalue_t.
     *
     * @param _pValue A pointer to the rvalue_t.
     *
     * @return The double-precision floating-point value obtained by casting the specified rvalue_t.
     *
     * Usage example:
     *
     *		double value = REAL_rvalue_t(pValue);
        */
    double (*REAL_rvalue_t)(const rvalue_t* _pValue);

    /**
     * @brief Attempts to retrieve a pointer value by parsing|casting from a single rvalue_t.
     *
     * @param _pValue A pointer to the rvalue_t.
     *
     * @return The pointer value obtained by casting the specified rvalue_t.
     *
     * Usage example:
     *
     *		void* value = PTR_rvalue_t(pValue);
        */
    void* (*PTR_rvalue_t)(const rvalue_t* _pValue);

    /**
     * @brief Attempts to retrieve a 64-bit integer value by parsing|casting from a single rvalue_t.
     *
     * @param _pValue A pointer to the rvalue_t.
     *
     * @return The 64-bit integer value obtained by casting the specified rvalue_t.
     *
     * Usage example:
     *
     *		int64 value = INT64_rvalue_t(pValue);
        */
    int64_t(*INT64_rvalue_t)(const rvalue_t* _pValue);

    /**
     * @brief Attempts to retrieve a 32-bit integer value by parsing|casting from a single rvalue_t.
     *
     * @param _pValue A pointer to the rvalue_t.
     *
     * @return The 32-bit integer value obtained by casting the specified rvalue_t.
     *
     * Usage example:
     *
     *		int32_t value = INT32_rvalue_t(pValue);
        */
    int32_t(*INT32_rvalue_t)(const rvalue_t* _pValue);


    // ########################################################################
    // HASHING
    // ########################################################################

    /**
     * @brief Calculates a hash value from a given rvalue_t.
     *
     * @param _pValue A pointer to the rvalue_t from which to calculate the hash value.
     *
     * @return The calculated hash value for the specified rvalue_t.
     *
     * Usage example:
     *
     *		int hashValue = HASH_rvalue_t(pValue);
        */
    int (*HASH_rvalue_t)(const rvalue_t* _pValue);


    // ########################################################################
    // COPYING, GETTING, SETTING & FREEING rvalue_tS
    // ########################################################################

    /**
     * @brief Assigns an rvalue_t to another, considering a given context and index.
     *
     * @param _pDest Pointer to the destination rvalue_t.
     * @param _pV Pointer to the source rvalue_t.
     * @param _pPropSelf Pointer to the yyobject_base_t, representing the "self" or context.
     * @param _index Index within an array of rvalue_ts, if applicable.
     *
     * Usage example:
     *
     *		SET_rvalue_t(&destinationrvalue_t, &sourcervalue_t, pContext, arrayIndex);
        */
    void (*SET_rvalue_t)(rvalue_t* _pDest, rvalue_t* _pV, yyobject_base_t* _pPropSelf, int _index);

    /**
     * @brief Retrieves an rvalue_t, considering a given context and index, and potentially prepares an array.
     *
     * @param _pRet Pointer to the rvalue_t where the result will be stored.
     * @param _pV Pointer to the source rvalue_t.
     * @param _pPropSelf Pointer to the yyobject_base_t, representing the "self" or context.
     * @param _index Index within an array of rvalue_ts, if applicable.
     * @param fPrepareArray Boolean flag indicating whether to prepare an array.
     * @param fPartOfSet Boolean flag indicating whether this action is part of a set operation.
     *
     * @return Boolean indicating the success of the get operation.
     *
     * @note Ensure to call FREE_rvalue_t on the retrieved rvalue_t once done to avoid memory leaks.
     */
    bool (*GET_rvalue_t)(rvalue_t* _pRet, rvalue_t* _pV, yyobject_base_t* _pPropSelf, int _index, bool fPrepareArray, bool fPartOfSet);

    /**
     * @brief Copies the value from one rvalue_t to another.
     *
     * @param _pDest Pointer to the destination rvalue_t.
     * @param _pSource Pointer to the source rvalue_t.
     *
     * Usage example:
     *
     *		COPY_rvalue_t(&destinationrvalue_t, &sourcervalue_t);
        */
    void (*COPY_rvalue_t)(rvalue_t* _pDest, const rvalue_t* _pSource);

    /**
     * @brief Retrieves the kind/type of the given rvalue_t.
     *
     * @param _pValue Pointer to the rvalue_t to retrieve the kind from.
     *
     * @return Integer representing the kind/type of the rvalue_t.
     */
    int (*KIND_rvalue_t)(const rvalue_t* _pValue);

    /**
     * @brief Frees the memory associated with a given rvalue_t.
     *
     * @param _pValue Pointer to the rvalue_t to be freed.
     *
     * Usage example:
     *
     *		FREE_rvalue_t(&myrvalue_t);
        *
        * @note Always call this function after you are done using an rvalue_t to prevent memory leaks.
        */
    void (*FREE_rvalue_t)(rvalue_t* _pValue);

    /**
     * @brief Creates a new string rvalue_t.
     *
     * @param _pVal Pointer to the rvalue_t where the string will be stored.
     * @param _pS Pointer to the source string.
     *
     * Usage example:
     *
     *		YYCreateString(&myrvalue_t, "Hello, world!");
        */
    void (*YYCreateString)(rvalue_t* _pVal, const char* _pS);

    /**
     * @brief Creates a new array rvalue_t.
     *
     * @param prvalue_t Pointer to the rvalue_t where the array will be stored.
     * @param n_values The number of elements (double values) to store in the array.
     * @param values Pointer to the array of double values that should be stored.
     *
     * Usage example:
     *
     *		double myValues[] = {1.0, 2.0, 3.0};
        *		YYCreateArray(&myrvalue_t, 3, myValues);
        *
        * @note This function initializes an rvalue_t as an array and populates it with the provided double values.
        */
    void (*YYCreateArray)(rvalue_t* prvalue_t, int n_values, const double* values);

    // Finding and running user scripts from name
    int (*Script_Find_Id)(const char* name);
    bool (*Script_Perform)(int ind, instance_t* selfinst, instance_t* otherinst, int argc, rvalue_t* res, rvalue_t* arg);

    // Finding builtin functions
    bool  (*Code_Function_Find)(const char* name, int* ind);

    // HTTP functions
    void (*HTTP_Get)(const char* _pFilename, int _type, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
    void (*HTTP_Post)(const char* _pFilename, const char* _pPost, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
    void (*HTTP_Request)(const char* _url, const char* _method, const char* _headers, const char* _pBody, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV, int _contentLength);

    // sprite loading helper functions
    int (*ASYNCFunc_SpriteAdd)(http_req_context_t* _p_context, void* _p, int* _p_map);
    void (*ASYNCFunc_SpriteCleanup)(http_req_context_t* _p_context);
    HSPRITEASYNC(*CreateSpriteAsync)(int* _pSpriteIndex, int _xOrig, int _yOrig, int _numImages, int _flags);


    // ########################################################################
    // TIMING
    // ########################################################################

    /**
     * @brief Retrieves the current time in microseconds (since start).
     *
     * This function returns the current time calculated from the start of the app,
     * measured in microseconds. It provides high-resolution time data useful for precise
     * timing, benchmarking, and profiling.
     *
     * @return An int64 representing the number of microseconds elapsed since the start of the app.
     *
     * @note Be mindful of the data type sizes when performing calculations to avoid overflow.
     */
    int64_t(*Timing_Time)(void);

    /**
     * @brief Pauses the execution of the main thread for a specified time.
     *
     * `Timing_Sleep` halts the execution of the calling thread for at least the
     * specified duration in microseconds, permitting the system to execute other threads.
     * The precision of the sleep duration can be influenced by the `precise` parameter.
     *
     * @param slp The duration for which the thread will sleep, in microseconds.
     * @param precise Boolean flag indicating whether to enforce precise sleeping.
     *        - true:  Aims to ensure the sleep duration is as exact as possible.
     *        - false: The sleep duration may be slightly shorter or longer.
     *
     * @note Prolonged and precise sleep durations might impact system power usage
     *       and performance adversely. Use precise sleeps judiciously, balancing
     *       requirement against resource utilization.
     */
    void (*Timing_Sleep)(int64_t slp, bool precise);


    // ########################################################################
    // MUTEX HANDLING
    // ########################################################################

    /**
     * Mutexes, or mutual exclusions, are used in concurrent programming to avoid the simultaneous
     * execution of a piece of code that accesses shared resources, such as runtime elements like ds_maps,
     * ds_lists, and buffers, by multiple threads.
     *
     * This helps in avoiding race conditions, ensuring that the shared resources are accessed in a mutually
     * exclusive manner. When a thread acquires a lock on a mutex, any other thread attempting to acquire the
     * same lock will block until the first thread releases the lock.
     */

        /**
         * @brief Creates a new mutex with the given name.
         *
         * @param _name A string representing the name of the mutex to be created.
         *
         * @return A handle to the newly created mutex.
         *
         * Usage example:
         *
         *		HYYMUTEX myMutex = YYMutexCreate("myMutexName");
        *
        * @note This is the runner's own implementation of mutexes, used for ensuring controlled access
        *       to shared resources in a concurrent environment.
        */
    HYYMUTEX(*YYMutexCreate)(const char* _name);

    /**
     * @brief Destroys a given mutex.
     *
     * @param hMutex A handle to the mutex to be destroyed.
     *
     * Usage example:
     *
     *		YYMutexDestroy(myMutex);
        */
    void (*YYMutexDestroy)(HYYMUTEX hMutex);

    /**
     * @brief Locks a given mutex.
     *
     * @param hMutex A handle to the mutex to be locked.
     *
     * Usage example:
     *
     *		YYMutexLock(myMutex);
        *
        * @note This function will block if the mutex is already locked by another thread,
        *       and it will return once the mutex has been successfully locked.
        */
    void (*YYMutexLock)(HYYMUTEX hMutex);

    /**
     * @brief Unlocks a given mutex.
     *
     * @param hMutex A handle to the mutex to be unlocked.
     *
     * Usage example:
     *
     *		YYMutexUnlock(myMutex);
        *
        * @note After unlocking, any other threads waiting to lock the mutex will be unblocked.
        */
    void (*YYMutexUnlock)(HYYMUTEX hMutex);


    // ########################################################################
    // ASYNC EVENTS
    // ########################################################################

    /**
     * @brief Triggers an asynchronous event, passing a ds_map to it.
     *
     * @param _map The index of the ds_map to pass to the event.
     * @param _event The event code number to trigger.
     *
     * Usage example:
     *
     *		CreateAsyncEventWithDSMap(myDsMap, myEventCode);
        *
        * @note The ds_map passed to the event should not be manually freed post-triggering.
        */
    void (*CreateAsyncEventWithDSMap)(int _map, int _event);

    /**
     * @brief Triggers an asynchronous event, passing a ds_map and a GML buffer to it.
     *
     * @param _map The index of the ds_map to pass to the event.
     * @param _buffer The index of the GML buffer to pass to the event.
     * @param _event The event code number to trigger.
     *
     * Usage example:
     *
     *		CreateAsyncEventWithDSMapAndBuffer(myDsMap, myBuffer, myEventCode);
        *
        * @note The ds_map and the GML buffer should not be manually freed post-triggering.
        *       They will be automatically freed once the event is finished.
        */
    void (*CreateAsyncEventWithDSMapAndBuffer)(int _map, int _buffer, int _event);


    // ########################################################################
    // DS_MAP MANIPULATION
    // ########################################################################

    /**
     * @brief Creates a new ds_map with specified key-value pairs.
     *
     * @param _num The number of key-value pairs to be added to the ds_map.
     * @param ... Key and value pairs to be added to the ds_map. Only double and string (char*) types are supported.
     *
     * @return The index referencing the created ds_map.
     *
     * Usage example:
     *
     *		int myDsMap = CreateDsMap(2, "key1", 1.23, "key2", "value2");
        *
        * @note Ensure that the keys and values are passed in pairs and the count (_num) matches the total number of arguments passed.
        */
    int (*CreateDsMap)(int _num, ...);

    /**
     * @brief Adds a key-double pair to a ds_map.
     *
     * @param _index The index of the ds_map to which the key-value pair will be added.
     * @param _pKey The key to be associated with the value.
     * @param value The double value to be added.
     *
     * @return True if the addition is successful, otherwise False.
     *
     * Usage example:
     *
     *		bool success = DsMapAddDouble(myDsMap, "key3", 3.45);
        */
    bool (*DsMapAddDouble)(int _index, const char* _pKey, double value);

    /**
     * @brief Adds a key-string pair to a ds_map.
     *
     * @param _index The index of the ds_map to which the key-value pair will be added.
     * @param _pKey The key to be associated with the value.
     * @param pVal The string value to be added.
     *
     * @return True if the addition is successful, otherwise False.
     *
     * Usage example:
     *
     *		bool success = DsMapAddString(myDsMap, "key4", "value4");
        */
    bool (*DsMapAddString)(int _index, const char* _pKey, const char* pVal);

    /**
     * @brief Adds a key-int64 pair to a ds_map.
     *
     * @param _index The index of the ds_map to which the key-value pair will be added.
     * @param _pKey The key to be associated with the value.
     * @param value The int64 value to be added.
     *
     * @return True if the addition is successful, otherwise False.
     *
     * Usage example:
     *
     *		bool success = DsMapAddInt64(myDsMap, "key5", 123456789012345);
        */
    bool (*DsMapAddInt64)(int _index, const char* _pKey, int64_t value);


    // ########################################################################
    // BUFFER ACCESS
    // ########################################################################

    /**
     * @brief Retrieves the content of a GameMaker Language (GML) buffer.
     *
     * @param _index The index of the GML buffer to get content from.
     * @param _ppData A pointer to store the address of the retrieved content.
     * @param _pDataSize A pointer to store the size of the retrieved content.
     *
     * @return A boolean indicating the success of the retrieval.
     *
     * Usage example:
     *
     *		void* data;
        *		int dataSize;
        *		bool success = BufferGetContent(bufferIndex, &data, &dataSize);
        *
        * @note The method creates a copy of the data from the runner, which needs to be properly freed to prevent memory leaks.
        */
    bool (*BufferGetContent)(int _index, void** _ppData, int* _pDataSize);

    /**
     * @brief Writes content into a GML buffer.
     *
     * @param _index The index of the GML buffer to write content to.
     * @param _dest_offset The destination offset within the GML buffer to start writing data.
     * @param _pSrcMem A pointer to the source memory to write from.
     * @param _size The size of the data to write.
     * @param _grow A boolean indicating whether the buffer should grow if necessary.
     * @param _wrap A boolean indicating whether the buffer should wrap if necessary.
     *
     * @return The number of bytes written to the buffer.
     *
     * Usage example:
     *
     *		int bytesWritten = BufferWriteContent(bufferIndex, offset, srcData, dataSize, true, false);
        */
    int (*BufferWriteContent)(int _index, int _dest_offset, const void* _pSrcMem, int _size, bool _grow, bool _wrap);

    /**
     * @brief Creates a new GML buffer.
     *
     * @param _size The size of the new buffer.
     * @param _bf The format of the new buffer, specified using EBUFFER_Format enum.
     * @param _alignment The memory alignment of the new buffer.
     *
     * @return The index of the newly created buffer.
     *
     * Usage example:
     *
     *		int newBufferIndex = CreateBuffer(1024, EBUFFER_Format_Grow, 0);
        *
        * Enum values for EBUFFER_Format:
        * - EBUFFER_Format_Fixed:    0
        * - EBUFFER_Format_Grow:     1
        * - EBUFFER_Format_Wrap:     2
        * - EBUFFER_Format_Fast:     3
        * - EBUFFER_Format_VBuffer:  4
        * - EBUFFER_Format_Network:  5
        */
    int (*CreateBuffer)(int _size, enum EBUFFER_Format _bf, int _alignment);


    // ########################################################################
    // VARIABLES
    // ########################################################################

    volatile bool* pLiveConnection;
    int* pHTTP_ID;


    // ########################################################################
    // DS_LIST AND DS_MAP MANIPULATION
    // ########################################################################

    /**
     * @brief Creates a new ds_list.
     *
     * @return The index of the newly created ds_list.
     *
     * Usage example:
     *
     *		int newListIndex = DsListCreate();
        */
    int (*DsListCreate)();

    /**
     * @brief Adds a ds_list to a ds_map with a specified key.
     *
     * @param _dsMap The index of the ds_map.
     * @param _key The key associated with the ds_list in the ds_map.
     * @param _listIndex The index of the ds_list to add to the ds_map.
     *
     * @note When a ds_list added to a ds_map using DsMapAddList is removed
     *       (e.g., via DsMapClear), it is automatically freed by the runner
     *       and does not require manual freeing.
     *
     * Usage example:
     *
     *		DsMapAddList(mapIndex, "myListKey", listIndex);
        */
    void (*DsMapAddList)(int _dsMap, const char* _key, int _listIndex);

    /**
     * @brief Adds a ds_map to a ds_list.
     *
     * @param _dsList The index of the ds_list.
     * @param _mapIndex The index of the ds_map to add to the ds_list.
     *
     * @note When a ds_map added to a ds_list using DsListAddMap is removed
     *       (e.g., via DsListClear), it is also automatically freed by the
     *       runner and does not require manual freeing.
     *
     * Usage example:
     *
     *		DsListAddMap(listIndex, mapIndex);
        */
    void (*DsListAddMap)(int _dsList, int _mapIndex);

    /**
     * @brief Clears all key-value pairs from a ds_map.
     *
     * @param _dsMap The index of the ds_map to clear.
     *
     * @note Clearing a ds_map with DsMapClear will also automatically free
     *       any ds_lists that have been added to it with DsMapAddList,
     *       ensuring that no memory leaks occur.
     *
     * Usage example:
     *
     *		DsMapClear(mapIndex);
        */
    void (*DsMapClear)(int _dsMap);

    /**
     * @brief Clears all elements from a ds_list.
     *
     * @param _dsList The index of the ds_list to clear.
     *
     * @note Clearing a ds_list with DsListClear will also automatically free
     *       any ds_maps that have been added to it with DsListAddMap,
     *       avoiding any memory leaks.
     *
     * Usage example:
     *
     *		DsListClear(listIndex);
        */
    void (*DsListClear)(int _dsList);


    // ########################################################################
    // FILES
    // ########################################################################

    bool (*BundleFileExists)(const char* _pszFileName);
    bool (*BundleFileName)(char* _name, int _size, const char* _pszFileName);
    bool (*SaveFileExists)(const char* _pszFileName);
    bool (*SaveFileName)(char* _name, int _size, const char* _pszFileName);


    // ########################################################################
    // BASE64 ENCODE
    // ########################################################################

    /**
     * @brief Encodes binary data using Base64 encoding.
     *
     * This function takes binary data as input and provides a Base64-encoded output.
     * Base64 encoding is commonly used to encode binary data, especially when that data
     * needs to be stored and transferred over media that is designed to deal with text.
     * It can be used to encode data in HTTP post calls or to encode data read from a GML buffer, for example.
     *
     * @param input_buf Pointer to the binary data to be encoded.
     * @param input_len Length (in bytes) of the data to be encoded.
     * @param output_buf Pointer to the buffer where the encoded data will be stored.
     * @param output_len Length of the output buffer. This should be at least 4/3 times the input length, to ensure that there is enough space to hold the encoded data.
     *
     * @return Returns `true` if the encoding is successful, and `false` otherwise. The function might fail if there is not enough space in the output buffer to hold the encoded data.
     *
     * Usage example:
     *
     *		const char* binaryData = "Binary data goes here";
        *		size_t binaryDataLength = strlen(binaryData);
        *		char encodedData[200]; // Ensure the output buffer is large enough
        *
        *		bool success = Base64Encode(binaryData, binaryDataLength, encodedData, sizeof(encodedData));
        *
        * @note Ensure the output buffer is large enough to store the encoded data.
        */
    bool (*Base64Encode)(const void* input_buf, size_t input_len, void* output_buf, size_t output_len);

    // ########################################################################
    // DS_LIST MANIPULATION
    // ########################################################################

    /**
     * @brief Adds a 64-bit integer value to a ds_list.
     *
     * Appends a 64-bit integer (int64) to the specified ds_list,
     * ensuring that large integer values can be stored without truncation
     * or data loss. Ds_lists are dynamic arrays that can store different
     * types of data, in this case, a 64-bit integer.
     *
     * @param _dsList The index of the ds_list to which the 64-bit integer will be added.
     * @param _value The 64-bit integer value to be added to the ds_list.
     *
     * @note Ensure the ds_list index provided is valid and the ds_list is properly initialized.
     *
     * Usage example:
     *
     *		int myDsList = DsListCreate(); // Assume DsListCreate is a function that creates a ds_list
        *		int64 myValue = 1234567890123456789;
        *		DsListAddInt64(myDsList, myValue);
        */
    void (*DsListAddInt64)(int _dsList, int64_t _value);


    // ########################################################################
    // FILE & DIRECTORY WHITELISTING
    // ########################################################################

    void (*AddDirectoryToBundleWhitelist)(const char* _pszFilename);
    void (*AddFileToBundleWhitelist)(const char* _pszFilename);
    void (*AddDirectoryToSaveWhitelist)(const char* _pszFilename);
    void (*AddFileToSaveWhitelist)(const char* _pszFilename);

    // ########################################################################
    // UTILITIES
    // ########################################################################

    /**
     * @brief Retrieves the string representation of the kind/type of an rvalue_t.
     *
     * The function obtains the kind of an rvalue_t, representing its type, and returns
     * its string representation. Useful for debugging, logging, or any situation
     * where the textual depiction of an rvalue_t's type is necessary.
     *
     * @param _pV A pointer to the rvalue_t whose kind/type name is to be retrieved.
     *
     * @return A string literal representing the kind/type of the given rvalue_t.
     *
     * @note The returned string should not be freed or modified.
     *
     * Usage example:
     *
     *		const rvalue_t myValue = ...; // Assume this is populated appropriately
        *		const char* typeName = KIND_NAME_rvalue_t(&myValue);
        *		printf("The type of myValue is: %s\n", typeName);
        */
    const char* (*KIND_NAME_rvalue_t)(const rvalue_t* _pV);

    // ########################################################################
    // DS_MAP MANIPULATION (PART 2)
    // ########################################################################

    /**
     * @brief Adds a key-boolean pair to a ds_map.
     *
     * @param _index The index of the ds_map to which the key-value pair will be added.
     * @param _pKey The key to be associated with the value.
     * @param value The boolean value to be added.
     *
     * Usage example:
     *
     *		DsMapAddBool(myDsMap, "keyBool", true);
        *
        * @note Ensure the ds_map referred to by _index is valid and created before using this function.
        */
    void (*DsMapAddBool)(int _index, const char* _pKey, bool value);

    /**
     * @brief Adds a key-rvalue_t pair to a ds_map.
     *
     * @param _index The index of the ds_map to which the key-value pair will be added.
     * @param _pKey The key to be associated with the value.
     * @param value A pointer to the rvalue_t to be added.
     *
     * Usage example:
     *
     *		rvalue_t myValue;
        *		// ... (initialize and set myValue)
        *		DsMapAddrvalue_t(myDsMap, "keyrvalue_t", &myValue);
        *
        * @note Ensure the ds_map referred to by _index is valid and created before using this function.
        * @note Ensure the rvalue_t is properly initialized and set before using it as a parameter.
        */
    void (*DsMapAddrvalue_t)(int _index, const char* _pKey, rvalue_t* value);

    /**
     * @brief Frees a ds_map from memory.
     *
     * @param _index The index of the ds_map to be freed.
     *
     * Usage example:
     *
     *		DestroyDsMap(myDsMap);
        *
        * @note Maps utilized to trigger asynchronous events do not require manual
        *       freeing and will be automatically freed post-event triggering.
        */
    void (*DestroyDsMap)(int _index);

    // ########################################################################
    // STRUCT MANIPULATION
    // ########################################################################

    /**
     * @brief Initializes a new structure in the given rvalue_t.
     *
     * @param _pStruct A pointer to the rvalue_t to initialize as a structure.
     *
     * Usage example:
     *
     *		rvalue_t pStruct = {0};
        *		StructCreate(&pStruct);
        */
    void (*StructCreate)(rvalue_t* _pStruct);

    /**
     * @brief Adds a boolean value to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _value The boolean value to add to the structure.
     *
     * Usage example:
     *
     *		StructAddBool(pStruct, "key", true);
        */
    void (*StructAddBool)(rvalue_t* _pStruct, const char* _pKey, bool _value);

    /**
     * @brief Adds a double-precision floating-point value to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _value The double value to add to the structure.
     *
     * Usage example:
     *
     *		StructAddDouble(pStruct, "key", 3.14);
        */
    void (*StructAddDouble)(rvalue_t* _pStruct, const char* _pKey, double _value);

    /**
     * @brief Adds an integer value to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _value The integer value to add to the structure.
     *
     * Usage example:
     *
     *		StructAddInt(pStruct, "key", 42);
        */
    void (*StructAddInt)(rvalue_t* _pStruct, const char* _pKey, int _value);

    /**
     * @brief Adds an rvalue_t to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _pValue A pointer to the rvalue_t to add to the structure.
     *
     * Usage example:
     *
     *		StructAddrvalue_t(pStruct, "key", pValue);
        */
    void (*StructAddrvalue_t)(rvalue_t* _pStruct, const char* _pKey, rvalue_t* _pValue);

    /**
     * @brief Adds a string value to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _pValue The string value to add to the structure.
     *
     * Usage example:
     *
     *		StructAddString(pStruct, "key", "value");
        */
    void (*StructAddString)(rvalue_t* _pStruct, const char* _pKey, const char* _pValue);

    // ########################################################################
    // DIRECTORY MANIPULATION
    // ########################################################################

    bool (*WhitelistIsDirectoryIn)(const char* _pszDirectory);
    bool (*WhiteListIsFilenameIn)(const char* _pszFilename);
    void (*WhiteListAddTo)(const char* _pszFilename, bool _bIsDir);
    bool (*DirExists)(const char* filename);

    // ########################################################################
    // BUFFER ACCESS (ADV)
    // ########################################################################

    /**
     * @brief Retrieves an `ibuffer_t` interface corresponding to a GML buffer.
     *
     * This function retrieves an `ibuffer_t` interface for a given GML buffer (specified by an index).
     * The `ibuffer_t` interface itself isn't directly accessible through the C++ API but can be used with
     * `BufferGet` to access the data of a GML buffer from the runner without copying the data.
     *
     * @param ind The index of the GML buffer.
     *
     * @return A pointer to an `ibuffer_t` interface struct.
     */
    ibuffer_t* (*BufferGetFromGML)(int ind);

    /**
     * @brief Gets the current read position within a buffer.
     *
     * This function returns the current position of the read cursor within a buffer.
     * This is important to know to ensure that data is read from the correct place in the buffer.
     *
     * @param buff Pointer to the `ibuffer_t` interface struct.
     *
     * @return The current read position in the buffer.
     */
    int (*BufferTELL)(ibuffer_t* buff);

    /**
     * @brief Obtains the actual memory pointer to the data of a buffer.
     *
     * This function retrieves a pointer to the actual data stored in a buffer,
     * without copying the data. This is useful for efficiently accessing the data
     * without the overhead of copying it to a new location.
     *
     * @param buff Pointer to the `ibuffer_t` interface struct.
     *
     * @return A pointer to the actual memory location of the buffer's data.
     *
     * @note Manipulating data directly through this pointer will affect the
     *       actual data in the buffer.
     */
    unsigned char* (*BufferGet)(ibuffer_t* buff);

    const char* (*FilePrePend)(void);

    // ########################################################################
    // STRUCT MANIPULATION (PART 2)
    // ########################################################################

    /**
     * @brief Adds a 32-bit integer value to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _value The 32-bit integer value to add to the structure.
     *
     * Usage example:
     *
     *		StructAddInt32(pStruct, "key", 42);
        */
    void (*StructAddInt32)(rvalue_t* _pStruct, const char* _pKey, int32_t _value);

    /**
     * @brief Adds a 64-bit integer value to the specified structure with the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be added.
     * @param _value The 64-bit integer value to add to the structure.
     *
     * Usage example:
     *
     *		StructAddInt64(pStruct, "key", 42);
        */
    void (*StructAddInt64)(rvalue_t* _pStruct, const char* _pKey, int64_t _value);

    /**
     * @brief Retrieves a member rvalue_t from the specified structure using the associated key.
     *
     * @param _pStruct A pointer to the structure.
     * @param _pKey The key associated with the value to be retrieved.
     *
     * @return A pointer to the rvalue_t associated with the specified key in the structure.
     *
     * Usage example:
     *
     *		rvalue_t* value = StructGetMember(pStruct, "key");
        *
        * @note This function returns a pointer to an rvalue_t and should be used with care.
        *       The user should ensure the returned rvalue_t is correctly interpreted and handled.
        */
    rvalue_t* (*StructGetMember)(rvalue_t* _pStruct, const char* _pKey);

    /**
     * @brief Query the keys in a struct.
     *
     * @param _pStruct  Pointer to a VALUE_OBJECT rvalue_t.
     * @param _keys     Pointer to an array of const char* pointers to receive the names.
     * @param _count    Length of _keys (in elements) on input, number filled on output.
     *
     * @return Total number of keys in the struct.
     *
     * NOTE: The strings in _keys are owned by the runner. You do not need to free them, however
     * you should make a copy if you intend to keep them around as the runner may invalidate them
     * in the future when performing variable modifications.
     *
     * Usage example:
     *
     *    // Get total number of keys in struct
     *    int num_keys = YYRunnerInterface_p->StructGetKeys(struct_rvalue_t, NULL, NULL);
     *
     *    // Fetch keys from struct
     *    std::vector<const char*> keys(num_keys);
     *    YYRunnerInterface_p->StructGetKeys(struct_rvalue_t, keys.data(), &num_keys);
     *
     *    // Loop over struct members
     *    for(int i = 0; i < num_keys; ++i)
     *    {
     *        rvalue_t *member = YYRunnerInterface_p->StructGetMember(struct_rvalue_t, keys[i]);
     *        ...
     *    }
     */
    int (*StructGetKeys)(rvalue_t* _pStruct, const char** _keys, int* _count);

    /**
     * @brief Parses and retrieves a structure as an rvalue_t from the argument array at the specified index.
     *
     * @param _pBase A pointer to the array of arguments.
     * @param _index The index in the array from which to retrieve the structure.
     *
     * @return A pointer to the rvalue_t representing the structure present at the specified index in the argument array.
     *
     * @note Care should be taken to ensure the retrieved rvalue_t is indeed representing a structure, and appropriate
     *       error handling should be implemented for cases where it might not be.
     *
     * Usage example:
     *
     *		rvalue_t* value = YYGetStruct(pArgs, 3);
        */
    rvalue_t* (*YYGetStruct)(rvalue_t* _pBase, int _index);

    // ########################################################################
    // EXTENSION OPTIONS
    // ########################################################################

    /**
     * @brief Retrieves the value of a specified extension option as an rvalue_t.
     *
     * @param result An rvalue_t reference where the result will be stored.
     * @param _ext The asset name of the extension whose option value needs to be retrieved.
     * @param _opt The key associated with the extension option.
     *
     * Usage example:
     *
     *		extOptGetrvalue_t(result, "MyExtension", "OptionKey");
        *
        * @note This function can be used to retrieve any type of value set by the extension user,
        *       and developers should ensure the returned rvalue_t is interpreted and handled correctly.
        */
    void (*extOptGetrvalue_t)(rvalue_t& result, const char* _ext, const char* _opt);

    /**
     * @brief Retrieves the value of a specified extension option as a string.
     *
     * @param _ext The asset name of the extension whose option value needs to be retrieved.
     * @param _opt The key associated with the extension option.
     *
     * @return The string value of the specified extension option.
     *
     * Usage example:
     *
     *		const char* value = extOptGetString("MyExtension", "OptionKey");
        */
    const char* (*extOptGetString)(const char* _ext, const char* _opt);

    /**
     * @brief Retrieves the value of a specified extension option as a double.
     *
     * @param _ext The asset name of the extension whose option value needs to be retrieved.
     * @param _opt The key associated with the extension option.
     *
     * @return The double value of the specified extension option.
     *
     * Usage example:
     *
     *		double value = extOptGetReal("MyExtension", "OptionKey");
        */
    double (*extOptGetReal)(const char* _ext, const char* _opt);

    // ########################################################################
    // UTILITIES (PART 2)
    // ########################################################################

    /**
     * @brief Determines whether the current game is being run from within the IDE.
     *
     * @return A boolean value representing whether the game is running from within the IDE.
     *         Returns true if it is running from within the IDE, otherwise false.
     *
     * Usage example:
     *
     *		bool runningFromIDE = isRunningFromIDE();
        *
        * @note This function is particularly useful for implementing security checks, allowing
        *       developers to conditionally enable or disable features based on the running environment
        *       of the game. Developers should use this function judiciously to ensure the security and
        *       integrity of the game.
        */
    bool (*isRunningFromIDE)();

    /**
     * @brief Retrieves the length of a specified YYArray.
     *
     * @param prvalue_t A pointer to the rvalue_t representing the YYArray.
     *
     * @return The length of the specified YYArray.
     *
     * Usage example:
     *
     *		int length = YYArrayGetLength(prvalue_t);
        *
        * @note Before using this function, users should ensure that the provided rvalue_t is indeed
        *       of type VALUE_ARRAY (prvalue_t->kind == VALUE_ARRAY) to avoid undefined behavior.
        *       Failing to confirm the type of rvalue_t can lead to runtime errors or unexpected outcomes.
        */
    int (*YYArrayGetLength)(rvalue_t* prvalue_t);

    // ########################################################################
    // EXTENSIONS
    // ########################################################################

    /**
     * @brief Retrieves the version of a specified extension in a "X.Y.Z" format.
     *
     * @param _ext The asset name of the extension whose version needs to be retrieved.
     *
     * @return The version of the specified extension as a string in "X.Y.Z" format.
     *
     * Usage example:
     *
     *		const char* version = extGetVersion("MyExtension");
        *
        * @note The returned string represents the version of the extension and can be used
        *       to perform version checks or logging. Developers should be aware of the
        *       format in which the version is returned and handle it appropriately.
        */
    const char* (*extGetVersion)(const char* _ext);

} yyrunner_interface_t;

void create_yyrunner_interface(yyrunner_interface_t* self) 
{
    memset(self, 0, sizeof(*self));
}
#endif  /* !RUNNER_INTERFACE_H_ */