/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC_BOOT_HEADER_PROBE_H
#define CLASSICK_MAC_BOOT_HEADER_PROBE_H
#include "../../core/storage/block-read.h"
extern volatile uint32_t cs_boot_header_report[16];
unsigned cs_boot_header_probe(const cs_block_device *device);
#endif
