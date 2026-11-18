/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_MFS_VOLUME_H
#define CLASSICK_CORE_STORAGE_MFS_VOLUME_H
#include "mfs-read.h"

typedef enum {
    CS_MFSV_OK=0, CS_MFSV_END=1, CS_MFSV_ARGUMENT=2, CS_MFSV_FORMAT=3,
    CS_MFSV_CHAIN=4, CS_MFSV_RANGE=5, CS_MFSV_CAPACITY=6,
    CS_MFSV_UNAVAILABLE=7, CS_MFSV_IO=8, CS_MFSV_PROVIDER=9,
    CS_MFSV_NOT_FOUND=10
} cs_mfsv_result;
typedef enum { CS_MFSV_DATA=0, CS_MFSV_RESOURCE=1 } cs_mfsv_fork;
typedef struct {
    cs_block_device device;
    cs_mfs_volume info;
    uint8_t map[960];
    uint32_t seal;
} cs_mfsv_volume;
typedef struct { uint8_t data[1024], staging[1024]; } cs_mfsv_workspace;
typedef struct { uint16_t block, offset, seen; } cs_mfsv_cursor;

/* SPEC-0115. All controls, provider storage, inputs, buffers and outputs are
 * truthful live mutually disjoint native objects within individual segments.
 * The provider/media and mounted volume remain stable, exclusively owned and
 * unmodified through their use. No arbitrary alias/pointer-validity detection.
 * No allocation, guest addresses, writable media or historical collation.
 * Errors preserve output objects and cursors; scratch/provider effects remain.
 * Zero-initialize a cursor before the first directory_next call. END commits
 * its exhausted cursor but preserves out. Metadata names are counted bytes.
 */
cs_mfsv_result cs_mfsv_mount(const cs_block_device *device,
    cs_mfsv_workspace *scratch, cs_mfsv_volume *out);
cs_mfsv_result cs_mfsv_directory_next(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch, cs_mfsv_cursor *cursor, cs_mfs_entry *out);
cs_mfsv_result cs_mfsv_find(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch, const uint8_t *name, size_t length,
    cs_mfs_entry *out);
cs_mfsv_result cs_mfsv_fork_validate(const cs_mfsv_volume *volume,
    const cs_mfs_entry *entry, cs_mfsv_fork fork);
/* count and both native capacities <=32767. Reads are exact, never truncated
 * at EOF; count0 accepts offset==logical EOF and still validates the full fork.
 * Nonnull disjoint output/staging are required even for count0. Full chain
 * validation precedes I/O; every requested byte stages before publication.
 * Unused output/staging tails are unchanged. Input disk/controls never change.
 */
cs_mfsv_result cs_mfsv_fork_read(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch, const cs_mfs_entry *entry, cs_mfsv_fork fork,
    uint32_t offset, size_t count, uint8_t *output, size_t output_capacity,
    uint8_t *staging, size_t staging_capacity);
#endif
