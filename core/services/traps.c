/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "traps.h"
#define CS_TRAP_MAGIC UINT32_C(0x43535431)
static int rom_address(const cs_trap_config *c,uint32_t value)
{
    return value>=c->rom_base && value-c->rom_base<UINT32_C(65536) && (value&1u)==0;
}
static int config_valid(const cs_trap_config *c)
{
    return c->rom_base!=0 && (c->rom_base&1u)==0 && c->rom_base<=UINT32_C(0xFF0000) &&
        c->ram_begin!=0 && (c->ram_begin&1u)==0 && (c->ram_end&1u)==0 &&
        c->ram_begin<c->ram_end && c->ram_end<=UINT32_C(0x1000000) &&
        c->ram_end-c->ram_begin<=UINT32_C(65536) &&
        (c->ram_end<=c->rom_base || c->ram_begin>=c->rom_base+UINT32_C(65536)) &&
        rom_address(c,c->get_token) && rom_address(c,c->set_token) && rom_address(c,c->unsupported_token) &&
        c->get_token!=c->set_token && c->get_token!=c->unsupported_token && c->set_token!=c->unsupported_token;
}
static int state_valid(const cs_trap_gateway *g)
{
    return g->magic==CS_TRAP_MAGIC && config_valid(&g->config);
}
static int executable(const cs_trap_config *c,uint32_t value)
{
    return (value&1u)==0 && (rom_address(c,value) || (value>=c->ram_begin && value<c->ram_end));
}
cs_trap_result cs_trap_decode(uint16_t word,cs_trap_word *out)
{
    if(out==0) return CS_TRAP_ARGUMENT;
    if((word&0xF000u)!=0xA000u) return CS_TRAP_NOT_TRAP;
    out->word=word; out->toolbox=(uint16_t)((word>>11)&1u);
    out->number=(uint16_t)(word&(out->toolbox?0x1FFu:0xFFu));
    out->a0_result=(uint16_t)(out->toolbox?0u:((word>>8)&1u));
    out->auto_pop=(uint16_t)(out->toolbox?((word>>10)&1u):0u);
    out->reserved=(uint16_t)(out->toolbox?((word>>9)&1u):0u);
    out->flags=(uint16_t)(out->toolbox?0u:((word>>9)&3u));
    return CS_TRAP_OK;
}
cs_trap_result cs_trap_init(cs_trap_gateway *out,const cs_trap_config *config)
{
    if(out==0 || config==0) return CS_TRAP_ARGUMENT;
    if(!config_valid(config)) return CS_TRAP_ADDRESS;
    out->config.rom_base=config->rom_base; out->config.ram_begin=config->ram_begin;
    out->config.ram_end=config->ram_end; out->config.get_token=config->get_token;
    out->config.set_token=config->set_token; out->config.unsupported_token=config->unsupported_token;
    for(unsigned i=0;i<CS_TRAP_ENTRIES;++i) out->entries[i]=config->unsupported_token;
    out->entries[CS_TRAP_GET]=config->get_token; out->entries[CS_TRAP_SET]=config->set_token;
    out->magic=CS_TRAP_MAGIC;
    return CS_TRAP_OK;
}
cs_trap_result cs_trap_get(const cs_trap_gateway *gateway,uint16_t number,uint32_t *out)
{
    if(gateway==0 || out==0) return CS_TRAP_ARGUMENT;
    if(!state_valid(gateway)) return CS_TRAP_STATE;
    if(number>=CS_TRAP_ENTRIES) return CS_TRAP_NUMBER;
    if(!executable(&gateway->config,gateway->entries[number])) return CS_TRAP_STATE;
    *out=gateway->entries[number]; return CS_TRAP_OK;
}
cs_trap_result cs_trap_set(cs_trap_gateway *gateway,uint16_t number,uint32_t address)
{
    if(gateway==0) return CS_TRAP_ARGUMENT;
    if(!state_valid(gateway)) return CS_TRAP_STATE;
    if(number>=CS_TRAP_ENTRIES) return CS_TRAP_NUMBER;
    if(!executable(&gateway->config,address)) return CS_TRAP_ADDRESS;
    gateway->entries[number]=address; return CS_TRAP_OK;
}
static cs_trap_result intrinsic(cs_trap_gateway *g,uint32_t target,cs_trap_frame *frame,int pass_a0)
{
    uint16_t number=(uint16_t)frame->d[0];
    if(target==g->config.get_token) {
        uint32_t value=0; cs_trap_result result=cs_trap_get(g,number,&value);
        if(result==CS_TRAP_OK && pass_a0) frame->a[0]=value;
        return result;
    }
    if(target==g->config.set_token) return cs_trap_set(g,number,frame->a[0]);
    return target==g->config.unsupported_token?CS_TRAP_UNIMPLEMENTED:CS_TRAP_NEEDS_EXECUTION;
}
static void begin_report(cs_trap_report *report,const cs_trap_frame *frame)
{
    report->status=CS_TRAP_OK; report->decoded.word=0; report->decoded.number=0;
    report->decoded.toolbox=0; report->decoded.a0_result=0; report->decoded.auto_pop=0;
    report->decoded.reserved=0; report->decoded.flags=0; report->target=0; report->pc=frame->pc;
}
cs_trap_result cs_trap_call(cs_trap_gateway *gateway,uint16_t word,cs_trap_frame *frame,cs_trap_report *report)
{
    if(gateway==0 || frame==0 || report==0) return CS_TRAP_ARGUMENT;
    begin_report(report,frame); report->decoded.word=word;
    cs_trap_result result=cs_trap_decode(word,&report->decoded);
    if(result==CS_TRAP_OK && !state_valid(gateway)) result=CS_TRAP_STATE;
    if(result==CS_TRAP_OK) result=cs_trap_get(gateway,report->decoded.number,&report->target);
    if(result==CS_TRAP_OK && report->decoded.toolbox) result=CS_TRAP_UNSUPPORTED_FORMAT;
    if(result==CS_TRAP_OK && report->decoded.flags) result=CS_TRAP_UNSUPPORTED_FLAGS;
    if(result==CS_TRAP_OK) result=intrinsic(gateway,report->target,frame,report->decoded.a0_result!=0);
    if(result==CS_TRAP_OK) {
        uint16_t low=(uint16_t)frame->d[0];
        frame->sr=(uint16_t)((frame->sr&0xFFF0u)|(low==0?4u:((low&0x8000u)?8u:0u)));
    }
    report->status=result; return result;
}
cs_trap_result cs_trap_direct(cs_trap_gateway *gateway,uint32_t target,cs_trap_frame *frame,cs_trap_report *report)
{
    if(gateway==0 || frame==0 || report==0) return CS_TRAP_ARGUMENT;
    begin_report(report,frame); report->target=target;
    cs_trap_result result=!state_valid(gateway)?CS_TRAP_STATE:
        (!executable(&gateway->config,target)?CS_TRAP_ADDRESS:intrinsic(gateway,target,frame,1));
    report->status=result; return result;
}
