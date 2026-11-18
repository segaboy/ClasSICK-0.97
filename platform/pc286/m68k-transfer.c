/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-transfer.h"
static cs_guest_result guest_result(cs_pg_result code)
{
    switch(code) {
        case CS_PG_OK:return CS_GUEST_OK;
        case CS_PG_ARGUMENT:return CS_GUEST_ARGUMENT;
        case CS_PG_WIDTH:return CS_GUEST_WIDTH;
        case CS_PG_ADDRESS:return CS_GUEST_ADDRESS;
        case CS_PG_PERMISSION:return CS_GUEST_PERMISSION;
        default:return CS_GUEST_MAP;
    }
}
static cs_guest_result write_bytes(const void *context,uint32_t at,unsigned bytes,const uint8_t *source)
{
    const cs_pc286_m68k_memory *memory=context;
    cs_m68k_bus_backend checked;
    cs_pg_result result=cs_pc286_m68k_backend(&checked,memory);
    if(result!=CS_PG_OK)return guest_result(result);
    return guest_result(cs_pc286_guest_write_bytes(&memory->guest,memory->generation,at,bytes,source));
}
cs_pg_result cs_pc286_m68k_transfer_backend(cs_m68k_transfer_backend *out,
    const cs_pc286_m68k_memory *memory)
{
    cs_m68k_bus_backend checked;cs_pg_result result;
    if(out==NULL)return CS_PG_ARGUMENT;
    result=cs_pc286_m68k_backend(&checked,memory);
    if(result!=CS_PG_OK)return result;
    out->scalar.context=checked.context;out->scalar.context_bytes=checked.context_bytes;
    out->scalar.validate=checked.validate;out->scalar.span=checked.span;
    out->scalar.read=checked.read;out->scalar.write=checked.write;out->write_bytes=write_bytes;
    return CS_PG_OK;
}
