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
typedef enum { CS_M68K_STRICT=0,CS_M68K_DATA24=1 } cs_m68k_data_policy;
/* Trusted internal OS adapter, never selected/cast from a guest address.
 * Preserve PC/A7; on failure preserve frame and all external state. */
typedef cs_trap_result (*cs_m68k_os_dispatch)(void *context,uint16_t word,
    cs_trap_frame *frame,cs_trap_report *report);
typedef struct { cs_m68k_data_policy data_policy;cs_m68k_os_dispatch dispatch;void *context; } cs_m68k_bus_options;
cs_m68k_result cs_m68k_bus_step_options(const cs_guest_map *m,cs_trap_gateway *g,
    cs_trap_frame *f,cs_m68k_bus_report *r,const cs_m68k_bus_options *options);
/* One bounded transaction, SPEC-0022; frame/gateway/map/output objects disjoint. */
cs_m68k_result cs_m68k_bus_step(const cs_guest_map *m,cs_trap_gateway *g,
    cs_trap_frame *f,cs_m68k_bus_report *r);
#endif
