/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MEMORY_PROBE_H
#define CLASSICK_MEMORY_PROBE_H
#include "../../core/cpu/m68k-bus.h"
typedef struct { uint32_t checks,status,signature,steps,subroutines; } cs_memory_probe_result;
int cs_memory_probe(cs_trap_gateway *g,cs_memory_probe_result *out);
#endif
