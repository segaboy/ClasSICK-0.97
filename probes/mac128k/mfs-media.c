/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/mac128k/iwm-read.h"
#include "../storage/mfs-media.h"
volatile uint32_t cs_mac_diag[7];
static cs_mac_iwm provider;
static cs_block_device device;
void cs_mac_main(void)
{
    unsigned result=0xd0;
    if(cs_mac_iwm_bind(&provider,&device))result=cs_mfs_media_probe(&device);
    cs_mac_diag[0]=(uint32_t)result;
    for(;;) { }
}
void cs_mac_line_a_handler(void){cs_mac_diag[6]=UINT32_C(0xf002);for(;;) { }}
