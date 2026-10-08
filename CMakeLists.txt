# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
cmake_minimum_required(VERSION 3.25)
project(ClasSICK VERSION 0.0.1 LANGUAGES C)

# Portable core plus hosted conformance tools. No bootable OS target yet.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
include(CTest)
option(CLASSICK_WINDOWS_ADAPTERS "Build Windows development adapters and viewer" ON)

add_library(classick_abi_probe OBJECT tools/toolchain-smoke/abi-probe.c)
add_executable(classick_toolchain_smoke tools/toolchain-smoke/main.c)
add_library(classick_surfaces STATIC core/graphics/surface.c)
target_include_directories(classick_surfaces PUBLIC core/graphics)
# Uninstrumented optimized object checks compiler helper/import boundaries.
add_library(classick_surfaces_freestanding OBJECT core/graphics/surface.c)
add_executable(classick_surface_tests tests/surfaces.c)
target_link_libraries(classick_surface_tests PRIVATE classick_surfaces)
add_library(classick_arenas STATIC core/memory/arena.c)
target_include_directories(classick_arenas PUBLIC core/memory)
add_library(classick_arenas_freestanding OBJECT core/memory/arena.c)
add_executable(classick_arena_tests tests/arenas.c)
target_link_libraries(classick_arena_tests PRIVATE classick_arenas)
add_library(classick_input STATIC core/input/input.c)
target_include_directories(classick_input PUBLIC core/input)
add_library(classick_input_freestanding OBJECT core/input/input.c)
add_executable(classick_input_tests tests/input.c)
target_link_libraries(classick_input_tests PRIVATE classick_input)
add_library(classick_clocks STATIC core/time/clock.c)
target_include_directories(classick_clocks PUBLIC core/time)
add_library(classick_clocks_freestanding OBJECT core/time/clock.c)
add_executable(classick_clock_tests tests/clocks.c)
target_link_libraries(classick_clock_tests PRIVATE classick_clocks)
add_executable(classick_core_link_tests tests/core-link.c tools/core-link/probe.c)
target_link_libraries(classick_core_link_tests PRIVATE classick_surfaces classick_arenas classick_input classick_clocks)
if(WIN32 AND CLASSICK_WINDOWS_ADAPTERS)
    add_library(classick_windows_clock STATIC platform/windows/clock.c)
    target_link_libraries(classick_windows_clock PUBLIC classick_clocks)
    add_executable(classick_windows_clock_tests tests/windows-clock.c)
    target_link_libraries(classick_windows_clock_tests PRIVATE classick_windows_clock)
    add_library(classick_windows_input STATIC platform/windows/input.c)
    target_link_libraries(classick_windows_input PUBLIC classick_input)
    add_executable(classick_windows_input_tests tests/windows-input.c)
    target_link_libraries(classick_windows_input_tests PRIVATE classick_windows_input)
    add_library(classick_windows_presenter STATIC platform/windows/pixels.c
        platform/windows/presenter.c)
    target_include_directories(classick_windows_presenter PUBLIC platform/windows)
    target_link_libraries(classick_windows_presenter PUBLIC classick_surfaces gdi32 user32)
    add_executable(classick_presentation_tests tests/windows-presentation.c)
    target_link_libraries(classick_presentation_tests PRIVATE classick_windows_presenter)
    add_executable(classick_surface_demo WIN32 apps/surface-demo/windows-main.c)
    target_link_libraries(classick_surface_demo PRIVATE classick_windows_presenter classick_arenas classick_windows_input classick_windows_clock)
    target_compile_definitions(classick_surface_demo PRIVATE WINVER=0x0A00 _WIN32_WINNT=0x0A00)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        foreach(target classick_windows_presenter classick_presentation_tests classick_surface_demo
                classick_windows_input classick_windows_input_tests classick_windows_clock classick_windows_clock_tests)
            target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror
                -Wconversion -Wsign-conversion
                "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
                "-ffile-prefix-map=${CMAKE_BINARY_DIR}=.")
        endforeach()
    elseif(MSVC)
        foreach(target classick_windows_presenter classick_presentation_tests classick_surface_demo
                classick_windows_input classick_windows_input_tests classick_windows_clock classick_windows_clock_tests)
            target_compile_options(${target} PRIVATE /W4 /WX)
        endforeach()
    endif()
    if(MINGW)
        foreach(target classick_presentation_tests classick_surface_demo classick_windows_input_tests classick_windows_clock_tests)
            target_link_options(${target} PRIVATE -Wl,--no-insert-timestamp)
        endforeach()
    endif()
endif()
option(CLASSICK_SURFACE_SANITIZERS "Validated Clang ASan/UBSan conformance build" OFF)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    foreach(target classick_abi_probe classick_toolchain_smoke classick_surfaces
            classick_surfaces_freestanding classick_surface_tests
            classick_arenas classick_arenas_freestanding classick_arena_tests
            classick_input classick_input_freestanding classick_input_tests classick_clocks classick_clocks_freestanding classick_clock_tests classick_core_link_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror
            "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
            # Last matching mapping wins: preserve the narrower nested build map.
            "-ffile-prefix-map=${CMAKE_BINARY_DIR}=.")
    endforeach()
    target_compile_options(classick_abi_probe PRIVATE -ffreestanding)
    foreach(target classick_surfaces classick_surfaces_freestanding
            classick_arenas classick_arenas_freestanding classick_input classick_input_freestanding classick_clocks classick_clocks_freestanding)
        target_compile_options(${target} PRIVATE -ffreestanding -fno-builtin
            -fno-stack-protector -Wconversion -Wsign-conversion)
    endforeach()
    target_compile_options(classick_surfaces_freestanding PRIVATE -O2)
    target_compile_options(classick_arenas_freestanding PRIVATE -O2)
    target_compile_options(classick_input_freestanding PRIVATE -O2)
    target_compile_options(classick_clocks_freestanding PRIVATE -O2)
    target_compile_options(classick_surface_tests PRIVATE -Wconversion -Wsign-conversion)
    target_compile_options(classick_arena_tests PRIVATE -Wconversion -Wsign-conversion)
    target_compile_options(classick_input_tests PRIVATE -Wconversion -Wsign-conversion)
    target_compile_options(classick_clock_tests PRIVATE -Wconversion -Wsign-conversion)
    target_compile_options(classick_core_link_tests PRIVATE -Wconversion -Wsign-conversion)
elseif(MSVC)
    foreach(target classick_abi_probe classick_toolchain_smoke classick_surfaces
            classick_surfaces_freestanding classick_surface_tests
            classick_arenas classick_arenas_freestanding classick_arena_tests
            classick_input classick_input_freestanding classick_input_tests classick_clocks classick_clocks_freestanding classick_clock_tests classick_core_link_tests)
        target_compile_options(${target} PRIVATE /W4 /WX)
    endforeach()
endif()
if(CLASSICK_SURFACE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Validate this compiler's sanitizer support before use")
    endif()
    foreach(target classick_surfaces classick_surface_tests classick_arenas classick_arena_tests
            classick_input classick_input_tests classick_clocks classick_clock_tests classick_core_link_tests)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined
            -fno-sanitize-recover=all -fno-omit-frame-pointer)
    endforeach()
    target_link_options(classick_surface_tests PRIVATE -fsanitize=address,undefined)
    target_link_options(classick_arena_tests PRIVATE -fsanitize=address,undefined)
    target_link_options(classick_input_tests PRIVATE -fsanitize=address,undefined)
    target_link_options(classick_clock_tests PRIVATE -fsanitize=address,undefined)
    target_link_options(classick_core_link_tests PRIVATE -fsanitize=address,undefined)
    if(WIN32 AND CLASSICK_WINDOWS_ADAPTERS)
        foreach(target classick_windows_presenter classick_presentation_tests classick_surface_demo
                classick_windows_input classick_windows_input_tests classick_windows_clock classick_windows_clock_tests)
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined
                -fno-sanitize-recover=all -fno-omit-frame-pointer)
        endforeach()
        foreach(target classick_presentation_tests classick_surface_demo classick_windows_input_tests classick_windows_clock_tests)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        endforeach()
    endif()
endif()
if(MINGW)
    target_link_options(classick_toolchain_smoke PRIVATE -Wl,--no-insert-timestamp)
    target_link_options(classick_surface_tests PRIVATE -Wl,--no-insert-timestamp)
    target_link_options(classick_arena_tests PRIVATE -Wl,--no-insert-timestamp)
    target_link_options(classick_input_tests PRIVATE -Wl,--no-insert-timestamp)
    target_link_options(classick_clock_tests PRIVATE -Wl,--no-insert-timestamp)
    target_link_options(classick_core_link_tests PRIVATE -Wl,--no-insert-timestamp)
endif()
if(BUILD_TESTING)
    add_test(NAME freestanding.probe COMMAND classick_core_link_tests)
    if(WIN32 AND CLASSICK_WINDOWS_ADAPTERS)
        foreach(suite conversion bounds gdi)
            add_test(NAME presentation.${suite} COMMAND classick_presentation_tests ${suite})
        endforeach()
        add_test(NAME presentation.window COMMAND classick_surface_demo --verify)
        set_tests_properties(presentation.window PROPERTIES TIMEOUT 15)
        add_test(NAME arenas.viewer-memory COMMAND classick_surface_demo --verify-memory)
        set_tests_properties(arenas.viewer-memory PROPERTIES TIMEOUT 15)
        foreach(suite conversion transitions live)
            add_test(NAME clocks.windows-${suite} COMMAND classick_windows_clock_tests ${suite})
        endforeach()
        add_test(NAME clocks.viewer COMMAND classick_surface_demo --verify-clock)
        set_tests_properties(clocks.viewer PROPERTIES TIMEOUT 15)
        add_test(NAME clocks.viewer-live COMMAND classick_surface_demo --verify-clock-live)
        set_tests_properties(clocks.viewer-live PROPERTIES TIMEOUT 15)
        add_test(NAME hosted.start COMMAND classick_surface_demo --verify-start)
        set_tests_properties(hosted.start PROPERTIES TIMEOUT 15
            PASS_REGULAR_EXPRESSION "B1 queued-message integration: PASS")
        add_test(NAME hosted.incomplete COMMAND ${CMAKE_COMMAND}
            "-DVIEWER=$<TARGET_FILE:classick_surface_demo>"
            -P ${CMAKE_SOURCE_DIR}/scripts/Check-Hosted-EarlyExit.cmake)
        set_tests_properties(hosted.incomplete PROPERTIES TIMEOUT 15)
        add_test(NAME input.windows-mapping COMMAND classick_windows_input_tests)
        add_test(NAME input.viewer COMMAND classick_surface_demo --verify-input)
        set_tests_properties(input.viewer PROPERTIES TIMEOUT 15)
        add_test(NAME input.viewer-memory COMMAND classick_surface_demo --verify-input-memory)
        set_tests_properties(input.viewer-memory PROPERTIES TIMEOUT 15)
    endif()
    add_test(NAME bootstrap.hosted-smoke COMMAND classick_toolchain_smoke)
    set_tests_properties(bootstrap.hosted-smoke PROPERTIES
        PASS_REGULAR_EXPRESSION "ClasSICK 0.97 toolchain smoke: PASS")
    foreach(suite layout errors matrix independent)
        add_test(NAME surfaces.${suite} COMMAND classick_surface_tests ${suite})
    endforeach()
    foreach(suite layout errors matrix ownership)
        add_test(NAME arenas.${suite} COMMAND classick_arena_tests ${suite})
    endforeach()
    foreach(suite arithmetic errors matrix deterministic)
        add_test(NAME clocks.${suite} COMMAND classick_clock_tests ${suite})
    endforeach()
    foreach(suite ordering errors traces independent)
        add_test(NAME input.${suite} COMMAND classick_input_tests ${suite})
    endforeach()
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        find_program(CLASSICK_NM NAMES llvm-nm nm REQUIRED)
        add_test(NAME surfaces.freestanding COMMAND ${CMAKE_COMMAND}
            "-DNM=${CLASSICK_NM}"
            "-DOBJECT=$<TARGET_OBJECTS:classick_surfaces_freestanding>"
            -P ${CMAKE_SOURCE_DIR}/scripts/Check-Freestanding.cmake)
        add_test(NAME arenas.freestanding COMMAND ${CMAKE_COMMAND}
            "-DNM=${CLASSICK_NM}"
            "-DOBJECT=$<TARGET_OBJECTS:classick_arenas_freestanding>"
            -P ${CMAKE_SOURCE_DIR}/scripts/Check-Freestanding.cmake)
        add_test(NAME clocks.freestanding COMMAND ${CMAKE_COMMAND}
            "-DNM=${CLASSICK_NM}"
            "-DOBJECT=$<TARGET_OBJECTS:classick_clocks_freestanding>"
            -P ${CMAKE_SOURCE_DIR}/scripts/Check-Freestanding.cmake)
        add_test(NAME input.freestanding COMMAND ${CMAKE_COMMAND}
            "-DNM=${CLASSICK_NM}"
            "-DOBJECT=$<TARGET_OBJECTS:classick_input_freestanding>"
            -P ${CMAKE_SOURCE_DIR}/scripts/Check-Freestanding.cmake)
    endif()
endif()
