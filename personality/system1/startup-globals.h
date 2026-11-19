/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_STARTUP_GLOBALS_H
#define CLASSICK_SYSTEM1_STARTUP_GLOBALS_H
#include <stdint.h>
#include <stddef.h>

#define CS_MAC128K_GLOBALS_BASE 0x0100u
#define CS_MAC128K_GLOBALS_BYTES 0x0728u

/* SPEC-0137. page100 maps guest $0100. Supply an exclusive contiguous RAM
 * window in one native segment. The four pointer values use guest byte order.
 * A null or short window is refused before the first write. */
int cs_mac128k_seed_startup_globals(volatile uint8_t *page100, size_t bytes);
#endif
