/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "probe.h"
#include "program.h"
static const uint8_t program[]={CS_ALINE_PROGRAM_BYTES,0x4E,0x71};
int cs_aline_probe(cs_trap_gateway *g,cs_aline_probe_result *out)
{
    if(g==0 || out==0) return 0;
    out->checks=0;out->status=1;out->signature=0;out->steps=0;out->calls=0;
    cs_trap_frame f;uint8_t stack[32];cs_m68k_memory m;cs_m68k_report r;uint32_t value=0;
    for(unsigned i=0;i<8;++i) {f.d[i]=0x100u+i;f.a[i]=0x200u+i;}
    for(unsigned i=0;i<32;++i) stack[i]=0xCC;
    stack[16]=0;stack[17]=0x40;stack[18]=0x10;stack[19]=0x0C;
    f.pc=0x401000u;f.sr=0x271Fu;f.a[7]=0x300010u;
    m.code=program;m.code_base=f.pc;m.code_size=sizeof program;
    m.stack=stack;m.stack_base=0x300000u;m.stack_size=sizeof stack;
#define CHECK(e) do {if(!(e)) return 0;++out->checks;} while(0)
    CHECK(cs_trap_set(g,0x20,g->config.unsupported_token)==CS_TRAP_OK);
    for(unsigned i=0;i<6;++i) {
        CHECK(cs_m68k_step(&m,g,&f,&r)==CS_M68K_OK);
        ++out->steps;if(r.service_called) ++out->calls;
    }
    int preserved=1;
    for(unsigned i=1;i<8;++i) if(f.d[i]!=0x100u+i || (i<7 && f.a[i]!=0x200u+i)) preserved=0;
    CHECK(out->calls==3 && f.d[0]==32 && f.a[0]==g->config.get_token && f.sr==0x2710 &&
        f.pc==0x40100Cu && f.a[7]==0x300014u && preserved &&
        cs_trap_get(g,0x20,&value)==CS_TRAP_OK && value==g->config.get_token &&
        stack[10]==0x27 && stack[11]==0x10 && stack[12]==0 && stack[13]==0x40 &&
        stack[14]==0x10 && stack[15]==0x0A && stack[9]==0xCC && stack[20]==0xCC);
#undef CHECK
    out->status=0;out->signature=UINT32_C(0x414C4E31);return 1;
}
