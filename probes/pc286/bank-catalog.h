/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_BANK_CATALOG_H
#define CLASSICK_PC286_BANK_CATALOG_H
#include "bank-plan.h"

/* Unbound numeric backing-catalog preflight only (SPEC-0069). */
typedef struct cs_bc_bank {
    uint32_t id, physical_base, native_offset;
} cs_bc_bank;

typedef enum cs_bc_result {
    CS_BC_OK = 0,
    CS_BC_ARGUMENT = 1,
    CS_BC_MAP = 2,
    CS_BC_BACKING = 3
} cs_bc_result;

cs_bc_result cs_bc_validate(const cs_bm_map *map, const cs_bc_bank *banks,
                            uint16_t count, uint32_t ram_first,
                            uint32_t ram_bytes);
#endif
