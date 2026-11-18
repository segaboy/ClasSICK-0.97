/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-volume.h"

#define MFSV_SEAL UINT32_C(0x43534d56)

static void copy_bytes(void *destination,const void *source,size_t count)
{
    uint8_t *d=destination;const uint8_t *s=source;size_t i;
    for(i=0;i<count;++i)d[i]=s[i];
}
static void zero_bytes(void *destination,size_t count)
{
    uint8_t *d=destination;size_t i;for(i=0;i<count;++i)d[i]=0;
}
static int zero_tail(const uint8_t *p,unsigned count)
{
    unsigned i;for(i=0;i<count;++i)if(p[i]!=0)return 0;return 1;
}
static cs_mfsv_result block_status(cs_block_result result)
{
    if(result==CS_BLOCK_OK)return CS_MFSV_OK;
    if(result==CS_BLOCK_UNAVAILABLE)return CS_MFSV_UNAVAILABLE;
    if(result==CS_BLOCK_IO)return CS_MFSV_IO;
    return CS_MFSV_PROVIDER;
}
static uint16_t map_value(const uint8_t *map,uint16_t index)
{
    unsigned at=(unsigned)index+((unsigned)index>>1);
    if((index&1u)==0)return (uint16_t)(((uint16_t)map[at]<<4)|(map[at+1u]>>4));
    return (uint16_t)((((uint16_t)map[at]&15u)<<8)|map[at+1u]);
}
/* Returns unit*count only when <=limit. No target wide multiply/divide. */
static int scaled(uint32_t unit,uint16_t count,uint32_t limit,uint32_t *out)
{
    uint32_t value=0;
    while(count!=0) {
        if((count&1u)!=0) {
            if(unit>limit-value)return 0;
            value+=unit;
        }
        count=(uint16_t)(count>>1);
        if(count!=0) {
            if(unit>limit || unit>limit-unit)return 0;
            unit+=unit;
        }
    }
    *out=value;return 1;
}
static int usable(const cs_mfsv_volume *volume)
{
    return volume!=NULL && volume->seal==MFSV_SEAL &&
        volume->device.read!=NULL && volume->device.blocks>=4 &&
        volume->info.alloc_count!=0 && volume->info.alloc_count<=640u &&
        volume->info.alloc_bytes!=0 && (volume->info.alloc_bytes&511u)==0;
}
static int allocation_base(const cs_mfs_volume *info,uint32_t blocks,
    uint16_t number,uint32_t *base)
{
    uint32_t delta,unit=info->alloc_bytes>>9;
    if(number<2u || number>(uint16_t)(info->alloc_count+1u) ||
        (uint32_t)info->alloc_first>=blocks || unit==0 ||
        !scaled(unit,(uint16_t)(number-2u),blocks-info->alloc_first,&delta))return 0;
    *base=(uint32_t)info->alloc_first+delta;
    return unit<=blocks-*base;
}
static int overlaps_directory(const cs_mfs_volume *info,uint32_t base)
{
    uint32_t end=base+(info->alloc_bytes>>9);
    uint32_t directory_end=(uint32_t)info->dir_first+info->dir_blocks;
    return base<directory_end && (uint32_t)info->dir_first<end;
}

cs_mfsv_result cs_mfsv_mount(const cs_block_device *device,
    cs_mfsv_workspace *scratch,cs_mfsv_volume *out)
{
    cs_mfs_volume info;cs_mfs_result decoded;cs_mfsv_result result;
    uint16_t i,free_count=0;const uint8_t *map;
    if(device==NULL || scratch==NULL || out==NULL || device->read==NULL ||
        (const void *)device==(const void *)scratch ||
        (const void *)device==(const void *)out ||
        (const void *)scratch==(const void *)out)return CS_MFSV_ARGUMENT;
    if(device->blocks<4)return CS_MFSV_FORMAT;
    result=block_status(cs_block_read(device,2,2,scratch->data,sizeof scratch->data,
        scratch->staging,sizeof scratch->staging));
    if(result!=CS_MFSV_OK)return result;
    zero_bytes(&info,sizeof info);
    decoded=cs_mfs_header_decode(scratch->data,512,device->blocks,&info);
    if(decoded!=CS_MFS_OK)return CS_MFSV_FORMAT;
    map=scratch->data+64;
    for(i=0;i<info.alloc_count;++i) {
        uint16_t value=map_value(map,i);uint32_t base;
        if(value==0)++free_count;
        else if(value!=1u && value!=0xfffu &&
            (value<2u || value>(uint16_t)(info.alloc_count+1u)))return CS_MFSV_FORMAT;
        if(!allocation_base(&info,device->blocks,(uint16_t)(i+2u),&base))return CS_MFSV_FORMAT;
        if(overlaps_directory(&info,base) && value!=0xfffu)return CS_MFSV_FORMAT;
    }
    if(free_count!=info.free_blocks)return CS_MFSV_FORMAT;
    out->device.blocks=device->blocks;out->device.read=device->read;
    out->device.context=device->context;
    copy_bytes(&out->info,&info,sizeof info);copy_bytes(out->map,map,sizeof out->map);
    out->seal=MFSV_SEAL;return CS_MFSV_OK;
}

cs_mfsv_result cs_mfsv_directory_next(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch,cs_mfsv_cursor *cursor,cs_mfs_entry *out)
{
    uint16_t block,offset,seen;
    if(!usable(volume) || scratch==NULL || cursor==NULL || out==NULL ||
        (const void *)scratch==(const void *)volume ||
        (const void *)scratch==(const void *)cursor ||
        (const void *)scratch==(const void *)out ||
        (const void *)cursor==(const void *)out)return CS_MFSV_ARGUMENT;
    block=cursor->block;offset=cursor->offset;seen=cursor->seen;
    if(block>volume->info.dir_blocks || offset>512u || (offset&1u)!=0 ||
        seen>volume->info.file_count || (block==volume->info.dir_blocks && offset!=0))
        return CS_MFSV_ARGUMENT;
    while(block<volume->info.dir_blocks) {
        cs_mfsv_result read_result;cs_mfs_result decoded;
        uint32_t lba=(uint32_t)volume->info.dir_first+block;
        if(offset==512u){++block;offset=0;continue;}
        read_result=block_status(cs_block_read(&volume->device,lba,1,
            scratch->data,sizeof scratch->data,scratch->staging,sizeof scratch->staging));
        if(read_result!=CS_MFSV_OK)return read_result;
        if(scratch->data[offset]==0) {
            if(!zero_tail(scratch->data+offset,512u-offset))return CS_MFSV_FORMAT;
            ++block;offset=0;continue;
        }
        if((scratch->data[offset]&0x80u)==0 || seen>=volume->info.file_count)
            return CS_MFSV_FORMAT;
        decoded=cs_mfs_entry_decode(scratch->data,512,offset,out);
        if(decoded!=CS_MFS_OK)return CS_MFSV_FORMAT;
        cursor->block=block;cursor->offset=(uint16_t)(offset+out->extent);
        cursor->seen=(uint16_t)(seen+1u);return CS_MFSV_OK;
    }
    if(seen!=volume->info.file_count)return CS_MFSV_FORMAT;
    cursor->block=block;cursor->offset=0;cursor->seen=seen;return CS_MFSV_END;
}

cs_mfsv_result cs_mfsv_find(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch,const uint8_t *name,size_t length,cs_mfs_entry *out)
{
    cs_mfsv_cursor cursor;cs_mfs_entry candidate;cs_mfsv_result result;size_t i;
    if(name==NULL || out==NULL || length>255u || (const void *)name==(const void *)out)
        return CS_MFSV_ARGUMENT;
    zero_bytes(&cursor,sizeof cursor);zero_bytes(&candidate,sizeof candidate);
    for(;;) {
        result=cs_mfsv_directory_next(volume,scratch,&cursor,&candidate);
        if(result==CS_MFSV_END)return CS_MFSV_NOT_FOUND;
        if(result!=CS_MFSV_OK)return result;
        if(candidate.name_length!=length)continue;
        for(i=0;i<length && candidate.name[i]==name[i];++i) { }
        if(i==length){copy_bytes(out,&candidate,sizeof candidate);return CS_MFSV_OK;}
    }
}

static cs_mfsv_result fork_fields(const cs_mfsv_volume *volume,const cs_mfs_entry *entry,
    cs_mfsv_fork fork,uint16_t *start,uint32_t *logical,uint32_t *physical)
{
    if(!usable(volume) || entry==NULL || (fork!=CS_MFSV_DATA && fork!=CS_MFSV_RESOURCE))
        return CS_MFSV_ARGUMENT;
    if(fork==CS_MFSV_DATA) {
        *start=entry->data_start;*logical=entry->data_logical;*physical=entry->data_physical;
    } else {
        *start=entry->resource_start;*logical=entry->resource_logical;*physical=entry->resource_physical;
    }
    if(*logical>*physical || ((*start==0)!=(*physical==0)))return CS_MFSV_CHAIN;
    return CS_MFSV_OK;
}

cs_mfsv_result cs_mfsv_fork_validate(const cs_mfsv_volume *volume,
    const cs_mfs_entry *entry,cs_mfsv_fork fork)
{
    uint8_t visited[80];uint16_t current,index,next;uint32_t logical,physical,total=0;
    cs_mfsv_result result=fork_fields(volume,entry,fork,&current,&logical,&physical);
    if(result!=CS_MFSV_OK)return result;
    if(current==0)return CS_MFSV_OK;
    zero_bytes(visited,sizeof visited);
    for(;;) {
        uint8_t bit;uint32_t base;
        if(!allocation_base(&volume->info,volume->device.blocks,current,&base) ||
            overlaps_directory(&volume->info,base))return CS_MFSV_CHAIN;
        index=(uint16_t)(current-2u);bit=(uint8_t)(1u<<(index&7u));
        if((visited[index>>3]&bit)!=0)return CS_MFSV_CHAIN;
        visited[index>>3]|=bit;
        if(volume->info.alloc_bytes>physical-total)return CS_MFSV_CHAIN;
        total+=volume->info.alloc_bytes;
        next=map_value(volume->map,index);
        if(next==1u)return total==physical?CS_MFSV_OK:CS_MFSV_CHAIN;
        if(next<2u || next>(uint16_t)(volume->info.alloc_count+1u))return CS_MFSV_CHAIN;
        current=next;
    }
}

cs_mfsv_result cs_mfsv_fork_read(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch,const cs_mfs_entry *entry,cs_mfsv_fork fork,
    uint32_t offset,size_t count,uint8_t *output,size_t output_capacity,
    uint8_t *staging,size_t staging_capacity)
{
    uint16_t current;uint32_t logical,physical,in_allocation;size_t written=0,i;
    cs_mfsv_result result;
    if(scratch==NULL || output==NULL || staging==NULL || output==staging ||
        count>CS_BLOCK_NATIVE_LIMIT || output_capacity>CS_BLOCK_NATIVE_LIMIT ||
        staging_capacity>CS_BLOCK_NATIVE_LIMIT ||
        (const void *)scratch==(const void *)output ||
        (const void *)scratch==(const void *)staging)return CS_MFSV_ARGUMENT;
    result=fork_fields(volume,entry,fork,&current,&logical,&physical);
    if(result!=CS_MFSV_OK)return result;
    if(offset>logical || (uint32_t)count>logical-offset)return CS_MFSV_RANGE;
    if(output_capacity<count || staging_capacity<count)return CS_MFSV_CAPACITY;
    result=cs_mfsv_fork_validate(volume,entry,fork);
    if(result!=CS_MFSV_OK)return result;
    if(count==0)return CS_MFSV_OK;
    in_allocation=offset;
    while(in_allocation>=volume->info.alloc_bytes) {
        in_allocation-=volume->info.alloc_bytes;
        current=map_value(volume->map,(uint16_t)(current-2u));
    }
    while(written<count) {
        uint32_t base,lba;unsigned within=(unsigned)(in_allocation&511u);
        size_t take=(size_t)(512u-within);
        if(!allocation_base(&volume->info,volume->device.blocks,current,&base))return CS_MFSV_CHAIN;
        lba=base+(in_allocation>>9);
        if(take>count-written)take=count-written;
        result=block_status(cs_block_read(&volume->device,lba,1,scratch->data,sizeof scratch->data,
            scratch->staging,sizeof scratch->staging));
        if(result!=CS_MFSV_OK)return result;
        for(i=0;i<take;++i)staging[written+i]=scratch->data[within+i];
        written+=take;in_allocation+=(uint32_t)take;
        if(in_allocation==volume->info.alloc_bytes && written<count) {
            current=map_value(volume->map,(uint16_t)(current-2u));in_allocation=0;
        }
    }
    copy_bytes(output,staging,count);return CS_MFSV_OK;
}
