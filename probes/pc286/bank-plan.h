/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_BANK_PLAN_H
#define CLASSICK_PC286_BANK_PLAN_H
#include "../../arch/i286/c16.h"

/* Unbound logical metadata and proposed write plan only. */
typedef struct cs_bm_bank {
    uint32_t id, bytes;
} cs_bm_bank;

typedef struct cs_bm_region {
    uint32_t base, bytes, flags;
    uint16_t first_bank, bank_count;
} cs_bm_region;

typedef struct cs_bm_map {
    const cs_bm_region *regions;
    const cs_bm_bank *banks;
    uint16_t region_count, bank_count;
} cs_bm_map;

typedef struct cs_bm_piece {
    uint32_t bank, offset, length;
    uint8_t data[4];
} cs_bm_piece;

typedef struct cs_bm_plan {
    uint32_t count;
    cs_bm_piece pieces[4];
} cs_bm_plan;

typedef enum cs_bm_result {
    CS_BM_OK = 0,
    CS_BM_ARGUMENT = 1,
    CS_BM_MAP = 2,
    CS_BM_ADDRESS = 3,
    CS_BM_PERMISSION = 4
} cs_bm_result;

cs_bm_result cs_bm_validate(const cs_bm_map *map);
cs_bm_result cs_bm_check(const cs_bm_map *map, uint32_t at, uint32_t bytes,
                         uint32_t access);
cs_bm_result cs_bm_plan4(const cs_bm_map *map, uint32_t at, uint32_t value,
                         cs_bm_plan *out);
#endif
