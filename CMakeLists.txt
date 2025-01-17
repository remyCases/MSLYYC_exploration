cmake_minimum_required(VERSION "3.15")

project(MSL_YYC)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Generator is set to ${CMAKE_GENERATOR}")

enable_language(C CXX)

set(CMAKE_C_STANDARD "99")
set(CMAKE_CXX_STANDARD "23")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM MATCHES Windows)
	message(STATUS "Target system is windows")
else(CMAKE_SYSTEM MATCHES Linux)
	message(STATUS "Target system is linux")
endif()

if(MSVC)
    # MSVC-specific flags
    add_compile_options(
        /W4     			# Warning level 4
        /MP     			# Multi-processor compilation
        /Zi     			# Debug information format
    )
else()
    # GCC/Clang flags
    add_compile_options(
        -Wall
        -Wextra
		-O2                 # Optimization level 2 (good balance of speed vs size)
        -pthread    		# Threading support
		-fstack-protector   # Stack protection against buffer overflows
    )
    
    # Optional: Specific GCC vs Clang flags
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        add_compile_options(/Od /RTC1)  # Disable optimization, enable runtime checks
    else()
        add_compile_options(-O0 -g3)    # No optimization, max debug info
    endif()
endif()

add_library(safetyhookwrapper
    "safety_hook_wrapper/source/wrapper.cpp"
    "safety_hook_wrapper/source/wrapper_c.cpp"
)

target_include_directories(safetyhookwrapper 
    PUBLIC "safety_hook_wrapper/include"
    PRIVATE "safety_hook_wrapper/source" "deps/safetyhook/include")

# Register Zydis dependency.
# Disable build of tools and examples.
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
add_subdirectory("deps/zydis")
add_subdirectory("deps/safetyhook")

file(GLOB all_SRCS
    "main.c"
	"include/*.h"
    "source/*.c"
)

message(STATUS "Files are ${all_SRCS}")
add_executable(msl_yyc ${all_SRCS})
target_link_libraries(msl_yyc PRIVATE "Zydis")
target_link_libraries(safetyhookwrapper PRIVATE safetyhook)
target_link_libraries(msl_yyc PRIVATE safetyhookwrapper)