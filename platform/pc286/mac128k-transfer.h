/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_MAC128K_TRANSFER_H
#define CLASSICK_PC286_MAC128K_TRANSFER_H
#include "mac128k-ram.h"
#include "../../core/cpu/m68k-transfer.h"

cs_pg_result cs_pc286_mac128k_transfer_backend(cs_m68k_transfer_backend *out,
    const cs_pc286_m68k_memory *memory);
#endif
