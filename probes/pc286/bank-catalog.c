/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "bank-catalog.h"

#define CS_BC_TOP 0x100000UL
#define CS_BC_NATIVE 65536UL

/* Finds the logical bank length by id; 0 if absent. */
static int cs_bc_find(const cs_bm_map *map, uint32_t id, uint32_t *bytes)
{
    uint16_t i;
    for (i = 0u; i < map->bank_count; i++) {
        if (map->banks[i].id == id) {
            *bytes = map->banks[i].bytes;
            return 1;
        }
    }
    return 0;
}

cs_bc_result cs_bc_validate(const cs_bm_map *map, const cs_bc_bank *banks,
                            uint16_t count, uint32_t ram_first,
                            uint32_t ram_bytes)
{
    cs_bm_result mr;
    uint32_t ram_end;
    uint16_t i, j, k;

    if (banks == NULL) {
        return CS_BC_ARGUMENT;
    }
    mr = cs_bm_validate(map);
    if (mr == CS_BM_ARGUMENT) {
        return CS_BC_ARGUMENT;
    }
    if (mr != CS_BM_OK) {
        return CS_BC_MAP;
    }
    if (count != map->bank_count || ram_bytes == 0u ||
        ram_first >= CS_BC_TOP || ram_bytes > CS_BC_TOP - ram_first) {
        return CS_BC_BACKING;
    }
    ram_end = ram_first + ram_bytes;

    for (i = 0u; i < count; i++) {
        uint32_t bytes = 0u;
        for (k = 0u; k < i; k++) {
            if (banks[k].id == banks[i].id) {
                return CS_BC_BACKING;
            }
        }
        if (!cs_bc_find(map, banks[i].id, &bytes) || bytes == 0u ||
            banks[i].native_offset > 65535u ||
            bytes > CS_BC_NATIVE - banks[i].native_offset ||
            banks[i].physical_base < ram_first ||
            banks[i].physical_base >= CS_BC_TOP ||
            banks[i].physical_base > ram_end ||
            bytes > ram_end - banks[i].physical_base) {
            return CS_BC_BACKING;
        }
    }

    for (i = 0u; i < count; i++) {
        uint32_t ab = 0u, ae;
        (void)cs_bc_find(map, banks[i].id, &ab);
        ae = banks[i].physical_base + ab;
        for (j = (uint16_t)(i + 1u); j < count; j++) {
            uint32_t bb = 0u, be;
            (void)cs_bc_find(map, banks[j].id, &bb);
            be = banks[j].physical_base + bb;
            if (banks[i].physical_base < be && banks[j].physical_base < ae) {
                return CS_BC_BACKING;
            }
        }
    }
    return CS_BC_OK;
}
