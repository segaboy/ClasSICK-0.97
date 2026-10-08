/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/native.h"
#include "acpi-fixture.h"
#include "scene-oracle.h"
#include <stdio.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
enum { W=320, H=200, PITCH=328, SPAN=PITCH*4*H, GUARD=64 };
static unsigned char map[2*48],trace[160],fb[SPAN+2*GUARD];
static _Alignas(16) unsigned char arena[CS_LOADER_ARENA_BYTES];
typedef struct { uint32_t value,step,mask,stop_after; unsigned long reads; } fake_port;
static uint32_t port_read(void *context,uint16_t port)
{
    fake_port *p=context;
    (void)port; ++p->reads;
    if(p->stop_after==0 || p->reads<p->stop_after) p->value=(p->value+p->step)&p->mask;
    return p->value;
}
static uint32_t get32(const unsigned char *p)
{
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static uint64_t get64(const unsigned char *p) { return get32(p)|((uint64_t)get32(p+4)<<32); }
static void descriptor(unsigned index,uint32_t type,uint64_t base,uint64_t pages)
{
    unsigned char *d=map+index*48;
    memset(d,0,48); fx_put(d,type,4); fx_put(d+8,base,8); fx_put(d+16,base,8); fx_put(d+24,pages,8);
}
static cs_native_memory mem;
static cs_native_devices setup(fake_port *p)
{
    cs_native_devices d;
    descriptor(0,9,FX_PHYS,8); descriptor(1,7,FX_PHYS+0x8000u,8);
    mem.map=map; mem.map_size=sizeof map; mem.map_stride=48; mem.map_version=1;
    mem.window_physical=FX_PHYS; mem.window_size=FX_SIZE; mem.window_base=(uintptr_t)fx;
    d.memory=&mem; d.port=port_read; d.port_context=p; d.framebuffer=fb+GUARD; d.arena=arena;
    memset(fb,0xA5,sizeof fb); memset(trace,0xE7,sizeof trace);
    return d;
}
static cs_uefi_handoff exited(void)
{
    cs_uefi_handoff h;
    memset(&h,0,sizeof h);
    h.stage=CS_UEFI_EXITED; h.rsdp=FX_PHYS+FX_RSDP; h.arena_size=CS_LOADER_ARENA_BYTES;
    h.framebuffer.base=UINT64_C(0x80000000); h.framebuffer.size=SPAN;
    h.framebuffer.width=W; h.framebuffer.height=H; h.framebuffer.pitch=PITCH; h.framebuffer.format=1;
    return h;
}
static int fb_untouched(void)
{
    for(size_t i=0;i<sizeof fb;++i) if(fb[i]!=0xA5) return 0;
    return 1;
}
static void expect(uint32_t result,uint32_t seconds)
{
    CHECK(get32(trace)==0x314C5043u && get32(trace+4)==1u);
    CHECK(get32(trace+8)==result && get32(trace+12)==seconds);
    for(unsigned i=48;i<sizeof trace;++i) CHECK(trace[i]==0xE7);
}
static int scene_matches(uint32_t step)
{
    for(long y=0;y<H;++y) for(long x=0;x<W;++x) {
        uint32_t c=oracle_scene(W,H,step,x,y);
        const unsigned char *q=fb+GUARD+(size_t)y*PITCH*4u+(size_t)x*4u;
        if(q[0]!=(unsigned char)(c>>8) || q[1]!=(unsigned char)(c>>16)
                || q[2]!=(unsigned char)(c>>24) || q[3]!=0) return 0;
    }
    for(unsigned i=0;i<GUARD;++i) if(fb[i]!=0xA5 || fb[GUARD+SPAN+i]!=0xA5) return 0;
    return 1;
}
static void gating(void)
{
    fake_port p={0,1000,0xFFFFFF,0,0};
    cs_native_devices d=setup(&p),bad;
    cs_uefi_handoff h=exited();
    fx_build(0,276);
    CHECK(cs_native_progress_loop(&h,1,&d,60,NULL)==CS_LOOP_ARGUMENT);
    for(unsigned i=0;i<sizeof trace;++i) CHECK(trace[i]==0xE7);
    CHECK(cs_native_progress_loop(NULL,1,&d,60,trace)==CS_LOOP_ARGUMENT); expect(1,60);
    CHECK(cs_native_progress_loop(&h,1,NULL,60,trace)==CS_LOOP_ARGUMENT);
    CHECK(cs_native_progress_loop(&h,1,&d,0,trace)==CS_LOOP_ARGUMENT); expect(1,0);
    CHECK(cs_native_progress_loop(&h,1,&d,3601,trace)==CS_LOOP_ARGUMENT);
    bad=d; bad.port=NULL; CHECK(cs_native_progress_loop(&h,1,&bad,60,trace)==CS_LOOP_ARGUMENT);
    bad=d; bad.framebuffer=NULL; CHECK(cs_native_progress_loop(&h,1,&bad,60,trace)==CS_LOOP_ARGUMENT);
    bad=d; bad.arena=NULL; CHECK(cs_native_progress_loop(&h,1,&bad,60,trace)==CS_LOOP_ARGUMENT);
    bad=d; bad.memory=NULL; CHECK(cs_native_progress_loop(&h,1,&bad,60,trace)==CS_LOOP_ARGUMENT);
    h.stage=CS_UEFI_FAILED; CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_NOT_EXITED);
    h=exited(); h.firmware_status=UINT64_C(0x8000000000000002);
    CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_NOT_EXITED); expect(2,60);
    h=exited(); CHECK(cs_native_progress_loop(&h,0,&d,60,trace)==CS_LOOP_NOT_READY); expect(3,60);
    h.rsdp=0; CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_TIMER); expect(4,60);
    h=exited(); fx_build(1u<<20,276);
    CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_TIMER);
    fx_build(0,276); h.framebuffer.format=2;
    CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_TARGET); expect(5,60);
    h=exited(); h.arena_size=4096;
    CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_ARENA); expect(6,60);
    CHECK(p.reads==0 && fb_untouched());
}
static void progress(void)
{
    /* 24- and 32-bit counters, with fine and coarse steps and wrapping start values. */
    static const struct { uint32_t flags,start,step,seconds,frames; } cases[]={
        {0,0xFFF000u,1000,60,17},{1u<<8,0xFFFFF000u,1000,60,17},
        {0,0,357954,60,17},{0,0,3579545,60,17},{0,12345,999,1,2},{0,0,500000,7,8}};
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];++i) {
        fake_port p={cases[i].start,cases[i].step,cases[i].flags!=0?0xFFFFFFFFu:0xFFFFFFu,0,0};
        cs_native_devices d=setup(&p);
        cs_uefi_handoff h=exited();
        uint64_t ticks;
        fx_build(cases[i].flags,276);
        CHECK(cs_native_progress_loop(&h,1,&d,cases[i].seconds,trace)==CS_LOOP_OK);
        expect(0,cases[i].seconds);
        ticks=(uint64_t)cases[i].step*(p.reads-1u);
        CHECK(get32(trace+16)==cases[i].seconds && get32(trace+16)==ticks/3579545u);
        CHECK(get32(trace+20)==(ticks%3579545u)*1000000000u/3579545u);
        CHECK(get32(trace+24)==cases[i].frames && get32(trace+44)==16);
        CHECK(get64(trace+32)==p.reads && get32(trace+40)==cases[i].step);
        /* Late samples count deltas above half of the counter period. */
        CHECK(get32(trace+28)==(cases[i].step>(p.mask/2u)?p.reads-1u:0u));
        CHECK(scene_matches(16));
    }
    /* A delta above half a 24-bit period is reported late, not silently trusted. */
    {
        fake_port p={0,0x900000u,0xFFFFFF,0,0};
        cs_native_devices d=setup(&p);
        cs_uefi_handoff h=exited();
        fx_build(0,276);
        CHECK(cs_native_progress_loop(&h,1,&d,3,trace)==CS_LOOP_OK);
        CHECK(get32(trace+28)==p.reads-1u && get32(trace+40)==0x900000u);
    }
}
static void failure(void)
{
    fake_port p={0,1000,0xFFFFFF,500,0};
    cs_native_devices d=setup(&p);
    cs_uefi_handoff h=exited();
    fx_build(0,276);
    CHECK(cs_native_progress_loop(&h,1,&d,60,trace)==CS_LOOP_STALLED); expect(9,60);
    CHECK(get64(trace+32)==499u+CS_LOOP_STALL_READS && p.reads==499u+CS_LOOP_STALL_READS);
    CHECK(get32(trace+24)==1 && get32(trace+44)==0 && scene_matches(0));
    {
        fake_port q={0xFFFF00u,1000,0x1FFFFFF,0,0};
        cs_native_devices e=setup(&q);
        CHECK(cs_native_progress_loop(&h,1,&e,60,trace)==CS_LOOP_VALUE); expect(8,60);
        CHECK(get32(trace+24)==1 && get64(trace+32)==1);
    }
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"gating")==0) gating();
    else if(strcmp(suite,"progress")==0) progress();
    else if(strcmp(suite,"failure")==0) failure();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("native loop %s: PASS\n",suite);
    return 0;
}
