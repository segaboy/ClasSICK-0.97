# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
execute_process(COMMAND "${NM}" --undefined-only "${OBJECT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE imports ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Cannot audit object: ${error}")
endif()
string(STRIP "${imports}" imports)
if(NOT imports STREQUAL "")
    message(FATAL_ERROR "Freestanding core imports detected: ${imports}")
endif()
execute_process(COMMAND "${NM}" --defined-only --format=posix "${OBJECT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Cannot audit data: ${error}")
endif()
if(symbols MATCHES "(^|\n)[^\n ]+ [BbCcDdGgSsVv] ")
    message(FATAL_ERROR "Core object defines global/static data: ${symbols}")
endif()
message(STATUS "Core freestanding audit PASS: no undefined symbols or global data")
