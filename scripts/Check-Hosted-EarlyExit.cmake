# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
execute_process(COMMAND "${VIEWER}" --verify-start-incomplete
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 10)
if(NOT result STREQUAL "1" OR NOT output MATCHES
        "B1 queued-message integration: INCOMPLETE/FAIL; presses=0 releases=0 escape=1 consumed=1 timer=[0-9]+ destroyed=1 retired=0")
    message(FATAL_ERROR "Early Escape must reject incomplete acceptance after orderly cleanup: ${result}\n${output}\n${error}")
endif()
message(STATUS "Early Escape exits cleanly but cannot pass hosted-start acceptance")
