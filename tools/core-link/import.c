/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Intentional import-audit negative control; linked, never loaded or executed. */
#include "probe.h"
#include <windows.h>
int cs_link_entry(unsigned char *storage, size_t capacity)
{
    (void)storage; (void)capacity;
    return DestroyWindow(NULL) ? 0 : 1;
}
