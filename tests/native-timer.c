/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/native.h"
#include "../platform/pc/acpi.h"
#include "acpi-fixture.h"
#include <stdio.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static unsigned char map[3*48],trace[96];
typedef struct { uint32_t value,step,mask,high; unsigned long reads; uint16_t port; } fake_port;
static uint32_t port_read(void *context,uint16_t port)
{
    fake_port *p=context;
    p->port=port; ++p->reads;
    p->value=(p->value+p->step)&p->mask;
    return p->value|p->high;
}
static uint32_t get32(const unsigned char *p)
{
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void descriptor(unsigned index,uint32_t type,uint64_t base,uint64_t pages)
{
    unsigned char *d=map+index*48;
    memset(d,0,48); fx_put(d,type,4); fx_put(d+8,base,8); fx_put(d+16,base,8); fx_put(d+24,pages,8);
}
/* Window maps FX_PHYS.. to the fixture; first half ACPI reclaim, second conventional. */
static cs_native_memory memory(uint32_t acpi_type)
{
    cs_native_memory m;
    descriptor(0,acpi_type,FX_PHYS,8);
    descriptor(1,7,FX_PHYS+0x8000u,8);
    descriptor(2,0,0x1000,1);
    m.map=map; m.map_size=sizeof map; m.map_stride=48; m.map_version=1;
    m.window_physical=FX_PHYS; m.window_size=FX_SIZE; m.window_base=(uintptr_t)fx;
    return m;
}
static cs_uefi_handoff exited(void)
{
    cs_uefi_handoff h;
    memset(&h,0,sizeof h);
    h.stage=CS_UEFI_EXITED; h.rsdp=FX_PHYS+FX_RSDP;
    return h;
}
static void expect(uint32_t result,uint32_t acpi,uint32_t source,uint32_t bits)
{
    CHECK(get32(trace)==0x31545043u && get32(trace+4)==1u);
    CHECK(get32(trace+8)==result && get32(trace+12)==acpi);
    CHECK(get32(trace+16)==source && get32(trace+24)==bits);
    for(unsigned i=48;i<96;++i) CHECK(trace[i]==0xE7);
}
static void gating(void)
{
    cs_native_memory m=memory(9);
    fake_port p={0,7,0xFFFFFF,0,0,0};
    cs_uefi_handoff h;
    fx_build(0,276);
    for(uint32_t stage=0;stage<=4;++stage) {
        if(stage==CS_UEFI_EXITED) continue;
        memset(trace,0xE7,sizeof trace); h=exited(); h.stage=stage;
        CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_NOT_EXITED);
        expect(2,0,0,0);
    }
    memset(trace,0xE7,sizeof trace); h=exited(); h.firmware_status=UINT64_C(0x8000000000000002);
    CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_NOT_EXITED);
    memset(trace,0xE7,sizeof trace); h=exited();
    CHECK(cs_native_timer_probe(&h,0,&m,port_read,&p,trace)==CS_TIMER_NOT_READY); expect(3,0,0,0);
    memset(trace,0xE7,sizeof trace); h.rsdp=0;
    CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_NO_RSDP); expect(4,0,0,0);
    memset(trace,0xE7,sizeof trace); h=exited();
    CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,NULL)==CS_TIMER_ARGUMENT);
    for(unsigned i=0;i<sizeof trace;++i) CHECK(trace[i]==0xE7);
    CHECK(cs_native_timer_probe(NULL,1,&m,port_read,&p,trace)==CS_TIMER_ARGUMENT); expect(1,0,0,0);
    CHECK(cs_native_timer_probe(&h,1,NULL,port_read,&p,trace)==CS_TIMER_ARGUMENT);
    CHECK(cs_native_timer_probe(&h,1,&m,NULL,&p,trace)==CS_TIMER_ARGUMENT);
    CHECK(p.reads==0);
}
static void reader(void)
{
    cs_native_memory m=memory(9);
    unsigned char out[64];
    fx_build(0,276);
    CHECK(cs_native_read(&m,FX_PHYS+FX_RSDP,out,36)==1 && memcmp(out,"RSD PTR ",8)==0);
    CHECK(cs_native_read(&m,FX_PHYS,out,1)==1 && cs_native_read(&m,FX_PHYS+0x7FFFu,out,1)==1);
    memset(out,0x11,sizeof out);
    CHECK(cs_native_read(&m,FX_PHYS+0x7FFFu,out,2)==0);   /* straddles into conventional */
    CHECK(cs_native_read(&m,FX_PHYS+0x8000u,out,4)==0);   /* conventional memory refused */
    CHECK(cs_native_read(&m,FX_PHYS-1u,out,4)==0);        /* outside the window */
    CHECK(cs_native_read(&m,0x1000,out,4)==0);            /* reserved type, outside window */
    CHECK(cs_native_read(&m,FX_PHYS,out,0)==0);
    CHECK(cs_native_read(&m,UINT64_MAX-1u,out,4)==0);
    CHECK(cs_native_read(NULL,FX_PHYS,out,4)==0 && cs_native_read(&m,FX_PHYS,NULL,4)==0);
    for(unsigned i=0;i<sizeof out;++i) CHECK(out[i]==0x11);
    for(unsigned t=0;t<12;++t) {
        cs_native_memory n=memory(t);
        int expected=t==0 || t==6 || t==9 || t==10;
        CHECK(cs_native_read(&n,FX_PHYS+FX_FADT,out,8)==expected);
    }
    m.map_stride=32; CHECK(cs_native_read(&m,FX_PHYS,out,1)==0);
}
static void probe(void)
{
    cs_uefi_handoff h=exited();
    cs_native_memory m=memory(9);
    for(unsigned bits=24;bits<=32;bits+=8) {
        fake_port p={bits==24?0xFFFFF0u:0xFFFFFFF0u,7,bits==24?0xFFFFFFu:0xFFFFFFFFu,0,0,0};
        fx_build(bits==32?1u<<8:0u,276);
        memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_OK);
        expect(0,0,1,bits);
        CHECK(p.port==0x608 && get32(trace+20)==0x608);
        CHECK(get32(trace+28)==p.reads && p.reads==1u+(3580u+6u)/7u);
        CHECK(get32(trace+32)==7u*(p.reads-1u) && get32(trace+36)==0);
        CHECK(get32(trace+40)==0 && get32(trace+44)==(uint32_t)((uint64_t)get32(trace+32)*1000000000u/3579545u));
    }
    {
        fake_port p={0,0,0xFFFFFF,0,0,0};
        fx_build(0,276); memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_STALLED);
        expect(8,0,1,24); CHECK(p.reads==CS_TIMER_POLLS && get32(trace+28)==CS_TIMER_POLLS);
    }
    {
        fake_port p={0,7,0xFFFFFF,0x01000000u,0,0};
        fx_build(0,276); memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_VALUE);
        expect(7,0,1,24); CHECK(p.reads==1);
    }
    {
        fake_port p={0,7,0xFFFFFF,0,0,0};
        fx_build(1u<<20,276); memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_UNSUPPORTED);
        expect(6,CS_ACPI_UNSUPPORTED,0,0);
        fx_build(0,276); fx_seal_rsdp(); fx[FX_FADT+9]^=1; memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&m,port_read,&p,trace)==CS_TIMER_ACPI);
        expect(5,CS_ACPI_CHECKSUM,0,0);
        /* Tables in conventional memory are refused by the map-checked reader. */
        cs_native_memory c=memory(7);
        fx_build(0,276); memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&c,port_read,&p,trace)==CS_TIMER_ACPI);
        expect(5,CS_ACPI_ACCESS,0,0);
        c=memory(9); c.map_version=2; memset(trace,0xE7,sizeof trace);
        CHECK(cs_native_timer_probe(&h,1,&c,port_read,&p,trace)==CS_TIMER_ACPI);
        expect(5,CS_ACPI_ACCESS,0,0);
        CHECK(p.reads==0);
    }
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"gating")==0) gating();
    else if(strcmp(suite,"reader")==0) reader();
    else if(strcmp(suite,"probe")==0) probe();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("native timer %s: PASS\n",suite);
    return 0;
}
