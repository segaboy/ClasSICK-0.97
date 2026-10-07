# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
cmake_minimum_required(VERSION 3.25)
project(ClasSICK VERSION 0.0.1 LANGUAGES C)

# Portable core plus hosted conformance tools. No bootable OS target yet.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
include(CTest)

add_library(classick_abi_probe OBJECT tools/toolchain-smoke/abi-probe.c)
add_executable(classick_toolchain_smoke tools/toolchain-smoke/main.c)
add_library(classick_surfaces STATIC core/graphics/surface.c)
target_include_directories(classick_surfaces PUBLIC core/graphics)
# Uninstrumented optimized object checks compiler helper/import boundaries.
add_library(classick_surfaces_freestanding OBJECT core/graphics/surface.c)
add_executable(classick_surface_tests tests/surfaces.c)
target_link_libraries(classick_surface_tests PRIVATE classick_surfaces)
option(CLASSICK_SURFACE_SANITIZERS "Validated Clang ASan/UBSan conformance build" OFF)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    foreach(target classick_abi_probe classick_toolchain_smoke classick_surfaces
            classick_surfaces_freestanding classick_surface_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror
            "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
            # Last matching mapping wins: preserve the narrower nested build map.
            "-ffile-prefix-map=${CMAKE_BINARY_DIR}=.")
    endforeach()
    target_compile_options(classick_abi_probe PRIVATE -ffreestanding)
    foreach(target classick_surfaces classick_surfaces_freestanding)
        target_compile_options(${target} PRIVATE -ffreestanding -fno-builtin
            -fno-stack-protector -Wconversion -Wsign-conversion)
    endforeach()
    target_compile_options(classick_surfaces_freestanding PRIVATE -O2)
    target_compile_options(classick_surface_tests PRIVATE -Wconversion -Wsign-conversion)
elseif(MSVC)
    foreach(target classick_abi_probe classick_toolchain_smoke classick_surfaces
            classick_surfaces_freestanding classick_surface_tests)
        target_compile_options(${target} PRIVATE /W4 /WX)
    endforeach()
endif()
if(CLASSICK_SURFACE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Validate this compiler's sanitizer support before use")
    endif()
    foreach(target classick_surfaces classick_surface_tests)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined
            -fno-sanitize-recover=all -fno-omit-frame-pointer)
    endforeach()
    target_link_options(classick_surface_tests PRIVATE -fsanitize=address,undefined)
endif()
if(MINGW)
    target_link_options(classick_toolchain_smoke PRIVATE -Wl,--no-insert-timestamp)
    target_link_options(classick_surface_tests PRIVATE -Wl,--no-insert-timestamp)
endif()
if(BUILD_TESTING)
    add_test(NAME bootstrap.hosted-smoke COMMAND classick_toolchain_smoke)
    set_tests_properties(bootstrap.hosted-smoke PROPERTIES
        PASS_REGULAR_EXPRESSION "ClasSICK 0.97 toolchain smoke: PASS")
    foreach(suite layout errors matrix independent)
        add_test(NAME surfaces.${suite} COMMAND classick_surface_tests ${suite})
    endforeach()
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        find_program(CLASSICK_NM NAMES llvm-nm nm REQUIRED)
        add_test(NAME surfaces.freestanding COMMAND ${CMAKE_COMMAND}
            "-DNM=${CLASSICK_NM}"
            "-DOBJECT=$<TARGET_OBJECTS:classick_surfaces_freestanding>"
            -P ${CMAKE_SOURCE_DIR}/scripts/Check-Freestanding.cmake)
    endif()
endif()
