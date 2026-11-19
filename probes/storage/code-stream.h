/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CODE_STREAM_PROBE_H
#define CLASSICK_CODE_STREAM_PROBE_H
#include "../../core/storage/block-read.h"
#define CS_CODE_STREAM_WORDS 16u
extern volatile uint32_t cs_code_stream_report[CS_CODE_STREAM_WORDS];
unsigned cs_code_stream_probe(const cs_block_device *device);
#endif
