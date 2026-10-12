/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_BUS_H
#define CLASSICK_CPU_M68K_BUS_H
#include "m68k.h"
#include "../memory/guest.h"
typedef struct {
    cs_m68k_report cpu; cs_guest_result memory;
    uint32_t address,access; unsigned bytes;
    unsigned branch_taken,subroutine;
} cs_m68k_bus_report;
/* One bounded transaction, SPEC-0022; frame/gateway/map/output objects disjoint. */
cs_m68k_result cs_m68k_bus_step(const cs_guest_map *m,cs_trap_gateway *g,
    cs_trap_frame *f,cs_m68k_bus_report *r);
#endif
