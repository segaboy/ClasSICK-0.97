/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_STORE4_MODEL_H
#define CLASSICK_PC286_STORE4_MODEL_H
#include "bank-catalog.h"
#define CS_BS_MAX_SPANS 192u
typedef struct cs_bs_span {
    uint32_t bank, offset, bytes;
    uint8_t *data;
} cs_bs_span;
typedef enum cs_bs_result {
    CS_BS_OK=0, CS_BS_ARGUMENT=1, CS_BS_MAP=2, CS_BS_BACKING=3,
    CS_BS_STALE=4, CS_BS_ADDRESS=5, CS_BS_PERMISSION=6,
    CS_BS_BINDING=7, CS_BS_EXHAUSTED=8
} cs_bs_result;
/* Host-memory experiment only. Caller owns all real extents,lifetimes,nonaliasing,
 * immutable metadata and exclusive binding stability. No native/far conversion. */
cs_bs_result cs_bs_store4(const cs_bm_map *map, const cs_bc_bank *catalog,
    uint16_t catalog_count, uint32_t ram_first, uint32_t ram_bytes,
    const cs_bs_span *spans, uint16_t span_count,
    uint32_t current_epoch, uint32_t expected_epoch,
    uint32_t at, uint32_t value);
cs_bs_result cs_bs_epoch_next(uint32_t current, uint32_t *out);
#endif
