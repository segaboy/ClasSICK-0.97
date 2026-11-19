/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mac128k-transfer.h"

static cs_guest_result copy_bytes(const void *context,uint32_t at,unsigned count,const uint8_t *source)
{
    const cs_pc286_m68k_memory *memory=context;
    cs_m68k_bus_backend scalar;
    if(source==NULL)return CS_GUEST_ARGUMENT;
    if(!count || count>64)return CS_GUEST_WIDTH;
    if(count>0x10000UL-(uint32_t)(uintptr_t)source)return CS_GUEST_ARGUMENT;
    if(cs_pc286_mac128k_backend(&scalar,memory)!=CS_PG_OK)return CS_GUEST_MAP;
    cs_guest_result admitted=scalar.span(context,at,count,CS_GUEST_WRITE);
    if(admitted!=CS_GUEST_OK)return admitted;
    unsigned first=count;
    if(at<0x400000UL){
        at&=0x1ffffUL;
        if(count>0x20000UL-at)first=(unsigned)(0x20000UL-at);
    }
    /* The full logical span and native source have been checked. Both pieces
     * have stable permissions and backing under the exclusive RAM contract. */
    if(cs_pc286_guest_write_bytes(&memory->guest,memory->generation,at,first,source)!=CS_PG_OK)
        return CS_GUEST_MAP;
    if(first<count && cs_pc286_guest_write_bytes(&memory->guest,memory->generation,
            0,count-first,source+first)!=CS_PG_OK)return CS_GUEST_MAP;
    return CS_GUEST_OK;
}

cs_pg_result cs_pc286_mac128k_transfer_backend(cs_m68k_transfer_backend *out,
    const cs_pc286_m68k_memory *memory)
{
    if(out==NULL)return CS_PG_ARGUMENT;
    cs_pg_result result=cs_pc286_mac128k_backend(&out->scalar,memory);
    if(result==CS_PG_OK)out->write_bytes=copy_bytes;
    return result;
}
