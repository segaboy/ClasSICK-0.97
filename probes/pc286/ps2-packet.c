/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Compiler witness only;no controller/backend,link or target execution. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc/ps2-mouse.c"
typedef char cs_packet_state_layout[(sizeof(cs_ps2_mouse)==12)?1:-1];
typedef char cs_packet_output_layout[(sizeof(cs_ps2_mouse_packet)==
    (CLASSICK_PC_RAW_PACKET_PROBE?24:12))?1:-1];
#ifdef CS_C16_FALSE_ASSERT
typedef char cs_probe_expected_refusal[-1];
#endif
