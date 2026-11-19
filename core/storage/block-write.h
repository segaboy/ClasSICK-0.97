/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_BLOCK_WRITE_H
#define CLASSICK_CORE_STORAGE_BLOCK_WRITE_H
#include "block-read.h"

/* Synchronous one-sector callback under SPEC-0128. Success commits exactly
 * block[512]. The source is live, stable, disjoint from provider state, never
 * retained or modified. Any other result may have changed ANY bytes of the
 * addressed sector, including all of it; no rollback or partial count is
 * implied. Other sectors are unchanged. The caller owns serialization and
 * admits the concrete provider/media; a descriptor is not write authorization. */
typedef cs_block_io_result (*cs_block_write_one)(void *context,uint32_t lba,
    const uint8_t *block);
typedef struct {
    uint32_t blocks;
    cs_block_write_one write;
    void *context;
} cs_block_writer;
#endif
