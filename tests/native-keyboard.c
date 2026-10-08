/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/keyboard.h"
#include "acpi-fixture.h"
#include "ps2-fixture.h"
#include "scene-oracle.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
enum { W=320,H=200,PITCH=328,SPAN=PITCH*4*H,GUARD=64 };
static unsigned char map[48],trace[160],fb[SPAN+2*GUARD];
static void *arena;
static cs_native_memory mem;
static ps2_fixture controller;
typedef struct { uint32_t value,mask,live_step; uint64_t reads; unsigned stop; } timer_fixture;
static uint32_t timer_read(void *context,uint16_t port)
{
    timer_fixture *p=context; CHECK(port==0x608); ++p->reads;
    if(!p->stop) p->value=(p->value+(controller.live?p->live_step:1000u))&p->mask;
    return p->value;
}
static uint32_t get32(const unsigned char *p)
{ return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static uint64_t get64(const unsigned char *p) { return get32(p)|((uint64_t)get32(p+4)<<32); }
static cs_native_devices setup(timer_fixture *p)
{
    cs_native_devices d;
    memset(map,0,sizeof map); fx_put(map,9,4); fx_put(map+8,FX_PHYS,8); fx_put(map+16,FX_PHYS,8); fx_put(map+24,16,8);
    mem.map=map; mem.map_size=sizeof map; mem.map_stride=48; mem.map_version=1;
    mem.window_physical=FX_PHYS; mem.window_size=FX_SIZE; mem.window_base=(uintptr_t)fx;
    d.memory=&mem; d.port=timer_read; d.port_context=p; d.framebuffer=fb+GUARD; d.arena=arena;
    memset(fb,0xA5,sizeof fb); memset(trace,0xE7,sizeof trace); pf_init(&controller);
    fx_build(p->mask==UINT32_MAX?1u<<8:0,276); fx[FX_FADT+8]=6; fx[FX_FADT+109]=2; fx_seal(fx+FX_FADT);
    return d;
}
static cs_uefi_handoff exited(void)
{
    cs_uefi_handoff h; memset(&h,0,sizeof h);
    h.stage=CS_UEFI_EXITED; h.rsdp=FX_PHYS+FX_RSDP; h.arena_size=CS_LOADER_ARENA_BYTES;
    h.framebuffer.base=UINT64_C(0x80000000); h.framebuffer.size=SPAN;
    h.framebuffer.width=W; h.framebuffer.height=H; h.framebuffer.pitch=PITCH; h.framebuffer.format=1;
    return h;
}
static int untouched(void) { for(size_t i=0;i<sizeof fb;++i) if(fb[i]!=0xA5) return 0; return 1; }
static void expect(uint32_t result)
{
    CHECK(get32(trace)==CS_KBD_TRACE_MAGIC && get32(trace+4)==1 && get32(trace+8)==result);
    for(unsigned i=112;i<sizeof trace;++i) CHECK(trace[i]==0xE7);
    CHECK(controller.violations==0);
}
static int matches(uint32_t step)
{
    for(long y=0;y<H;++y) for(long x=0;x<W;++x) {
        uint32_t c=oracle_scene(W,H,step,x,y);
        const unsigned char *q=fb+GUARD+(size_t)y*PITCH*4u+(size_t)x*4u;
        if(q[0]!=(unsigned char)(c>>8) || q[1]!=(unsigned char)(c>>16) || q[2]!=(unsigned char)(c>>24) || q[3]!=0) return 0;
    }
    for(unsigned i=0;i<GUARD;++i) if(fb[i]!=0xA5 || fb[GUARD+SPAN+i]!=0xA5) return 0;
    for(unsigned y=0;y<H;++y) for(unsigned x=W*4;x<PITCH*4;++x) if(fb[GUARD+y*PITCH*4+x]!=0xA5) return 0;
    return 1;
}
static void gating(void)
{
    timer_fixture p={0,0xFFFFFF,1000,0,0}; cs_native_devices d=setup(&p),bad=d;
    cs_ps2_io io=pf_io(&controller),no=io; cs_uefi_handoff h=exited();
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,NULL)==1);
    CHECK(cs_native_keyboard_loop(NULL,1,&d,&io,60,trace)==1); expect(1);
    CHECK(cs_native_keyboard_loop(&h,1,NULL,&io,60,trace)==1);
    CHECK(cs_native_keyboard_loop(&h,1,&d,NULL,60,trace)==1);
    no.write=NULL; CHECK(cs_native_keyboard_loop(&h,1,&d,&no,60,trace)==1);
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,0,trace)==1);
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,3601,trace)==1);
    bad.port=NULL; CHECK(cs_native_keyboard_loop(&h,1,&bad,&io,60,trace)==1);
    h.stage=CS_UEFI_FAILED; CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==2);
    h=exited(); h.firmware_status=1; CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==2);
    h=exited(); CHECK(cs_native_keyboard_loop(&h,0,&d,&io,60,trace)==3);
    h.rsdp=0; CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==4);
    h=exited(); fx[FX_FADT+109]=0; fx_seal(fx+FX_FADT);
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==10); expect(10);
    fx[FX_FADT+109]=2; fx[FX_FADT+8]=1; fx_seal(fx+FX_FADT);
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==10);
    fx[FX_FADT+8]=6; fx_seal(fx+FX_FADT); h.framebuffer.format=2;
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==5);
    h=exited(); h.arena_size=4096; CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==6);
    CHECK(p.reads==0 && controller.status_reads==0 && controller.writes==0 && untouched());
}
static void interactive(void)
{
    /* Independent scene oracle and literal events: repeat does not repeat the action. */
    static const uint8_t bytes[]={0x29,0x29,0xF0,0x29,0x29,0xF0,0x29,0x76};
    timer_fixture p={0xFFF000u,0xFFFFFF,1000,0,0}; cs_native_devices d=setup(&p);
    cs_uefi_handoff h=exited(); cs_ps2_io io=pf_io(&controller);
    memcpy(controller.live_bytes,bytes,sizeof bytes); controller.live_count=sizeof bytes;
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==CS_KBD_ESCAPED); expect(12);
    CHECK(get32(trace+24)==0 && get32(trace+36)==2 && get32(trace+40)==1 && get32(trace+44)==6);
    CHECK(get32(trace+84)==2 && get32(trace+88)==1 && get32(trace+92)==6);
    CHECK(get32(trace+76)==3 && get32(trace+32)==4 && matches(3));
    CHECK(get64(trace+96)==p.reads && get32(trace+108)==0x34);
    for(unsigned width=0;width<2;++width) {
        timer_fixture q={width?0xFFFFF000u:0xFFF000u,width?UINT32_MAX:0xFFFFFFu,3579545,0,0};
        cs_native_devices e=setup(&q); io=pf_io(&controller);
        CHECK(cs_native_keyboard_loop(&h,1,&e,&io,60,trace)==0); expect(0);
        CHECK(get32(trace+24)==60 && get32(trace+28)==0 && get32(trace+32)==17 && matches(16));
        CHECK(get32(trace+44)==0 && get64(trace+96)==q.reads);
    }
    /* A Space offset remains visible after a completed run. */
    p.value=0; p.reads=0; p.live_step=3579545; d=setup(&p); io=pf_io(&controller);
    controller.live_bytes[0]=0x29; controller.live_count=1;
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==0);
    CHECK(get32(trace+36)==1 && get32(trace+76)==0 && matches(0));
}
static void failure(void)
{
    timer_fixture p={0,0xFFFFFF,1000,0,0}; cs_native_devices d=setup(&p);
    cs_uefi_handoff h=exited(); cs_ps2_io io=pf_io(&controller);
    controller.self_test=0xFC;
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==11); expect(11);
    CHECK(get32(trace+12)==CS_PS2_CONTROLLER && get32(trace+32)==0 && untouched());
    d=setup(&p); controller.blocked=1; p.reads=0;
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==11);
    CHECK(get32(trace+12)==CS_PS2_TIMEOUT && controller.writes==0 && untouched());
    d=setup(&p); p.reads=0; p.stop=1;
    CHECK(cs_native_keyboard_loop(&h,1,&d,&io,60,trace)==CS_KBD_STALLED); expect(9);
    CHECK(get32(trace+32)==1 && matches(0));
}
int main(int argc,char **argv)
{
    const char *s=argc>1?argv[1]:""; arena=malloc(CS_LOADER_ARENA_BYTES);
    if(!arena) return 2;
    if(strcmp(s,"gating")==0) gating(); else if(strcmp(s,"interactive")==0) interactive();
    else if(strcmp(s,"failure")==0) failure(); else { free(arena); return 2; }
    free(arena);
    if(failures) return 1;
    printf("native keyboard %s: PASS\n",s); return 0;
}
