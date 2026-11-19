/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC_STARTUP_FLOPPY_H
#define CLASSICK_MAC_STARTUP_FLOPPY_H
#include "iwm-read.h"

typedef struct {
    cs_mac_iwm drive;
    uint32_t checksum_retries;
} cs_mac_startup_floppy;

/* The startup reader retries a decoded checksum mismatch once per block.
 * completed reads count only successful publications in drive.reads.
 * checksum_retries counts additional attempts, including unsuccessful ones. */
int cs_mac_startup_floppy_bind(cs_mac_startup_floppy *reader, cs_block_device *device);
#endif
