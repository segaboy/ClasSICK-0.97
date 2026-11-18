/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MFS_MEDIA_PROBE_H
#define CLASSICK_MFS_MEDIA_PROBE_H
#include "../../core/storage/block-read.h"
extern volatile uint32_t cs_mfs_media_report[12];
unsigned cs_mfs_media_probe(const cs_block_device *device);
#endif
