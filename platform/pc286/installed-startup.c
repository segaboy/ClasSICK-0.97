/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "boothush-floppy-read.h"
#include "../../core/storage/startup-files.h"

/* SPEC-0131. The installed payload owns its workspace and the two open
 * resource sources for the rest of this boot. The separate 1.2 MB transport
 * carries all 800 Macintosh logical sectors at LBA 0. */
static cs_pc286_floppy drive;
static cs_block_device disk;
static cs_startup_workspace workspace;
static cs_startup_files files;
static cs_startup_report detail;
volatile uint32_t cs_installed_startup_state[8];

static void post(unsigned char code);
#pragma aux post = "out 80h,al" parm [al] modify exact [];

void start(void)
{
    unsigned char code = 0xd0;
    if (cs_pc286_floppy_bind(&drive, 0, 800, &disk)) {
        cs_startup_result result = cs_startup_files_open(&disk, &workspace, &files, &detail);
        cs_installed_startup_state[0] = (uint32_t)result;
        cs_installed_startup_state[1] = (uint32_t)detail.stage;
        cs_installed_startup_state[2] = (uint32_t)detail.domain;
        cs_installed_startup_state[3] = detail.status;
        cs_installed_startup_state[4] = detail.opened;
        cs_installed_startup_state[5] = detail.types;
        cs_installed_startup_state[6] = detail.references;
        cs_installed_startup_state[7] = detail.warnings;
        code = result == CS_STARTUP_OK ? 0xb8 : 0xd1;
    }
    post(code);
    for (;;) { }
}
