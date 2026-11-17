/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_BOOTHUSH_FLOPPY_READ_H
#define CLASSICK_PC286_BOOTHUSH_FLOPPY_READ_H

#include "../../core/storage/block-read.h"

#define CS_PC286_FLOPPY_BLOCKS 2400u

/* SPEC-0113: caller-owned near native data; controls and buffer remain stable
 * and disjoint from the block device, output and staging. Built-in BootHush
 * firmware, drive zero, fixed original PC 1.2MB floppy only. No physical claim. */
typedef struct {
    uint16_t first_sector;
    uint16_t blocks;
    uint16_t data_segment;
    uint8_t bounce[CS_BLOCK_BYTES];
} cs_pc286_floppy;

/* Numeric admission only; creates/dereferences no pointer and proves no native
 * object ownership. Exposed for the diagnostic's exact boundary checks. */
int cs_pc286_floppy_buffer_fits(uint16_t segment, uint16_t offset);

/* Success publishes context controls and device; failure changes neither.
 * Does not probe media. Caller supplies live, distinct small-model objects. */
int cs_pc286_floppy_bind(cs_pc286_floppy *provider, uint32_t first_sector,
    uint32_t blocks, cs_block_device *device);

cs_block_io_result cs_pc286_floppy_read_one(void *context, uint32_t lba,
    uint8_t *block);

#endif
