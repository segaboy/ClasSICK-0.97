/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "probe.h"
extern int cs_missing_runtime(unsigned char *storage, size_t capacity);
int cs_link_entry(unsigned char *storage, size_t capacity)
{
    return cs_missing_runtime(storage,capacity);
}
