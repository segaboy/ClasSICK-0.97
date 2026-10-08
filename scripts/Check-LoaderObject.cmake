# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
execute_process(COMMAND "${NM}" --undefined-only --format=posix "${OBJECT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE imports ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Cannot audit loader object: ${error}")
endif()
string(REPLACE "\n" ";" lines "${imports}")
set(allowed cs_uefi_check_framebuffer cs_uefi_check_map cs_uefi_check_owned
    cs_uefi_exit_init cs_uefi_exit_snapshot cs_uefi_exit_observe)
foreach(line IN LISTS lines)
    if(line STREQUAL "")
        continue()
    endif()
    if(NOT line MATCHES "^([^ ]+) U( |$)")
        message(FATAL_ERROR "Unexpected loader symbol record: ${line}")
    endif()
    if(NOT CMAKE_MATCH_1 IN_LIST allowed)
        message(FATAL_ERROR "External loader helper: ${line}")
    endif()
endforeach()
execute_process(COMMAND "${NM}" --defined-only --format=posix "${OBJECT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR symbols MATCHES "(^|\n)[^\n ]+ [BbCcDdGgSsVv] ")
    message(FATAL_ERROR "Loader object defines mutable globals or cannot be audited: ${symbols} ${error}")
endif()
message(STATUS "Loader object PASS: only original preboot helpers, no mutable global data")
