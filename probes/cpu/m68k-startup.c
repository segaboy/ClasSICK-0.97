/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-startup.h"
#include "../../core/cpu/m68k-guard.h"
typedef struct {
    cs_trap_gateway gateway;cs_trap_frame frame,before;
    cs_m68k_bus_report report;cs_guest_region regions[2];cs_guest_map map;
    cs_m68k_stack_limits limits;uint8_t code[32],stack[64],saved[64];
} probe_state;
static probe_state state;
static int accept(const void *context,const cs_m68k_guard_event *event)
{(void)context;(void)event;return 1;}
static int no_commit(const void *context,const cs_m68k_guard_event *event)
{(void)context;return event->kind!=CS_M68K_GUARD_COMMIT;}
static int no_operand(const void *context,const cs_m68k_guard_event *event)
{(void)context;return event->kind!=CS_M68K_GUARD_READ && event->kind!=CS_M68K_GUARD_TARGET;}
static const cs_m68k_guard normal={accept,0,0},denied={no_commit,0,0},value_only={no_operand,0,0};
static void word(uint8_t *p,uint16_t value){p[0]=(uint8_t)(value>>8);p[1]=(uint8_t)value;}
static uint32_t longword(const uint8_t *p)
{return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static int reset(void)
{
    unsigned i;cs_trap_config config;
    config.rom_base=0x400000;config.ram_begin=0x10000;config.ram_end=0x20000;
    config.get_token=0x400100;config.set_token=0x400120;config.unsupported_token=0x400140;
    if(cs_trap_init(&state.gateway,&config)!=CS_TRAP_OK)return 0;
    for(i=0;i<32;i+=2)word(state.code+i,0x4e71);
    for(i=0;i<64;++i)state.stack[i]=0xa5;
    state.regions[0].bytes=state.code;state.regions[0].writable=0;
    state.regions[0].base=0x401000;state.regions[0].size=32;state.regions[0].flags=5;
    state.regions[1].bytes=state.stack;state.regions[1].writable=state.stack;
    state.regions[1].base=0x300000;state.regions[1].size=64;state.regions[1].flags=3;
    state.map.regions=state.regions;state.map.count=2;state.limits.base=0x300000;state.limits.size=64;
    for(i=0;i<8;++i){state.frame.d[i]=0;state.frame.a[i]=0x1800u+2u*i;}
    state.frame.a[7]=0x300020;state.frame.pc=0x401000;state.frame.sr=0x271f;
    return 1;
}
static cs_m68k_result step(const cs_m68k_guard *guard)
{return cs_m68k_bus_step_guarded(&state.map,&state.gateway,&state.frame,&state.report,&state.limits,guard);}
static void snapshot(void)
{
    unsigned i;for(i=0;i<8;++i){state.before.d[i]=state.frame.d[i];state.before.a[i]=state.frame.a[i];}
    state.before.pc=state.frame.pc;state.before.sr=state.frame.sr;
    for(i=0;i<64;++i)state.saved[i]=state.stack[i];
}
static int same(void)
{
    unsigned i;for(i=0;i<8;++i)if(state.before.d[i]!=state.frame.d[i] || state.before.a[i]!=state.frame.a[i])return 0;
    if(state.before.pc!=state.frame.pc || state.before.sr!=state.frame.sr)return 0;
    for(i=0;i<64;++i)if(state.saved[i]!=state.stack[i])return 0;
    return state.report.cpu.service_called==0;
}
unsigned cs_m68k_startup_probe(void)
{
    unsigned i;
    static const uint8_t program[]={0x4e,0x56,0xff,0xf8,0x41,0xee,0xff,0xfc,0x48,0x50,0x20,0x1f,0x4e,0x5e};
    if(!reset())return 0xe0;
    /* Address values do not cause operand reads, and preserve all CCR bits. */
    word(state.code,0x43f8);word(state.code+2,0xff81);
    if(step(&value_only)!=CS_M68K_OK || state.frame.a[1]!=UINT32_C(0xffffff81) || state.frame.sr!=0x271f)return 0xe1;
    if(!reset())return 0xe0;
    word(state.code,0x4857);
    if(step(&value_only)!=CS_M68K_OK || state.frame.a[7]!=0x30001c ||
        longword(state.stack+28)!=0x300020 || state.frame.sr!=0x271f || state.stack[27]!=0xa5 || state.stack[32]!=0xa5)return 0xe2;
    if(!reset())return 0xe0;
    for(i=0;i<sizeof program;++i)state.code[i]=program[i];
    for(i=0;i<5;++i)if(step(&normal)!=CS_M68K_OK)return 0xe3;
    if(state.frame.pc!=0x40100e || state.frame.a[0]!=0x300018 || state.frame.d[0]!=0x300018 ||
        state.frame.a[6]!=0x180c || state.frame.a[7]!=0x300020 || state.frame.sr!=0x2710 ||
        longword(state.stack+16)!=0x300018 || longword(state.stack+28)!=0x180c)return 0xe3;
    /* The staged push must survive neither final bounds nor COMMIT refusal. */
    if(!reset())return 0xe0;
    word(state.code,0x4e56);word(state.code+2,0xffd8);snapshot();
    if(step(&normal)!=CS_M68K_ADDRESS || !same())return 0xe4;
    word(state.code+2,0xfff8);snapshot();
    if(step(&denied)!=CS_M68K_POLICY || !same())return 0xe5;
    /* UNLK read may succeed before the final guard rejects its proposal. */
    state.frame.a[6]=0x300008;word(state.code,0x4e5e);snapshot();
    if(step(&denied)!=CS_M68K_POLICY || !same())return 0xe6;
    word(state.code,0x4e57);snapshot();
    if(step(&normal)!=CS_M68K_OPCODE || !same())return 0xe7;
    if(sizeof(size_t)<sizeof(uint32_t)) {
        /* A declared guest span cannot be silently narrowed to a near object. */
        state.regions[1].size=UINT32_C(0x10040);word(state.code,0x4e71);snapshot();
        if(step(&normal)!=CS_M68K_ARGUMENT || !same())return 0xe8;
    }
    return 0xb8;
}
