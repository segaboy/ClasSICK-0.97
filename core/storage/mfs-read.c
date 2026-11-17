/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-read.h"

static uint16_t read16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0]<<8)|(uint16_t)p[1]);
}
static uint32_t read32(const uint8_t *p)
{
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
        ((uint32_t)p[2]<<8)|(uint32_t)p[3];
}
/* At most ten iterations for the admitted count<=640. Check every addition
 * and every still-needed doubled term against available logical blocks. */
static int fits(uint32_t unit,uint16_t count,uint32_t available)
{
    uint32_t used=0;
    while(count!=0) {
        if((count&1u)!=0) {
            if(unit>available-used)return 0;
            used+=unit;
        }
        count=(uint16_t)(count>>1);
        if(count!=0) {
            if(unit>available || unit>available-unit)return 0;
            unit+=unit;
        }
    }
    return 1;
}
cs_mfs_result cs_mfs_header_decode(const uint8_t *block,size_t length,
    uint32_t device_blocks,cs_mfs_volume *out)
{
    uint16_t dir_first,dir_blocks,alloc_count,alloc_first,free_blocks;
    uint32_t alloc_bytes;
    unsigned i,name_length;
    if(block==NULL || out==NULL || length!=512u)return CS_MFS_ARGUMENT;
    if(device_blocks<4)return CS_MFS_GEOMETRY;
    if(read16(block)!=0xd2d7u)return CS_MFS_SIGNATURE;
    dir_first=read16(block+14);dir_blocks=read16(block+16);
    alloc_count=read16(block+18);alloc_bytes=read32(block+20);
    alloc_first=read16(block+28);free_blocks=read16(block+34);
    name_length=block[36];
    if(name_length>27u || dir_first<4u || dir_blocks==0 ||
        (uint32_t)dir_first>=device_blocks ||
        (uint32_t)dir_blocks>device_blocks-(uint32_t)dir_first ||
        alloc_first<4u || alloc_count==0 || alloc_count>640u ||
        alloc_bytes==0 || (alloc_bytes&UINT32_C(511))!=0 ||
        (uint32_t)alloc_first>=device_blocks ||
        !fits(alloc_bytes>>9,alloc_count,device_blocks-(uint32_t)alloc_first) ||
        free_blocks>alloc_count)return CS_MFS_GEOMETRY;
    /* All refusals precede this publication. Disjoint stable input/output are
     * caller preconditions, so no candidate-sized temporary or second copy. */
    out->created=read32(block+2);out->backup=read32(block+6);
    out->attributes=read16(block+10);out->file_count=read16(block+12);
    out->dir_first=dir_first;out->dir_blocks=dir_blocks;out->alloc_count=alloc_count;
    out->alloc_bytes=alloc_bytes;out->clump_bytes=read32(block+24);
    out->alloc_first=alloc_first;out->next_file=read32(block+30);out->free_blocks=free_blocks;
    out->name_length=(uint8_t)name_length;
    for(i=0;i<27u;++i)out->name[i]=i<name_length?block[37u+i]:0;
    return CS_MFS_OK;
}
cs_mfs_result cs_mfs_header_read(const cs_block_device *device,
    cs_mfs_workspace *scratch,cs_mfs_volume *out)
{
    cs_block_result result;
    if(device==NULL || scratch==NULL || out==NULL ||
        (const void *)device==(const void *)scratch ||
        (const void *)device==(const void *)out ||
        (const void *)scratch==(const void *)out)return CS_MFS_ARGUMENT;
    if(device->read==NULL || device->blocks==0)return CS_MFS_ARGUMENT;
    if(device->blocks<4)return CS_MFS_GEOMETRY;
    result=cs_block_read(device,2,1,scratch->block,sizeof scratch->block,
        scratch->staging,sizeof scratch->staging);
    if(result==CS_BLOCK_UNAVAILABLE)return CS_MFS_UNAVAILABLE;
    if(result==CS_BLOCK_IO)return CS_MFS_IO;
    if(result!=CS_BLOCK_OK)return CS_MFS_PROVIDER;
    return cs_mfs_header_decode(scratch->block,sizeof scratch->block,device->blocks,out);
}
cs_mfs_result cs_mfs_entry_decode(const uint8_t *block,size_t length,
    uint16_t offset,cs_mfs_entry *out)
{
    const uint8_t *p;
    unsigned i,extent,name_length;
    if(block==NULL || out==NULL || length!=512u || (offset&1u)!=0)return CS_MFS_ARGUMENT;
    if(offset>=512u)return CS_MFS_ENTRY_BOUNDS;
    p=block+offset;
    if((p[0]&0x80u)==0)return CS_MFS_INACTIVE;
    if(512u-offset<51u)return CS_MFS_ENTRY_BOUNDS;
    name_length=p[50];extent=(51u+name_length+1u)&~1u;
    if(extent>512u-offset)return CS_MFS_ENTRY_BOUNDS;
    out->offset=offset;out->extent=(uint16_t)extent;
    out->flags=p[0];out->version=p[1];
    for(i=0;i<16u;++i)out->finder[i]=p[2u+i];
    out->file_id=read32(p+18);out->data_start=read16(p+22);
    out->data_logical=read32(p+24);out->data_physical=read32(p+28);
    out->resource_start=read16(p+32);out->resource_logical=read32(p+34);
    out->resource_physical=read32(p+38);out->created=read32(p+42);
    out->modified=read32(p+46);out->name_length=(uint8_t)name_length;
    for(i=0;i<255u;++i)out->name[i]=i<name_length?p[51u+i]:0;
    return CS_MFS_OK;
}
