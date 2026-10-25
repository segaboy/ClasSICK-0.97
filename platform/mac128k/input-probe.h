/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_INPUT_PROBE_H
#define CS_MAC_INPUT_PROBE_H
#include <stdint.h>

#ifndef CLASSICK_MAC_FIRST_INPUT_PROBE
#define CLASSICK_MAC_FIRST_INPUT_PROBE 0
#endif
#if CLASSICK_MAC_FIRST_INPUT_PROBE < 0 || CLASSICK_MAC_FIRST_INPUT_PROBE > 1
#error "CLASSICK_MAC_FIRST_INPUT_PROBE must be 0 or 1"
#endif

/* Service-count fallback, not elapsed-time units. */
#define CS_MAC_FIRST_INPUT_LIMIT 5000u

static inline int cs_mac_first_input_ready(uint32_t services,uint32_t losses,
    uint32_t deliveries)
{
    return losses!=0u || deliveries!=0u || services>=CS_MAC_FIRST_INPUT_LIMIT;
}
#endif
