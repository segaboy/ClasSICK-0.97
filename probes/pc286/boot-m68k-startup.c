/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../arch/i286/c16.h"
#include "../cpu/m68k-startup.h"
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
void start(void){post((unsigned char)cs_m68k_startup_probe());for(;;) { }}
