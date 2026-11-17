/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PROBES_STORAGE_BLOCK_READ_H
#define CLASSICK_PROBES_STORAGE_BLOCK_READ_H
/* Original RAM fixture: 0xb4 passes; 0xe1..0xea identify failed checks.
 * Native stack needs two 1026-byte buffers plus locals/calls. No static data,
 * heap, system services, historical media or hardware is used by this probe. */
unsigned cs_block_read_probe(void);
#endif
