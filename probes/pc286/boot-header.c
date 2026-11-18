/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc286/boothush-floppy-read.h"
#include "../storage/mac-boot-header.h"
static cs_pc286_floppy provider;
static cs_block_device device;
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
void start(void)
{
    unsigned result=0xd0;
    if(cs_pc286_floppy_bind(&provider,60u,800u,&device))result=cs_boot_header_probe(&device);
    post((unsigned char)result);for(;;) { }
}
