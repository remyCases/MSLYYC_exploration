# Copyright (C) 2025 Rémy Cases
# See LICENSE file for extended copyright information.
# This file is part of adventOfCode project from https://github.com/remyCases/MSLYYC_exploration.

# Convenient values
empty:=
space:= $(empty) $(empty)
comma:= ,

BINS := $(wildcard build/*.exe)
$(info $(BINS))

# Detect OS
ifeq ($(OS), Windows_NT)
    DETECTED_OS := Windows
else
    DETECTED_OS := $(shell uname -s)
endif

# Default values
BUILD_TYPE ?= Release

# C99 specific default values
COMPILER ?= gcc
VERBOSE ?= OFF
CMAKE_GENERATOR ?= "Ninja"

# Cross compatibilty Linux-Windows
ifeq ($(DETECTED_OS), Linux)
	RM = rm -rf
	RMBINS = $(BINS)
	MD = mkdir -p
	CAT = cat
	SANITIZE ?= OFF
else
	RM = powershell Remove-Item -Recurse -Path
	RMBINS = $(subst $(space),$(comma),$(BINS))
	MD = powershell New-Item -Type Directory -Force
	CAT = powershell Get-Content -encoding UTF8
endif

.DELETE_ON_ERROR:
prerequisite:
	$(MD) build/
build: prerequisite main

# Base build command
CMAKE_BASE_CMD = cmake -Bbuild -G $(CMAKE_GENERATOR) \
    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
    -DCMAKE_CXX_COMPILER=$(COMPILER) \
    -DCMAKE_VERBOSE_MAKEFILE=$(VERBOSE)

# OS-specific build commands
ifeq ($(DETECTED_OS),Windows)
    CMAKE_CMD = $(CMAKE_BASE_CMD)
else
    CMAKE_CMD = $(CMAKE_BASE_CMD) -DENABLE_SANITIZER=$(SANITIZE)
endif

main:
	$(CMAKE_CMD)
	cd build && ninja

# Common targets for both OS
.PHONY: build_debug build_release build_verbose
build_debug:
	$(MAKE) build BUILD_TYPE=Debug

build_release:
	$(MAKE) build BUILD_TYPE=Release

build_verbose:
	$(MAKE) build VERBOSE=ON

# Unix-only targets
ifneq ($(DETECTED_OS),Windows)
.PHONY: build_asan build_msan
build_asan:
	$(MAKE) build BUILD_TYPE=Debug SANITIZE=address

build_msan:
	$(MAKE) build BUILD_TYPE=Debug SANITIZE=memory
endif

debug: clean build_debug
	gdb -ex 'break main' -ex 'run -c' .\build\msl_yyc.exe

run: clean build_release
	.\build\msl_yyc.exe

### CLEAN ###
clean:
	-$(RM) $(RMBINS)
