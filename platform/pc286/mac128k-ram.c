/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mac128k-ram.h"

#define RAM_SIZE 0x20000UL
#define RAM_LIMIT 0x400000UL

static cs_pg_result physical(cs_m68k_bus_backend *base,const cs_pc286_m68k_memory *memory)
{
    cs_pg_result result=cs_pc286_m68k_backend(base,memory);
    if(result!=CS_PG_OK)return result;
    unsigned found=0;
    for(unsigned i=0;i<memory->guest.region_count;++i){
        const cs_pg_region *region=&memory->guest.regions[i];
        if(region->base>=RAM_LIMIT)continue;
        if(region->base!=0 || region->bytes!=RAM_SIZE)return CS_PG_MAP;
        ++found;
    }
    return found==1?CS_PG_OK:CS_PG_MAP;
}

static cs_guest_result validate(const void *context)
{
    cs_m68k_bus_backend base;
    return physical(&base,context)==CS_PG_OK?CS_GUEST_OK:CS_GUEST_MAP;
}

static cs_guest_result span(const void *context,uint32_t at,uint32_t bytes,uint32_t access)
{
    cs_m68k_bus_backend base;
    if(physical(&base,context)!=CS_PG_OK)return CS_GUEST_MAP;
    if(at<RAM_LIMIT){
        if(!bytes || bytes>RAM_LIMIT-at)return CS_GUEST_ADDRESS;
        at&=RAM_SIZE-1;
        /* Every mirror selects the very same region and access permissions. */
        if(bytes>RAM_SIZE-at){at=0;bytes=RAM_SIZE;}
    }
    return base.span(context,at,bytes,access);
}

/* For a valid even scalar, only a longword can straddle the physical wrap.
 * A transfer crossing the aperture's top instead crosses into a different
 * device selection and is refused as a unit. */
static cs_guest_result address(uint32_t *at,unsigned width)
{
    if(width!=1 && width!=2 && width!=4)return CS_GUEST_WIDTH;
    if(width>1 && (*at&1u))return CS_GUEST_ADDRESS;
    if(*at<RAM_LIMIT){
        if(width>RAM_LIMIT-*at)return CS_GUEST_ADDRESS;
        *at&=RAM_SIZE-1;
    }
    return CS_GUEST_OK;
}

static cs_guest_result read_ram(const void *context,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{
    cs_m68k_bus_backend base;
    uint32_t high,low;
    if(out==NULL || !(access&CS_PG_READ) || (access&~7UL))return CS_GUEST_ARGUMENT;
    if(physical(&base,context)!=CS_PG_OK)return CS_GUEST_MAP;
    cs_guest_result result=address(&at,width);
    if(result!=CS_GUEST_OK)return result;
    if(width!=4 || at!=RAM_SIZE-2)return base.read(context,at,width,access,out);
    result=base.read(context,at,2,access,&high);
    if(result==CS_GUEST_OK)result=base.read(context,0,2,access,&low);
    if(result==CS_GUEST_OK)*out=(high<<16)|low;
    return result;
}

static cs_guest_result write_ram(const void *context,uint32_t at,unsigned width,uint32_t value)
{
    cs_m68k_bus_backend base;
    if(physical(&base,context)!=CS_PG_OK)return CS_GUEST_MAP;
    cs_guest_result result=address(&at,width);
    if(result!=CS_GUEST_OK)return result;
    if(width!=4 || at!=RAM_SIZE-2)return base.write(context,at,width,value);
    /* Both aligned destination words are in validated ordinary RAM under
     * the same generation and an exclusive, immutable bank layout. After
     * permission/coverage preflight no further refusal can occur while
     * writing them. This contract cannot be used for MMIO or DMA. */
    result=base.span(context,at,2,CS_PG_WRITE);
    if(result==CS_GUEST_OK)result=base.span(context,0,2,CS_PG_WRITE);
    if(result==CS_GUEST_OK)result=base.write(context,at,2,value>>16);
    if(result==CS_GUEST_OK)result=base.write(context,0,2,value);
    return result;
}

cs_pg_result cs_pc286_mac128k_backend(cs_m68k_bus_backend *out,const cs_pc286_m68k_memory *memory)
{
    cs_m68k_bus_backend base;
    if(out==NULL)return CS_PG_ARGUMENT;
    cs_pg_result result=physical(&base,memory);
    if(result!=CS_PG_OK)return result;
    out->context=memory;out->context_bytes=sizeof *memory;out->validate=validate;
    out->span=span;out->read=read_ram;out->write=write_ram;
    return CS_PG_OK;
}
