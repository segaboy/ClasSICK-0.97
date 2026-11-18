/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_RESOURCEFORK_MFS_TRANSPORT_H
#define CLASSICK_RESOURCEFORK_MFS_TRANSPORT_H
#include "transport.h"
#include "../../core/storage/mfs-volume.h"
typedef struct {
    const cs_mfsv_volume *volume;const cs_mfs_entry *entry;
    cs_mfsv_workspace *scratch;uint8_t staging[CS_RFS_CHUNK];
} cs_rfs_mfs_adapter;
/* Validate the complete resource fork, then bind borrowed stable controls.
 * Failure preserves adapter/source. No disk I/O or allocation during bind.
 * Adapter, source, volume, entry and scratch must be mutually disjoint.
 */
cs_mfsv_result cs_rfs_mfs_bind(const cs_mfsv_volume *volume,
    const cs_mfs_entry *entry,cs_mfsv_workspace *scratch,
    cs_rfs_mfs_adapter *adapter,cs_rfs_source *source);
#endif
