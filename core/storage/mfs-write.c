/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-write.h"

static uint16_t next_allocation(const cs_mfsv_volume *volume,uint16_t allocation)
{
    uint16_t index=(uint16_t)(allocation-2u);
    unsigned at=(unsigned)index+((unsigned)index>>1);
    if((index&1u)==0)return (uint16_t)(((uint16_t)volume->map[at]<<4)|(volume->map[at+1u]>>4));
    return (uint16_t)((((uint16_t)volume->map[at]&15u)<<8)|volume->map[at+1u]);
}

/* The complete chain and identical writer geometry have already validated
 * this product and sum. Bounded shift/add avoids a native wide-multiply helper. */
static uint32_t allocation_lba(const cs_mfsv_volume *volume,uint16_t allocation)
{
    uint16_t index=(uint16_t)(allocation-2u);
    uint32_t unit=volume->info.alloc_bytes>>9,delta=0;
    while(index!=0) {
        if((index&1u)!=0)delta+=unit;
        index=(uint16_t)(index>>1);
        if(index!=0)unit+=unit;
    }
    return (uint32_t)volume->info.alloc_first+delta;
}

static void publish(cs_mfsw_progress *out,size_t completed,uint32_t lba,
    cs_mfsw_phase phase,uint8_t uncertain)
{
    out->completed=(uint32_t)completed;out->lba=lba;
    out->phase=phase;out->uncertain=uncertain;
}

static cs_mfsv_result read_status(cs_block_result status)
{
    if(status==CS_BLOCK_OK)return CS_MFSV_OK;
    if(status==CS_BLOCK_UNAVAILABLE)return CS_MFSV_UNAVAILABLE;
    if(status==CS_BLOCK_IO)return CS_MFSV_IO;
    return CS_MFSV_PROVIDER;
}

static cs_mfsv_result write_status(cs_block_io_result status)
{
    if(status==CS_BLOCK_IO_OK)return CS_MFSV_OK;
    if(status==CS_BLOCK_IO_UNAVAILABLE)return CS_MFSV_UNAVAILABLE;
    if(status==CS_BLOCK_IO_ERROR)return CS_MFSV_IO;
    return CS_MFSV_PROVIDER;
}

cs_mfsv_result cs_mfsw_fork_overwrite(const cs_mfsv_volume *volume,
    cs_mfsv_workspace *scratch,const cs_mfs_entry *entry,cs_mfsv_fork fork,
    const cs_block_writer *writer,uint32_t offset,size_t count,
    const uint8_t *input,size_t input_capacity,cs_mfsw_progress *progress)
{
    const void *objects[6];unsigned i,j;uint16_t allocation;
    uint32_t logical,in_allocation;size_t completed=0;
    cs_mfsv_result result;
    objects[0]=volume;objects[1]=scratch;objects[2]=entry;
    objects[3]=writer;objects[4]=input;objects[5]=progress;
    for(i=0;i<6u;++i) {
        if(objects[i]==NULL)return CS_MFSV_ARGUMENT;
        for(j=0;j<i;++j)if(objects[i]==objects[j])return CS_MFSV_ARGUMENT;
    }
    if(writer->write==NULL || writer->blocks!=volume->device.blocks ||
        (fork!=CS_MFSV_DATA && fork!=CS_MFSV_RESOURCE) ||
        count>CS_BLOCK_NATIVE_LIMIT || input_capacity>CS_BLOCK_NATIVE_LIMIT)
        return CS_MFSV_ARGUMENT;
    if(input_capacity<count)return CS_MFSV_CAPACITY;
    logical=fork==CS_MFSV_DATA?entry->data_logical:entry->resource_logical;
    if(offset>logical || (uint32_t)count>logical-offset)return CS_MFSV_RANGE;
    result=cs_mfsv_fork_validate(volume,entry,fork);
    if(result!=CS_MFSV_OK)return result;
    if(count==0) {
        publish(progress,0,0,CS_MFSW_DONE,0);return CS_MFSV_OK;
    }
    if((volume->info.attributes & UINT16_C(0x8080)) != 0 || (entry->flags & 1u) != 0)
        return CS_MFSV_UNAVAILABLE;
    allocation=fork==CS_MFSV_DATA?entry->data_start:entry->resource_start;
    in_allocation=offset;
    while(in_allocation>=volume->info.alloc_bytes) {
        in_allocation-=volume->info.alloc_bytes;
        allocation=next_allocation(volume,allocation);
    }
    while(completed<count) {
        const uint32_t lba=allocation_lba(volume,allocation)+(in_allocation>>9);
        const unsigned within=(unsigned)(in_allocation&511u);
        size_t take=(size_t)(512u-within),index;
        const uint8_t *sector;
        if(take>count-completed)take=count-completed;
        if(within!=0 || take!=512u) {
            result=read_status(cs_block_read(&volume->device,lba,1,scratch->data,
                sizeof scratch->data,scratch->staging,sizeof scratch->staging));
            if(result!=CS_MFSV_OK) {
                publish(progress,completed,lba,CS_MFSW_READ,0);return result;
            }
            for(index=0;index<take;++index)scratch->data[within+index]=input[completed+index];
            sector=scratch->data;
        } else sector=input+completed;
        result=write_status(writer->write(writer->context,lba,sector));
        if(result!=CS_MFSV_OK) {
            publish(progress,completed,lba,CS_MFSW_WRITE,1);return result;
        }
        completed+=take;
        in_allocation+=(uint32_t)take;
        if(in_allocation==volume->info.alloc_bytes && completed<count) {
            allocation=next_allocation(volume,allocation);in_allocation=0;
        }
    }
    publish(progress,completed,0,CS_MFSW_DONE,0);return CS_MFSV_OK;
}
