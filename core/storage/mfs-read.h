/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_MFS_READ_H
#define CLASSICK_CORE_STORAGE_MFS_READ_H
#include "block-read.h"

typedef enum {
    CS_MFS_OK=0, CS_MFS_ARGUMENT=1, CS_MFS_SIGNATURE=2, CS_MFS_GEOMETRY=3,
    CS_MFS_ENTRY_BOUNDS=4, CS_MFS_INACTIVE=5, CS_MFS_UNAVAILABLE=6,
    CS_MFS_IO=7, CS_MFS_PROVIDER=8
} cs_mfs_result;

typedef struct {
    uint32_t created,backup,alloc_bytes,clump_bytes,next_file;
    uint16_t attributes,file_count,dir_first,dir_blocks,alloc_count,alloc_first,free_blocks;
    uint8_t name_length,name[27];
} cs_mfs_volume;

typedef struct {
    uint32_t file_id,data_logical,data_physical,resource_logical,resource_physical;
    uint32_t created,modified;
    uint16_t data_start,resource_start,offset,extent;
    uint8_t flags,version,finder[16],name_length,name[255];
} cs_mfs_entry;

typedef struct { uint8_t block[512],staging[512]; } cs_mfs_workspace;

/* SPEC-0112. Truthful live disjoint native objects/spans inside single native
 * segments, stable input/provider and no reentry are caller preconditions.
 * No arbitrary overlap/far-alias detection, guest pointers or allocation.
 * Every non-OK preserves the complete out object. Success copies raw fields
 * and counted names, zeroing unused name tails; no retained pointers or C-string
 * contract. Native structure padding is not a serialized format.
 * Decode consumes exactly one 512-byte block. read acquires only logical block2
 * using caller-owned scratch; scratch/provider effects may survive a refusal. */
cs_mfs_result cs_mfs_header_decode(const uint8_t *block,size_t length,
    uint32_t device_blocks,cs_mfs_volume *out);
cs_mfs_result cs_mfs_header_read(const cs_block_device *device,
    cs_mfs_workspace *scratch,cs_mfs_volume *out);
/* One caller-selected even offset. INACTIVE retains out, including at510;
 * no inactive-slot traversal or directory termination is inferred. */
cs_mfs_result cs_mfs_entry_decode(const uint8_t *block,size_t length,
    uint16_t offset,cs_mfs_entry *out);
#endif
