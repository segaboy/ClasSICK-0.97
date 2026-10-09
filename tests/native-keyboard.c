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
/* SPEC-0016: qualification record, bit-20 probe and output-port preservation.
   The partner Q=FX_PHYS+0xF000 lies in the synthetic window; P=Q^1 MiB. */
enum { Q_OFFSET=0xF000, Q_PHYS=FX_PHYS+Q_OFFSET };
static unsigned char map2[96];
static uint64_t qual_storage[32];
static uintptr_t shift;
static void shifting_write(void *context,uint16_t port,uint8_t value)
{
    pf_write(context,port,value);
    if(port==0x64 && value==0xAA && shift!=0) mem.window_base=shift;
}
static void ram_map(uint32_t type,uint64_t attribute)
{
    memset(map2,0,sizeof map2);
    fx_put(map2,9,4); fx_put(map2+8,FX_PHYS,8); fx_put(map2+16,FX_PHYS,8); fx_put(map2+24,8,8);
    fx_put(map2+48,type,4); fx_put(map2+56,FX_PHYS+0x8000u,8); fx_put(map2+64,FX_PHYS+0x8000u,8);
    fx_put(map2+72,8,8); fx_put(map2+80,attribute,8);
    mem.map=map2; mem.map_size=sizeof map2;
}
static cs_uefi_handoff qualified(void)
{
    cs_uefi_handoff h=exited();
    const char *name="Original test firmware";
    h.trace_base=((uint64_t)Q_PHYS^(UINT64_C(1)<<20))-CS_QUAL_CELL_OFFSET;
    h.image_base=UINT64_C(0x10000000); h.bundle_base=UINT64_C(0x1FE80000); h.stack_top=UINT64_C(0x1FEC1000);
    h.firmware_revision=0x00010000u; h.firmware_vendor_state=CS_UEFI_VENDOR_COMPLETE;
    for(size_t i=0;name[i]!=0;++i) h.firmware_vendor[i]=(unsigned char)name[i];
    return h;
}
static void record_matches(const unsigned char *q,const cs_uefi_handoff *h,uint32_t result,uint32_t ps2,
    uint32_t before,uint32_t after,uint32_t type,uint64_t attribute)
{
    CHECK(get32(q)==CS_QUAL_TRACE_MAGIC && get32(q+4)==1 && get32(q+8)==result && get32(q+12)==ps2);
    CHECK(get32(q+24)==before && get32(q+28)==after && get32(q+32)==type && get64(q+56)==attribute);
    CHECK(get32(q+36)==h->firmware_revision && get64(q+40)==h->trace_base+CS_QUAL_CELL_OFFSET);
    CHECK(get64(q+48)==((h->trace_base+CS_QUAL_CELL_OFFSET)^(UINT64_C(1)<<20)));
    CHECK(get64(q+64)==h->image_base && get64(q+72)==h->bundle_base && get64(q+80)==h->stack_top);
    CHECK(get32(q+88)==h->firmware_vendor_state && get32(q+92)==0 && memcmp(q+96,h->firmware_vendor,32)==0);
}
static void qualification(void)
{
    timer_fixture p={0xFFF000u,0xFFFFFFu,3579545,0,0}; cs_native_devices d;
    unsigned char *qual=(unsigned char *)qual_storage,*cell=qual+CS_QUAL_TRACE_BYTES;
    cs_uefi_handoff h=qualified(); cs_ps2_io io;
    static const struct { uint32_t type; uint64_t attribute; uint64_t window; } absent[]={
        {3,8,FX_SIZE},{1,8,FX_SIZE},{6,8,FX_SIZE},{7,0,FX_SIZE},{7,8|(UINT64_C(1)<<63),FX_SIZE},{7,8,Q_OFFSET+4}};
    /* Eligible partner: both probes pass, the partner is never written, the cell is restored. */
    for(unsigned type=2;type<=7;type+=type==2?2u:3u) {
        d=setup(&p); ram_map(type,0xF); io=pf_io(&controller); memset(qual_storage,0xE7,sizeof qual_storage);
        CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,qual,NULL,NULL)==0); expect(0);
        record_matches(qual,&h,0,CS_PS2_READY,0,0,type,0xF);
        CHECK(get32(qual+16)==0xCF && get32(qual+20)==0xCF && matches(16));
        for(unsigned i=CS_QUAL_TRACE_BYTES;i<sizeof qual_storage;++i) CHECK(qual[i]==0xE7);
        for(unsigned i=0;i<8;++i) CHECK(fx[Q_OFFSET+i]==0xCC);
    }
    /* Ineligible type, attribute, runtime bit or window: UNAVAILABLE, run continues. */
    for(unsigned c=0;c<sizeof absent/sizeof absent[0];++c) {
        d=setup(&p); ram_map(absent[c].type,absent[c].attribute); mem.window_size=absent[c].window;
        io=pf_io(&controller); memset(qual_storage,0xE7,sizeof qual_storage);
        CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,qual,NULL,NULL)==0);
        record_matches(qual,&h,0,CS_PS2_READY,2,2,absent[c].type,absent[c].attribute);
        for(unsigned i=0;i<8;++i) CHECK(cell[i]==0xE7);
    }
    /* ACPI reclaim memory is never a partner; an unmapped partner records no type. */
    d=setup(&p); io=pf_io(&controller); memset(qual_storage,0xE7,sizeof qual_storage);
    CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,qual,NULL,NULL)==0);
    record_matches(qual,&h,0,CS_PS2_READY,2,2,9,0);
    { cs_uefi_handoff low=exited(); d=setup(&p); io=pf_io(&controller);
      CHECK(cs_native_keyboard_observed(&low,1,&d,&io,60,trace,qual,NULL,NULL)==0);
      record_matches(qual,&low,0,CS_PS2_READY,2,2,UINT32_MAX,0); CHECK(get64(qual+48)==0x100480u); }
    /* Masking before startup: the partner is our own cell. Fail closed with no device I/O. */
    d=setup(&p); ram_map(7,8); io=pf_io(&controller);
    { unsigned char *inside=fx+Q_OFFSET-CS_QUAL_TRACE_BYTES; p.reads=0;
      CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,inside,NULL,NULL)==CS_KBD_MACHINE); expect(14);
      record_matches(inside,&h,CS_KBD_MACHINE,0,1,3,7,8);
      CHECK(controller.status_reads==0 && controller.writes==0 && untouched() && get32(trace+32)==0);
      for(unsigned i=0;i<8;++i) CHECK(fx[Q_OFFSET+i]==0xCC); }
    /* Masking appears after controller self-test: fail closed before drawing. */
    d=setup(&p); ram_map(7,8); io=pf_io(&controller); io.write=shifting_write;
    memset(qual_storage,0xE7,sizeof qual_storage); shift=(uintptr_t)cell-Q_OFFSET;
    CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,qual,NULL,NULL)==CS_KBD_MACHINE); expect(14);
    shift=0; record_matches(qual,&h,CS_KBD_MACHINE,CS_PS2_READY,0,1,7,8);
    CHECK(get32(trace+12)==CS_PS2_READY && get32(trace+32)==0 && untouched());
    for(unsigned i=0;i<8;++i) CHECK(cell[i]==0xE7);
    /* Self-test changes Gate A20/System Reset: PS/2 MACHINE maps to loop MACHINE. */
    d=setup(&p); ram_map(7,8); io=pf_io(&controller); controller.tested_port=0xCD;
    CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,qual,NULL,NULL)==CS_KBD_MACHINE); expect(14);
    record_matches(qual,&h,CS_KBD_MACHINE,CS_PS2_MACHINE,0,3,7,8);
    CHECK(get32(qual+16)==0xCF && get32(qual+20)==0xCD && get32(trace+12)==CS_PS2_MACHINE && untouched());
    /* A controller that never answers D0 still completes; the ports stay unread. */
    d=setup(&p); ram_map(7,8); io=pf_io(&controller); controller.silent_d0=1;
    { timer_fixture slow={0,0xFFFFFF,1000,0,0}; d.port_context=&slow;
      CHECK(cs_native_keyboard_observed(&h,1,&d,&io,1,trace,qual,NULL,NULL)==0);
      record_matches(qual,&h,0,CS_PS2_READY,0,0,7,8);
      CHECK(get32(qual+16)==CS_PS2_PORT_UNREAD && get32(qual+20)==CS_PS2_PORT_UNREAD); }
    /* Rejected gates still write an initialized record; a null trace writes nothing. */
    d=setup(&p); io=pf_io(&controller); memset(qual_storage,0xE7,sizeof qual_storage); p.reads=0;
    CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,NULL,qual,NULL,NULL)==1 && qual[0]==0xE7);
    CHECK(cs_native_keyboard_observed(NULL,1,&d,&io,60,trace,qual,NULL,NULL)==1);
    CHECK(get32(qual)==CS_QUAL_TRACE_MAGIC && get32(qual+8)==1 && get32(qual+16)==CS_PS2_PORT_UNREAD);
    CHECK(get32(qual+24)==3 && get32(qual+28)==3 && get64(qual+40)==0 && get64(qual+48)==0);
    CHECK(get32(qual+88)==CS_UEFI_VENDOR_ABSENT && qual[96]==0 && get32(qual+32)==UINT32_MAX);
    fx[FX_FADT+109]=0; fx_seal(fx+FX_FADT);
    CHECK(cs_native_keyboard_observed(&h,1,&d,&io,60,trace,qual,NULL,NULL)==10);
    record_matches(qual,&h,10,0,3,3,UINT32_MAX,0);
    CHECK(get32(qual+16)==CS_PS2_PORT_UNREAD && controller.writes==0 && p.reads==0 && untouched());
}
int main(int argc,char **argv)
{
    const char *s=argc>1?argv[1]:""; arena=malloc(CS_LOADER_ARENA_BYTES);
    if(!arena) return 2;
    if(strcmp(s,"gating")==0) gating(); else if(strcmp(s,"interactive")==0) interactive();
    else if(strcmp(s,"failure")==0) failure(); else if(strcmp(s,"qualification")==0) qualification();
    else { free(arena); return 2; }
    free(arena);
    if(failures) return 1;
    printf("native keyboard %s: PASS\n",s); return 0;
}
