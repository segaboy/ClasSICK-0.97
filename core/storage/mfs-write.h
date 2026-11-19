/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_MFS_WRITE_H
#define CLASSICK_CORE_STORAGE_MFS_WRITE_H
#include "mfs-volume.h"
#include "block-write.h"

typedef enum { CS_MFSW_DONE=0,CS_MFSW_READ=1,CS_MFSW_WRITE=2 } cs_mfsw_phase;
typedef struct {
    uint32_t completed,lba;
    cs_mfsw_phase phase;
    uint8_t uncertain;
} cs_mfsw_progress;

/* SPEC-0128: bounded existing-fork overwrite, no allocation/metadata changes.
 * Use a valid mounted volume and decoded entry. The writer has EXACTLY the
 * mounted logical block count and addresses the same exclusively owned medium.
 * Its metadata/map and all native control objects remain stable. Only fork
 * data changes; no concurrent read, write, reentry or shared allocation owner.
 * All objects/spans are truthful/live, within one native segment, and mutually
 * disjoint from input/work/progress and provider backing. Read/write contexts
 * may deliberately share a device. Arbitrary overlap/far aliases are excluded,
 * not detected. The mount's read-only API contract is not otherwise widened.
 *
 * count and input_capacity <=32767; nonnull pointers even for count0. No
 * extending past logical EOF. Complete chain validation and exact writer
 * geometry precede every write. A nonempty request on a locked volume or file
 * returns MFSV_UNAVAILABLE before I/O and preserves progress.
 * Edge sectors use read-modify-write; full
 * sectors need no pre-read. No media writes occur on preflight refusal, and
 * progress is byte-for-byte preserved. Scratch/provider effects may survive.
 *
 * After preflight, success publishes DONE, completed=count, lba=0, uncertain=0.
 * An I/O error publishes the definite completed request-byte prefix and failing
 * LBA. READ failures have uncertain=0; WRITE failures have uncertain=1 because
 * the entire failing sector may have changed, including bytes outside the
 * requested range. Earlier successful sectors stay committed. No whole-fork
 * rollback, automatic retry, timestamp change, flush or snapshot is promised.
 * The return status reuses MFSV read/provider statuses; phase distinguishes
 * read and write failures. Invalid callback values return MFSV_PROVIDER.
 */
cs_mfsv_result cs_mfsw_fork_overwrite(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch,const cs_mfs_entry *entry,cs_mfsv_fork fork,
    const cs_block_writer *writer,uint32_t offset,size_t count,
    const uint8_t *input,size_t input_capacity,cs_mfsw_progress *progress);
#endif
