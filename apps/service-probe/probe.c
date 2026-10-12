/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "probe.h"
int cs_service_probe(cs_trap_gateway *g,cs_service_probe_result *out)
{
    cs_trap_config c;
    c.rom_base=0x400000u; c.ram_begin=0x10000u; c.ram_end=0x18000u;
    c.get_token=0x400100u; c.set_token=0x400120u; c.unsupported_token=0x400140u;
    cs_trap_frame f; cs_trap_report r;
    if(g==0 || out==0) return 0;
    out->checks=0; out->status=1; out->signature=0;
    for(unsigned i=0;i<8;++i) { f.d[i]=0x100u+i; f.a[i]=0x200u+i; }
    f.pc=0x400200u; f.sr=0x271Fu;
#define CHECK(expression) do { if(!(expression)) return 0; ++out->checks; } while(0)
    CHECK(cs_trap_init(g,&c)==CS_TRAP_OK);
    f.d[0]=0xCAFE0046u;
    CHECK(cs_trap_call(g,0xA146u,&f,&r)==CS_TRAP_OK && f.a[0]==c.get_token && f.sr==0x2710u);
    f.d[0]=42; f.a[0]=0x11000u;
    CHECK(cs_trap_call(g,0xA047u,&f,&r)==CS_TRAP_OK);
    f.a[0]=0x2222u;
    CHECK(cs_trap_call(g,0xA146u,&f,&r)==CS_TRAP_OK && f.a[0]==0x11000u);
    CHECK(cs_trap_call(g,0xA02Au,&f,&r)==CS_TRAP_NEEDS_EXECUTION && r.target==0x11000u);
    f.d[0]=0x46; f.a[0]=0x11000u;
    CHECK(cs_trap_call(g,0xA047u,&f,&r)==CS_TRAP_OK);
    CHECK(cs_trap_call(g,0xA146u,&f,&r)==CS_TRAP_NEEDS_EXECUTION);
    f.a[0]=c.get_token;
    CHECK(cs_trap_direct(g,c.set_token,&f,&r)==CS_TRAP_OK);
    f.a[0]=0x2222u;
    CHECK(cs_trap_call(g,0xA146u,&f,&r)==CS_TRAP_OK && f.a[0]==c.get_token);
    CHECK(f.d[1]==0x101u && f.a[1]==0x201u && f.a[7]==0x207u && f.pc==0x400200u);
#undef CHECK
    out->status=0; out->signature=UINT32_C(0x53564331); return 1;
}
