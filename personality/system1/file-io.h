/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_S1_FILE_IO_H
#define CLASSICK_S1_FILE_IO_H
#include "../../core/memory/guest.h"
#include "../../core/services/traps.h"
#include "../../core/storage/mfs-write.h"

typedef struct {
    const cs_mfsv_volume *volume;
    const cs_block_writer *writer;  /* NULL opens a read-only access path. */
    cs_mfs_entry entry;
    cs_mfsv_fork fork;
    uint16_t reference;
    uint32_t mark;
} cs_s1_open_fork;

cs_mfsv_result cs_s1_open_named_fork(cs_s1_open_fork *out, const cs_mfsv_volume *volume,
    cs_mfsv_workspace *work, const uint8_t *name, size_t length, cs_mfsv_fork fork,
    uint16_t reference, const cs_block_writer *writer);

/* SPEC-0136. Return 1 when a Macintosh result was published, 0 on native
 * refusal. The caller supplies a valid bound open fork and eligible stable
 * guest mapping. The staging area is used only for reads. */
int cs_s1_file_io(cs_s1_open_fork *file, cs_mfsv_workspace *work,
    const cs_guest_map *map, cs_trap_frame *frame, uint16_t word,
    uint8_t *staging, size_t capacity);
/* Narrow 68000 instruction step: fetch the actual opcode at PC, dispatch
 * A002/A003 to the native service and advance PC only after publication. */
int cs_s1_file_step(cs_s1_open_fork *file, cs_mfsv_workspace *work,
    const cs_guest_map *map, cs_trap_frame *frame, uint8_t *staging, size_t capacity);
#endif
