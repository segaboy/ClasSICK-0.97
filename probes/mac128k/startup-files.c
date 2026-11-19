/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/mac128k/startup-floppy.h"
#include "../storage/startup-files.h"
volatile uint32_t cs_mac_diag[7];
volatile uint32_t cs_startup_checksum_retries;
static cs_mac_startup_floppy provider;static cs_block_device device;
void cs_mac_main(void)
{
    unsigned outcome=0xd0;
    if(cs_mac_startup_floppy_bind(&provider,&device))outcome=cs_startup_files_probe(&device);
    cs_startup_checksum_retries=provider.checksum_retries;
    cs_startup_files_report[14]=(uint32_t)outcome;cs_mac_diag[0]=UINT32_C(0xb8);
    for(;;) { }
}
void cs_mac_line_a_handler(void){cs_mac_diag[6]=UINT32_C(0xf002);for(;;) { }}
