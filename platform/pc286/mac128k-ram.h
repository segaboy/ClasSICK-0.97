/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_MAC128K_RAM_H
#define CLASSICK_PC286_MAC128K_RAM_H
#include "m68k-banked.h"

/* SPEC-0139. Use after the boot overlay is released, with exclusively owned
 * ordinary bank RAM and the complete native-envelope contract of SPEC-0120. */
cs_pg_result cs_pc286_mac128k_backend(cs_m68k_bus_backend *out,const cs_pc286_m68k_memory *memory);
#endif
