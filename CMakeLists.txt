# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
cmake_minimum_required(VERSION 3.25)
project(ClasSICKBootstrap VERSION 0.0.0 LANGUAGES C)

# This is development infrastructure only. There is no OS target at bootstrap.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
include(CTest)

add_library(classick_abi_probe OBJECT tools/toolchain-smoke/abi-probe.c)
add_executable(classick_toolchain_smoke tools/toolchain-smoke/main.c)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    foreach(target classick_abi_probe classick_toolchain_smoke)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror
            "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
            # Last matching mapping wins: preserve the narrower nested build map.
            "-ffile-prefix-map=${CMAKE_BINARY_DIR}=.")
    endforeach()
    target_compile_options(classick_abi_probe PRIVATE -ffreestanding)
elseif(MSVC)
    target_compile_options(classick_abi_probe PRIVATE /W4 /WX)
    target_compile_options(classick_toolchain_smoke PRIVATE /W4 /WX)
endif()
if(MINGW)
    target_link_options(classick_toolchain_smoke PRIVATE -Wl,--no-insert-timestamp)
endif()
if(BUILD_TESTING)
    add_test(NAME bootstrap.hosted-smoke COMMAND classick_toolchain_smoke)
    set_tests_properties(bootstrap.hosted-smoke PROPERTIES
        PASS_REGULAR_EXPRESSION "ClasSICK 0.97 toolchain smoke: PASS")
endif()
