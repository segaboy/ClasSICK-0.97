/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_RESOURCE_MEDIA_PROBE_H
#define CLASSICK_RESOURCE_MEDIA_PROBE_H
#include "../../core/storage/block-read.h"
#define CS_RESOURCE_MEDIA_WORDS 16u
extern volatile uint32_t cs_resource_media_report[CS_RESOURCE_MEDIA_WORDS];
unsigned cs_resource_media_probe(const cs_block_device *device);
#endif
