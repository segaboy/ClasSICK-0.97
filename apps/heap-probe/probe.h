/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_HEAP_PROBE_H
#define CLASSICK_HEAP_PROBE_H
#include "../../personality/system1/memory-registers.h"
typedef struct { uint32_t checks,status,signature; } cs_heap_probe_result;
/* Fresh initialized manager with at least 256 writable payload bytes per zone. */
int cs_heap_probe(cs_mm_manager *m,cs_heap_probe_result *out);
#endif
