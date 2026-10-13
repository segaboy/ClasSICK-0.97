/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "probe.h"
static int call(cs_mm_manager *m,uint16_t word,cs_trap_frame *f,cs_mm_register_report *r)
{return cs_mm_register_call(m,word,0,f,r)==CS_MM_OK && r->reply.code==0;}
int cs_heap_probe(cs_mm_manager *m,cs_heap_probe_result *out)
{
    if(m==0 || out==0) return 0;
    out->checks=0;out->status=1;out->signature=0;
    cs_trap_frame f;cs_mm_register_report r;uint32_t h,p,cell=0,value=0;
    for(unsigned i=0;i<8;++i) {f.d[i]=0x100u+i;f.a[i]=0x200u+i;}
    f.pc=0x401000;f.sr=0x271F;
#define CHECK(e) do {if(!(e)) return 0;++out->checks;} while(0)
    f.d[0]=16;CHECK(call(m,0xA322,&f,&r));h=f.a[0];
    CHECK(cs_guest_read(m->map,h,4,CS_GUEST_READ,&cell)==CS_GUEST_OK && cell!=0 && (cell&1u)==0);
    int zero=1;for(unsigned i=0;i<16;++i) {
        if(cs_guest_read(m->map,cell+i,1,CS_GUEST_READ,&value)!=CS_GUEST_OK || value!=0) zero=0;
    }
    CHECK(zero && f.sr==0x2714);
    f.a[0]=h;CHECK(call(m,0xA029,&f,&r));
    CHECK(cs_guest_read(m->map,h,4,CS_GUEST_READ,&value)==CS_GUEST_OK && value==(cell|CS_MM_LOCK));
    CHECK(call(m,0xA02A,&f,&r));
    CHECK(cs_guest_write(m->map,cell,4,0x12345678)==CS_GUEST_OK);
    f.d[0]=32;CHECK(call(m,0xA024,&f,&r));
    CHECK(cs_guest_read(m->map,h,4,CS_GUEST_READ,&cell)==CS_GUEST_OK &&
        cs_guest_read(m->map,cell,4,CS_GUEST_READ,&value)==CS_GUEST_OK && value==0x12345678);
    CHECK(call(m,0xA025,&f,&r) && f.d[0]==32 && f.a[0]==h);
    f.d[0]=16;CHECK(call(m,0xA71E,&f,&r));p=f.a[0];
    CHECK(p>=m->config.system.begin+CS_MM_CELL_BYTES && p<m->config.system.end);
    f.a[0]=cell;f.a[1]=p;f.d[0]=4;CHECK(call(m,0xA02E,&f,&r));
    CHECK(cs_guest_read(m->map,p,4,CS_GUEST_READ,&value)==CS_GUEST_OK && value==0x12345678);
    f.a[0]=p;CHECK(call(m,0xA021,&f,&r) && f.d[0]==16);
    f.d[0]=8;CHECK(call(m,0xA020,&f,&r));
    CHECK(call(m,0xA01F,&f,&r) && f.a[0]==p);
    f.a[0]=h;CHECK(call(m,0xA023,&f,&r) && f.a[0]==h);
    CHECK(cs_guest_read(m->map,h,4,CS_GUEST_READ,&value)==CS_GUEST_OK && value==0);
    int intact=1;
    for(unsigned i=1;i<8;++i) if(f.d[i]!=0x100u+i || (i>=2 && f.a[i]!=0x200u+i)) intact=0;
    CHECK(intact && f.a[1]==p && f.pc==0x401000 && f.a[7]==0x207 && f.sr==0x2714);
#undef CHECK
    out->status=0;out->signature=UINT32_C(0x48454131);return 1;
}
