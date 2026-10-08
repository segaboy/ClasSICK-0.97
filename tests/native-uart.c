/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/uart.h"
#include "acpi-fixture.h"
#include "ps2-fixture.h"
#include "uart-fixture.h"
#include "scene-oracle.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
enum { W=320,H=200,PITCH=328,SPAN=PITCH*4*H,GUARD=64 };
static unsigned char map[48],trace[CS_UART_TRACE_BYTES+2*GUARD],kt[160],fb[SPAN+2*GUARD];
static void *arena;
static cs_native_memory memory;
static ps2_fixture keyboard;
static uart_fixture serial;
typedef struct { uint32_t value,mask,step; uint64_t reads; unsigned stop,invalid,fast; } timer_fixture;
static uint32_t timer_read(void *context,uint16_t port)
{
    timer_fixture *p=context; CHECK(port==0x608); ++p->reads;
    if(!p->stop) p->value=(p->value+(p->fast && keyboard.live?200000u:p->step))&p->mask;
    return p->invalid && p->reads>2?0x1000000u:p->value;
}
static cs_native_devices setup(timer_fixture *p)
{
    cs_native_devices d;
    memset(map,0,sizeof map); fx_put(map,9,4); fx_put(map+8,FX_PHYS,8); fx_put(map+16,FX_PHYS,8); fx_put(map+24,16,8);
    memory.map=map; memory.map_size=48; memory.map_stride=48; memory.map_version=1;
    memory.window_physical=FX_PHYS; memory.window_size=FX_SIZE; memory.window_base=(uintptr_t)fx;
    d.memory=&memory; d.port=timer_read; d.port_context=p; d.framebuffer=fb+GUARD; d.arena=arena;
    memset(fb,0xA5,sizeof fb); memset(trace,0xE7,sizeof trace); memset(kt,0xD7,sizeof kt);
    pf_init(&keyboard); uf_init(&serial);
    fx_build(p->mask==UINT32_MAX?1u<<8:0,276); fx[FX_FADT+8]=6; fx[FX_FADT+109]=2; fx_seal(fx+FX_FADT);
    return d;
}
static cs_uefi_handoff exited(void)
{
    cs_uefi_handoff h; memset(&h,0,sizeof h); h.stage=CS_UEFI_EXITED;
    h.rsdp=FX_PHYS+FX_RSDP; h.arena_size=CS_LOADER_ARENA_BYTES;
    h.framebuffer.base=UINT64_C(0x80000000); h.framebuffer.size=SPAN;
    h.framebuffer.width=W; h.framebuffer.height=H; h.framebuffer.pitch=PITCH; h.framebuffer.format=1; return h;
}
static uint32_t get32(const unsigned char *p)
{ return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static uint32_t field(unsigned offset) { return get32(trace+GUARD+offset); }
static uint32_t run(cs_uefi_handoff *h,uint32_t ready,cs_native_devices *d,uint32_t seconds)
{
    cs_ps2_io k=pf_io(&keyboard); cs_uart_io u=uf_io(&serial);
    return cs_native_uart_loop(h,ready,d,&k,&u,0x3F8,12,seconds,kt,trace+GUARD);
}
static void guards(void)
{
    CHECK(field(0)==CS_UART_TRACE_MAGIC && field(4)==1);
    for(unsigned i=0;i<GUARD;++i) CHECK(trace[i]==0xE7 && trace[GUARD+CS_UART_TRACE_BYTES+i]==0xE7);
    for(unsigned i=88;i<128;++i) CHECK(trace[GUARD+i]==0);
    for(unsigned i=112;i<sizeof kt;++i) CHECK(kt[i]==0xD7);
    CHECK(serial.violations==0 && keyboard.violations==0);
}
static int matches(uint32_t step)
{
    for(long y=0;y<H;++y) for(long x=0;x<W;++x) {
        uint32_t c=oracle_scene(W,H,step,x,y); const unsigned char *q=fb+GUARD+(size_t)y*PITCH*4u+(size_t)x*4u;
        if(q[0]!=(unsigned char)(c>>8) || q[1]!=(unsigned char)(c>>16) || q[2]!=(unsigned char)(c>>24) || q[3]!=0) return 0;
    }
    for(unsigned i=0;i<GUARD;++i) if(fb[i]!=0xA5 || fb[GUARD+SPAN+i]!=0xA5) return 0;
    for(unsigned y=0;y<H;++y) for(unsigned x=W*4;x<PITCH*4;++x) if(fb[GUARD+y*PITCH*4+x]!=0xA5) return 0;
    return 1;
}
static void gating(void)
{
    timer_fixture p={0,0xFFFFFF,1000,0,0,0,0}; cs_native_devices d=setup(&p); cs_uefi_handoff h=exited();
    cs_ps2_io k=pf_io(&keyboard); cs_uart_io u=uf_io(&serial),bad=u;
    CHECK(cs_native_uart_loop(&h,1,&d,&k,&u,0x3F8,12,60,kt,NULL)==1);
    CHECK(run(NULL,1,&d,60)==1 && field(12)==9); guards();
    CHECK(run(&h,1,NULL,60)==1 && field(12)==9);
    bad.read=NULL; CHECK(cs_native_uart_loop(&h,1,&d,&k,&bad,0x3F8,12,60,kt,trace+GUARD)==1);
    CHECK(cs_native_uart_loop(&h,1,&d,&k,&u,65529,12,60,kt,trace+GUARD)==1);
    CHECK(cs_native_uart_loop(&h,1,&d,&k,&u,0x3F8,0,60,kt,trace+GUARD)==1);
    CHECK(run(&h,1,&d,0)==1 && run(&h,1,&d,3601)==1);
    h.stage=CS_UEFI_FAILED; CHECK(run(&h,1,&d,60)==2); h=exited(); h.firmware_status=1;
    CHECK(run(&h,1,&d,60)==2); h=exited(); CHECK(run(&h,0,&d,60)==3);
    h.rsdp=0; CHECK(run(&h,1,&d,60)==4); h=exited(); fx[FX_FADT+109]=0; fx_seal(fx+FX_FADT);
    CHECK(run(&h,1,&d,60)==10); fx[FX_FADT+109]=2; fx_seal(fx+FX_FADT);
    h.framebuffer.format=2; CHECK(run(&h,1,&d,60)==5); h=exited(); h.arena_size=1;
    CHECK(run(&h,1,&d,60)==6); guards(); CHECK(field(12)==9);
    CHECK(p.reads==0 && serial.operations==0 && keyboard.status_reads==0);
    for(unsigned i=0;i<sizeof fb;++i) CHECK(fb[i]==0xA5);
}
static void output(void)
{
    static const char expected[]="ClasSICK 0.97 UART v1\r\nkeyboard ready\r\n"
        "frame=00000000 sec=00000000 space=00000000\r\n"
        "frame=00000001 sec=00000000 space=00000001\r\n"
        "frame=00000002 sec=00000000 space=00000002\r\n"
        "frame=00000003 sec=00000000 space=00000002\r\n"
        "result=0000000C kbd=00000001\r\n";
    static const uint8_t bytes[]={0x29,0x29,0xF0,0x29,0x29,0xF0,0x29,0x76};
    timer_fixture p={0xFFF000,0xFFFFFF,1000,0,0,0,0}; cs_native_devices d=setup(&p); cs_uefi_handoff h=exited();
    memcpy(keyboard.live_bytes,bytes,sizeof bytes); keyboard.live_count=sizeof bytes;
    CHECK(run(&h,1,&d,60)==12); guards();
    CHECK(field(8)==12 && field(12)==2 && field(28)==0 && field(76)==1 && field(40)==0);
    CHECK(serial.output_count==sizeof expected-1 && memcmp(serial.output,expected,sizeof expected-1)==0);
    CHECK(memcmp(trace+GUARD+128,expected,sizeof expected-1)==0);
    CHECK(field(36)==serial.output_count && field(72)==7 && field(68)<CS_UART_POLLS);
    CHECK(p.reads==(uint64_t)field(64)+field(68)+2u);
    CHECK(get32(kt+36)==2 && get32(kt+40)==1 && get32(kt+24)==0 && matches(3));
    for(unsigned width=0;width<2;++width) {
        p.value=width?0xFFFFF000u:0xFFF000u; p.mask=width?UINT32_MAX:0xFFFFFFu; p.reads=0;
        d=setup(&p); CHECK(run(&h,1,&d,60)==0); guards();
        CHECK(field(12)==2 && field(40)==0 && get32(kt+24)==60 && get32(kt+32)==17 && matches(16));
        CHECK(p.reads==(uint64_t)field(64)+field(68)+2u);
        const char *end="frame=00000010 sec=0000003C space=00000000\r\nresult=00000000 kbd=00000001\r\n";
        size_t n=strlen(end); CHECK(serial.output_count>=n && memcmp(serial.output+serial.output_count-n,end,n)==0);
    }
}
static void failure(void)
{
    timer_fixture p={0,0xFFFFFF,1000,0,0,0,0}; cs_native_devices d=setup(&p); cs_uefi_handoff h=exited();
    serial.lsr=0; CHECK(run(&h,1,&d,1)==0); guards();
    CHECK(field(12)==5 && field(28)>0 && get32(kt+24)==1 && matches(16));
    d=setup(&p); serial.lsr=0x62; CHECK(run(&h,1,&d,1)==0 && field(12)==7 && field(52)==2); guards();
    d=setup(&p); serial.bad_offset=7; serial.bad_read=2;
    CHECK(run(&h,1,&d,1)==0 && field(12)==6 && serial.operations==7); guards();
    d=setup(&p); keyboard.live_bytes[0]=0x76; keyboard.live_count=1; serial.lsr=0;
    CHECK(run(&h,1,&d,60)==12 && field(12)==5 && field(68)<=CS_UART_POLLS); guards();
    d=setup(&p); keyboard.self_test=0xFC;
    CHECK(run(&h,1,&d,60)==11 && field(8)==11 && field(12)==2); guards();
    d=setup(&p); p.stop=1; keyboard.live_bytes[0]=0x76; keyboard.live_count=1; serial.lsr=0x20;
    CHECK(run(&h,1,&d,60)==12 && field(12)==5 && field(68)==CS_UART_POLLS); guards();
    p.stop=0; p.reads=0; d=setup(&p); p.invalid=1;
    CHECK(run(&h,1,&d,60)==8 && field(12)==4); guards();
    p.invalid=0; p.reads=0; p.fast=1; d=setup(&p);
    CHECK(run(&h,1,&d,1)==0 && field(12)==5); guards();
    CHECK(get32(kt+24)==1 && matches(16) && field(28)>0);
    p.reads=0; p.fast=0; p.step=100; d=setup(&p);
    for(unsigned i=0;i<21;++i) {
        keyboard.live_bytes[i*3]=0x29; keyboard.live_bytes[i*3+1]=0xF0; keyboard.live_bytes[i*3+2]=0x29;
    }
    keyboard.live_bytes[63]=0x76; keyboard.live_count=64;
    CHECK(run(&h,1,&d,60)==12 && field(12)==2 && field(40)>0 && field(44)>0); guards();
    CHECK(field(28)==0 && get32(kt+36)==21 && matches(5));
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:""; arena=malloc(CS_LOADER_ARENA_BYTES); if(!arena) return 2;
    if(strcmp(suite,"gating")==0) gating(); else if(strcmp(suite,"output")==0) output();
    else if(strcmp(suite,"failure")==0) failure(); else { free(arena); return 2; }
    free(arena); if(failures) return 1;
    printf("native UART %s: PASS\n",suite); return 0;
}
