# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# Freestanding object audit that permits only an exact comma-separated set of
# original project symbols as imports, and rejects mutable global/static data.
cmake_policy(SET CMP0057 NEW)
execute_process(COMMAND "${NM}" --undefined-only --format=posix "${OBJECT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE imports ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Cannot audit object: ${error}")
endif()
string(REPLACE "," ";" allowed "${ALLOWED}")
string(REPLACE "\n" ";" lines "${imports}")
foreach(line IN LISTS lines)
    if(line STREQUAL "")
        continue()
    endif()
    if(NOT line MATCHES "^([^ ]+) U( |$)")
        message(FATAL_ERROR "Unexpected symbol record: ${line}")
    endif()
    # Accept a 32-bit COFF leading underscore only as a spelling of an allowed name.
    set(name "${CMAKE_MATCH_1}")
    string(REGEX REPLACE "^_" "" bare "${name}")
    if(NOT name IN_LIST allowed AND NOT bare IN_LIST allowed)
        message(FATAL_ERROR "External helper or unexpected import: ${line}")
    endif()
endforeach()
execute_process(COMMAND "${NM}" --defined-only --format=posix "${OBJECT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR symbols MATCHES "(^|\n)[^\n ]+ [BbCcDdGgSsVv] ")
    message(FATAL_ERROR "Object defines mutable data or cannot be audited: ${symbols} ${error}")
endif()
message(STATUS "Object import audit PASS: only ${ALLOWED}; no mutable global data")
