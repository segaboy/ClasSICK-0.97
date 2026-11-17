/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PROBES_STORAGE_MFS_READ_H
#define CLASSICK_PROBES_STORAGE_MFS_READ_H
/* Shared original metadata fixture, not external media or a filesystem mount.
 * B5 passes; C1..CC identify specific failing assertions. */
unsigned cs_mfs_read_probe(void);
#endif
