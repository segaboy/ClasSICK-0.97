/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_STARTUP_FILES_PROBE_H
#define CLASSICK_STARTUP_FILES_PROBE_H
#include "../../core/storage/startup-files.h"
#define CS_STARTUP_REPORT_WORDS 16u
extern volatile uint32_t cs_startup_files_report[CS_STARTUP_REPORT_WORDS];
unsigned cs_startup_files_probe(const cs_block_device *device);
#endif
