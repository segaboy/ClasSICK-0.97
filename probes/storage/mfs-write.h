/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MFS_WRITE_PROBE_H
#define CLASSICK_MFS_WRITE_PROBE_H
#include "../../core/storage/mfs-write.h"
extern volatile uint32_t cs_mfs_write_report[16];
unsigned cs_mfs_write_probe(const cs_block_device *device,const cs_block_writer *writer,unsigned mode);
#endif
