/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_ALINE_PROBE_H
#define CLASSICK_ALINE_PROBE_H
#include "../../core/cpu/m68k.h"
typedef struct {uint32_t checks,status,signature,steps,calls;} cs_aline_probe_result;
int cs_aline_probe(cs_trap_gateway *g,cs_aline_probe_result *out);
#endif
