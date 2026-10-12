/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SERVICE_PROBE_H
#define CLASSICK_SERVICE_PROBE_H
#include "../../core/services/traps.h"
typedef struct { uint32_t checks,status,signature; } cs_service_probe_result;
/* Uses borrowed disjoint native state/result storage; no hardware/guest execution. */
int cs_service_probe(cs_trap_gateway *gateway,cs_service_probe_result *out);
#endif
