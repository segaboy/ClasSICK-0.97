/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mac-boot-header.h"
#include "../../core/storage/mac-boot-header.h"
volatile uint32_t cs_boot_header_report[16];
static cs_mac_boot_workspace work;
static struct {uint8_t before[8];cs_mac_boot_header value;uint8_t after[8];} guarded;
static const cs_block_device *provider;
static unsigned calls;
static cs_block_io_result read_one(void *unused,uint32_t lba,uint8_t *out)
{
    (void)unused;++calls;
    if(lba!=(uint32_t)(calls-1u) || calls>2u)return CS_BLOCK_IO_ERROR;
    return provider->read(provider->context,lba,out);
}
static int uniform(const void *p,size_t n,uint8_t value)
{
    const uint8_t *b=p;size_t i;for(i=0;i<n;++i)if(b[i]!=value)return 0;return 1;
}
unsigned cs_boot_header_probe(const cs_block_device *device)
{
    cs_block_device wrapped;cs_mac_boot_result result;
    uint8_t *bytes=(uint8_t *)&guarded;uint32_t digest=0;size_t i;unsigned j;
    provider=device;calls=0;wrapped.blocks=device->blocks;wrapped.read=read_one;wrapped.context=NULL;
    for(i=0;i<sizeof guarded;++i)bytes[i]=0xa5;
    for(i=0;i<sizeof work.area;++i){work.area[i]=0x73;work.staging[i]=0x73;}
    for(i=0;i<16;++i)cs_boot_header_report[i]=0;
    result=cs_mac_boot_header_read(&wrapped,&work,&guarded.value);
    cs_boot_header_report[0]=UINT32_C(0x4d424844);cs_boot_header_report[1]=(uint32_t)result;
    cs_boot_header_report[2]=(uint32_t)calls;
    if(!uniform(guarded.before,8,0xa5) || !uniform(guarded.after,8,0xa5))return 0xe1;
    cs_boot_header_report[12]=1;
    if(result!=CS_MAC_BOOT_OK) {
        if(!uniform(&guarded.value,sizeof guarded.value,0xa5))return 0xe2;
        cs_boot_header_report[13]=1;
        if(result==CS_MAC_BOOT_UNAVAILABLE || result==CS_MAC_BOOT_IO || result==CS_MAC_BOOT_PROVIDER) {
            if(!uniform(work.area,sizeof work.area,0x73))return 0xe3;
            cs_boot_header_report[14]=1;
        }
        return 0xb7;
    }
    cs_boot_header_report[3]=guarded.value.signature;cs_boot_header_report[4]=guarded.value.version;
    cs_boot_header_report[5]=guarded.value.page_flags;cs_boot_header_report[6]=guarded.value.fcb_count;
    cs_boot_header_report[7]=guarded.value.event_count;cs_boot_header_report[8]=guarded.value.heap_128k;
    cs_boot_header_report[9]=guarded.value.heap_256k;cs_boot_header_report[10]=guarded.value.heap_other;
    for(i=0;i<7;++i) {
        digest=((digest<<5)|(digest>>27))^guarded.value.names[i].length;
        for(j=0;j<15;++j)digest=((digest<<5)|(digest>>27))^guarded.value.names[i].bytes[j];
    }
    cs_boot_header_report[11]=digest;
    return 0xb7;
}
