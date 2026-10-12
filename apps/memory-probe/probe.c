/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "probe.h"
#include "program.h"
static const uint8_t program[]={CS_MEMORY_PROGRAM_BYTES,0x4E,0x71};
int cs_memory_probe(cs_trap_gateway *g,cs_memory_probe_result *out)
{
    if(g==0 || out==0) return 0;
    out->checks=0;out->status=1;out->signature=0;out->steps=0;out->subroutines=0;
    cs_trap_frame f;uint8_t data[64],stack[32];cs_guest_region regions[3];
    cs_guest_map map;cs_m68k_bus_report report;
    for(unsigned i=0;i<8;++i) {f.d[i]=0x100u+i;f.a[i]=0x200u+i;}
    for(unsigned i=0;i<64;++i) data[i]=0xCC;
    for(unsigned i=0;i<32;++i) stack[i]=0xCC;
    stack[16]=0;stack[17]=0x40;stack[18]=0x10;stack[19]=0x24;
    f.pc=0x401000;f.a[7]=0x300010;f.sr=0x271F;
    regions[0].bytes=program;regions[0].writable=0;regions[0].base=f.pc;
    regions[0].size=sizeof program;regions[0].flags=CS_GUEST_READ|CS_GUEST_EXECUTE;
    regions[1].bytes=data;regions[1].writable=data;regions[1].base=0x1800;
    regions[1].size=sizeof data;regions[1].flags=CS_GUEST_READ|CS_GUEST_WRITE;
    regions[2].bytes=stack;regions[2].writable=stack;regions[2].base=0x300000;
    regions[2].size=sizeof stack;regions[2].flags=CS_GUEST_READ|CS_GUEST_WRITE;
    map.regions=regions;map.count=3;
#define CHECK(e) do {if(!(e)) return 0;++out->checks;} while(0)
    for(unsigned i=0;i<12;++i) {
        CHECK(cs_m68k_bus_step(&map,g,&f,&report)==CS_M68K_OK);
        ++out->steps;out->subroutines+=report.subroutine;
    }
    CHECK(f.pc==0x401024 && f.a[7]==0x300014 && out->subroutines==1 && f.sr==0x2710 &&
        f.d[0]==0x100 && f.d[1]==0x11223344 && f.d[2]==0x7F5A && f.d[3]==0x11227F5A && f.a[0]==0x1800);
    int intact=1;
    for(unsigned i=4;i<8;++i) if(f.d[i]!=0x100u+i) intact=0;
    for(unsigned i=1;i<7;++i) if(f.a[i]!=0x200u+i) intact=0;
    CHECK(intact);
    CHECK(data[0]==0x11 && data[1]==0x22 && data[2]==0x7F && data[3]==0x5A);
    intact=1;for(unsigned i=4;i<64;++i) if(data[i]!=0xCC) intact=0;
    for(unsigned i=0;i<32;++i) if((i<12 || i>=20) && stack[i]!=0xCC) intact=0;
    CHECK(intact && stack[12]==0 && stack[13]==0x40 && stack[14]==0x10 && stack[15]==0x18 &&
        stack[16]==0 && stack[17]==0x40 && stack[18]==0x10 && stack[19]==0x24);
#undef CHECK
    out->status=0;out->signature=UINT32_C(0x4D454D31);return 1;
}
