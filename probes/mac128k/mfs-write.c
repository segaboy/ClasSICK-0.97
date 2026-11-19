/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/mac128k/iwm-write.h"
#include "../storage/mfs-write.h"
#ifndef PROBE_CASE
#define PROBE_CASE 0
#endif
volatile uint32_t cs_mac_diag[8];
static cs_mac_iwm provider;
static cs_block_device device;
static cs_block_writer writer;
static uint8_t write_scratch[512];
static cs_block_io_result write_one(void *context,uint32_t lba,const uint8_t *block)
{
    cs_mac_iwm_write_result result=cs_mac_iwm_write_one(context,lba,block,write_scratch);
    return result==CS_MAC_IWM_WRITE_OK?CS_BLOCK_IO_OK:CS_BLOCK_IO_ERROR;
}
void cs_mac_main(void)
{
    unsigned result=0xd0;
    if(cs_mac_iwm_bind(&provider,&device)) {
        writer.blocks=device.blocks;writer.write=write_one;writer.context=&provider;
        result=cs_mfs_write_probe(&device,&writer,PROBE_CASE);
    }
    cs_mac_diag[0]=(uint32_t)result;
    for(;;) { }
}
void cs_mac_line_a_handler(void){cs_mac_diag[7]=UINT32_C(0xf002);for(;;) { }}
