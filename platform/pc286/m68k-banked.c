/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-banked.h"

static cs_guest_result result(cs_pg_result code)
{
    switch (code) {
        case CS_PG_OK: return CS_GUEST_OK;
        case CS_PG_ARGUMENT: return CS_GUEST_ARGUMENT;
        case CS_PG_WIDTH: return CS_GUEST_WIDTH;
        case CS_PG_ADDRESS: return CS_GUEST_ADDRESS;
        case CS_PG_PERMISSION: return CS_GUEST_PERMISSION;
        default: return CS_GUEST_MAP;
    }
}
static cs_pg_result admitted(const cs_pc286_m68k_memory *m)
{
    cs_pg_result code;
    uint32_t begin, end, native;
    if (m == NULL) return CS_PG_ARGUMENT;
    code = cs_pc286_guest_validate(&m->guest, m->generation);
    if (code != CS_PG_OK) return code;
    begin = m->guest.arena_first; end = begin + m->guest.arena_bytes;
    native = (uint32_t)m->guest.cs << 4;
    if (begin < native + 0x10000UL && native < end) return CS_PG_BACKING;
    native = (uint32_t)m->guest.ds << 4;
    if (begin < native + 0x10000UL && native < end) return CS_PG_BACKING;
    return CS_PG_OK;
}
static cs_guest_result validate(const void *context)
{
    return result(admitted((const cs_pc286_m68k_memory *)context));
}
static cs_guest_result span(const void *context,uint32_t at,uint32_t bytes,uint32_t access)
{
    const cs_pc286_m68k_memory *m = context;
    cs_pg_result code = admitted(m);
    return result(code == CS_PG_OK ? cs_pc286_guest_span(&m->guest,m->generation,at,bytes,access) : code);
}
static cs_guest_result read_value(const void *context,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{
    const cs_pc286_m68k_memory *m = context;
    cs_pg_result code = admitted(m);
    return result(code == CS_PG_OK ? cs_pc286_guest_read(&m->guest,m->generation,at,width,access,out) : code);
}
static cs_guest_result write_value(const void *context,uint32_t at,unsigned width,uint32_t value)
{
    const cs_pc286_m68k_memory *m = context;
    cs_pg_result code = admitted(m);
    return result(code == CS_PG_OK ? cs_pc286_guest_write(&m->guest,m->generation,at,width,value) : code);
}
cs_pg_result cs_pc286_m68k_backend(cs_m68k_bus_backend *out,const cs_pc286_m68k_memory *memory)
{
    cs_pg_result code;
    if (out == NULL) return CS_PG_ARGUMENT;
    code = admitted(memory);
    if (code != CS_PG_OK) return code;
    out->context = memory; out->context_bytes = sizeof *memory;
    out->validate = validate; out->span = span; out->read = read_value; out->write = write_value;
    return CS_PG_OK;
}
