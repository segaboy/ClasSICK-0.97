/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mac-boot-header.h"

static uint16_t read16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0]<<8)|(uint16_t)p[1]);
}
static uint32_t read32(const uint8_t *p)
{
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
        ((uint32_t)p[2]<<8)|(uint32_t)p[3];
}
cs_mac_boot_result cs_mac_boot_header_decode(const uint8_t *area,size_t length,
    cs_mac_boot_header *out)
{
    unsigned i,j,offset;
    if(area==NULL || out==NULL || length!=CS_MAC_BOOT_AREA_BYTES ||
        (const void *)area==(const void *)out)return CS_MAC_BOOT_ARGUMENT;
    if(read16(area)!=0x4c4bu)return CS_MAC_BOOT_SIGNATURE;
    for(i=0;i<CS_MAC_BOOT_NAME_COUNT;++i)
        if(area[10u+16u*i]>15u)return CS_MAC_BOOT_NAME;
    /* Stable disjoint source/output are preconditions. No fallible operation
     * follows this point, and no entry or trailing byte is accessed. */
    out->signature=read16(area);out->version=read16(area+6);
    out->page_flags=read16(area+8);out->fcb_count=read16(area+122);
    out->event_count=read16(area+124);out->heap_128k=read32(area+126);
    out->heap_256k=read32(area+130);out->heap_other=read32(area+134);
    for(i=0;i<CS_MAC_BOOT_NAME_COUNT;++i) {
        offset=10u+16u*i;out->names[i].length=area[offset];
        for(j=0;j<15u;++j)
            out->names[i].bytes[j]=j<area[offset]?area[offset+1u+j]:0;
    }
    return CS_MAC_BOOT_OK;
}
cs_mac_boot_result cs_mac_boot_header_read(const cs_block_device *device,
    cs_mac_boot_workspace *scratch,cs_mac_boot_header *out)
{
    cs_block_result result;
    if(device==NULL || scratch==NULL || out==NULL ||
        (const void *)device==(const void *)scratch ||
        (const void *)device==(const void *)out ||
        (const void *)scratch==(const void *)out)return CS_MAC_BOOT_ARGUMENT;
    if(device->read==NULL || device->blocks==0)return CS_MAC_BOOT_ARGUMENT;
    if(device->blocks<2)return CS_MAC_BOOT_RANGE;
    result=cs_block_read(device,0,2,scratch->area,sizeof scratch->area,
        scratch->staging,sizeof scratch->staging);
    if(result==CS_BLOCK_UNAVAILABLE)return CS_MAC_BOOT_UNAVAILABLE;
    if(result==CS_BLOCK_IO)return CS_MAC_BOOT_IO;
    if(result!=CS_BLOCK_OK)return CS_MAC_BOOT_PROVIDER;
    return cs_mac_boot_header_decode(scratch->area,sizeof scratch->area,out);
}
