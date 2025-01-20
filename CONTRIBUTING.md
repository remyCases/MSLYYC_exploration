# Contributing to MSLYYC_exploration

Thank you for your interest in contributing to this project! This document provides guidelines and information for contributors.

## Project Overview

This project is a port of [Aurie](https://github.com/AurieFramework/Aurie) and [YYTK](https://github.com/AurieFramework/YYToolkit) from C++ to C99. While it's currently maintained by a single developer, contributions in the form of feedback, code reviews, and pull requests are welcome to help improve the codebase and design decisions.

## Getting Started

### Prerequisites

The following tools are required to build the project:

- MSYS2 GCC 13.2.0
- MSYS2 G++ 13.2.0
- CMake 3.26.4
- Make 4.4.1
- Ninja 1.11.1

### Clone and Build

1. Clone the repository:
```bash
git clone git@github.com:remyCases/MSLYYC_exploration.git
cd MSLYYC_exploration
```

2. Initialize submodules:
```bash
git submodule update --init --recursive
```

3. Build using make:
```bash
make build_release
```

4. For testing, you can run with a specific file:
```bash
make run VAR=path/to/file
```

## Code Style Guidelines

### File Organization

Header files must declare types in the following order at the start of the file:
1. Typedefs for basic types
2. Enums
3. Unions
4. Structs
5. Function pointers

All type declarations should be complete in the header declaration section. Do not use typedef during type definitions later in the file.

Example:
```c
// At the start of the header file
// declaration of basic types
typedef const char* str;

// declaration of enums
typedef enum OBJECT_TYPE OBJECT_TYPE;

// declaration of unions
typedef union xmm_register xmm_register;

// declaration of structs
typedef struct interface_s interface_t;

// declaration of function pointers
typedef int(*Entry)(module_t*,const char*);

// Then the definitions
struct interface_base_s
{
    // members
};
```

### Naming Conventions

1. Enums:
   - Names must be in UPPERCASE
   - Example: `enum OBJECT_TYPE { ... };`

2. Structs:
   - Struct definitions must end with `_s`
   - Typedef names must end with `_t`
   - Example:
     ```c
     // declaration
     typedef struct interface_s interface_t;
     // definition
     struct interface_s 
     {
         // members
     };
     ```

### Function Guidelines

1. Return Values:
   - All functions must return an `int` as an error code
   - Output parameters should be passed as pointers at the end of the argument list
   - Example:
     ```c
     int md_is_image_runtime_loaded(module_t* module, bool* runtime_loaded)
     {
        int last_status = MSL_SUCCESS;
        /* Logic */
        return last_status;
     }
     ```

2. Error Handling:
   - All custom function calls must use the CHECK macro family (`CHECK_CALL`, `CHECK_CALL_CUSTOM_ERROR` or `CHECK_CALL_GOTO_ERROR`)
   - Windows API and libc functions are exempt from this requirement
   - Example:
     ```c
     // Correct usage with custom functions
     CHECK_CALL(mdp_mark_module_for_purge, module);
     // Inorrect usage with custom functions
     mdp_mark_module_for_purge(module);
     
     // Direct calls for Win API and libc
     GetModuleFileName(NULL, buffer, MAX_PATH);
     malloc(size);
     ```

## Pull Request Guidelines

1. All pull requests must:
   - Be free of compilation errors (linker errors are currently acceptable)
   - Be as warning-free as possible
   - Follow the code style guidelines above
   - Include appropriate documentation for new features
   - Include test cases when applicable

2. PR Process:
   - Fork the repository
   - Create a feature branch
   - Make your changes
   - Submit a pull request with a clear description of changes
   - Respond to review comments if any

## Types of Contributions

Currently the main part of the project is to rewrite YYTK and Aurie in plain C99. we expect contributions to be focused on the port.

However, we welcome all kinds of contributions through pull requests, including:
- Bug fixes
- Feature implementations
- Documentation improvements
- Code refactoring
- Performance optimizations
- Test coverage improvements

## Questions and Feedback

If you have questions about the codebase or want to discuss design decisions, please:
1. Open an issue with the "question" or "discussion" label
2. Be specific about which part of the code you're referring to
3. Explain your concerns or alternative approaches

Your feedback helps improve the project for everyone!