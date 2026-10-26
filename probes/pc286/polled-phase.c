/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Compilation witness only. The decoder's flat-span premise is not a
 * segmented-pointer/far-memory backend contract. Never linked or executed. */
#include "../../arch/i286/c16.h"
#include "../../platform/mac128k/polled-phase.c"
typedef char cs_polled_state_layout[(sizeof(cs_mac_polled_phase)==40)?1:-1];
typedef char cs_polled_sample_layout[(sizeof(cs_pointer_sample)==12)?1:-1];
#ifdef CS_C16_FALSE_ASSERT
typedef char cs_probe_expected_refusal[-1];
#endif
