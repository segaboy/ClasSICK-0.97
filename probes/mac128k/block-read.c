/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original RAM-provider diagnostic bound to the project's original Mac reset. */
#include <stdint.h>
#include "../storage/block-read.h"

volatile uint32_t cs_mac_diag[7];

void cs_mac_main(void)
{
    cs_mac_diag[0] = (uint32_t)cs_block_read_probe();
    for (;;) { }
}

/* Unexpected A-line entry is a failure, never a replacement trap service. */
void cs_mac_line_a_handler(void)
{
    cs_mac_diag[6] = UINT32_C(0xf002);
    for (;;) { }
}
