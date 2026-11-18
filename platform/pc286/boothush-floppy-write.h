/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_BOOTHUSH_FLOPPY_WRITE_H
#define CLASSICK_PC286_BOOTHUSH_FLOPPY_WRITE_H
#include "boothush-floppy-read.h"
typedef struct {
    uint16_t first_sector,blocks,data_segment;
    uint8_t bounce[CS_BLOCK_BYTES];
} cs_pc286_floppy_writer;
typedef enum {
    CS_PC286_WRITE_OK=0,CS_PC286_WRITE_ARGUMENT=1,CS_PC286_WRITE_RANGE=2,
    CS_PC286_WRITE_STATE=3,CS_PC286_WRITE_PROTECTED=4,CS_PC286_WRITE_IO=5
} cs_pc286_write_result;
/* SPEC-0126: caller-owned stable disjoint small-model objects in live RAM.
 * Failure preserves writer; success changes only its three control fields.
 * Does not access media or enable writes. Numeric admission is not ownership. */
int cs_pc286_floppy_write_bind(cs_pc286_floppy_writer *writer,
    uint32_t first_sector,uint32_t blocks);
/* Exactly one sector, no retries. Source is512 live bytes and never changes.
 * Pre-I/O refusal preserves bounce; BIOS refusal may change bounce but the
 * admitted model preserves media. Successful calls commit independently.
 * Arbitrary source/writer/physical aliases are excluded by caller contract. */
cs_pc286_write_result cs_pc286_floppy_write_one(cs_pc286_floppy_writer *writer,
    uint32_t lba,const uint8_t *source);
#endif
