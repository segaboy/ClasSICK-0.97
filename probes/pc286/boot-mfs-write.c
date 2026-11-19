/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc286/boothush-floppy-write.h"
#include "../storage/mfs-write.h"
#ifndef PROBE_CASE
#define PROBE_CASE 0
#endif
static cs_pc286_floppy reader;
static cs_pc286_floppy_writer native_writer;
static cs_block_device device;
static cs_block_writer writer;
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static cs_block_io_result write_one(void *context,uint32_t lba,const uint8_t *block)
{
    cs_pc286_write_result result=cs_pc286_floppy_write_one(context,lba,block);
    return result==CS_PC286_WRITE_OK?CS_BLOCK_IO_OK:CS_BLOCK_IO_ERROR;
}
void start(void)
{
    unsigned result=0xd0;
    if(cs_pc286_floppy_bind(&reader,60u,800u,&device) &&
        cs_pc286_floppy_write_bind(&native_writer,60u,800u)) {
        writer.blocks=800;writer.write=write_one;writer.context=&native_writer;
        result=cs_mfs_write_probe(&device,&writer,PROBE_CASE);
    }
    post((unsigned char)result);for(;;) { }
}
