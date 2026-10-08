/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_LINK_PROBE_H
#define CLASSICK_CORE_LINK_PROBE_H
#include <stddef.h>
#define CS_CORE_LINK_STORAGE 128u
/* Caller storage must permit native typed objects and satisfy arena alignment. */
int cs_core_link_probe(unsigned char *storage, size_t capacity);
int cs_link_entry(unsigned char *storage, size_t capacity);
#endif
