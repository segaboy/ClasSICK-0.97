/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../core/services/traps.h"
#include "../apps/service-probe/probe.h"
#define REQUIRE(e) do { if(!(e)) { fprintf(stderr,"line %d: %s\n",__LINE__,#e); exit(1); } } while(0)
static cs_trap_config config(void)
{
    cs_trap_config c={0x400000u,0x10000u,0x20000u,0x400100u,0x400120u,0x400140u}; return c;
}
static cs_trap_frame frame(void)
{
    cs_trap_frame f; memset(&f,0xCC,sizeof f);
    for(unsigned i=0;i<8;++i) { f.d[i]=UINT32_C(0x12340000)+i; f.a[i]=UINT32_C(0xABCDE000)+i; }
    f.pc=0x401000u; f.sr=0xA71Fu; return f;
}
static void decode(void)
{
    for(uint32_t w=0;w<65536u;++w) {
        cs_trap_word got,before; memset(&got,0x5A,sizeof got); before=got;
        cs_trap_result r=cs_trap_decode((uint16_t)w,&got);
        if(w<40960u || w>=45056u) { REQUIRE(r==CS_TRAP_NOT_TRAP); REQUIRE(memcmp(&got,&before,sizeof got)==0); }
        else {
            uint32_t ordinal=w-40960u; unsigned toolbox=ordinal>=2048u;
            REQUIRE(r==CS_TRAP_OK && got.word==w && got.toolbox==toolbox);
            REQUIRE(got.number==ordinal%(toolbox?512u:256u));
            REQUIRE(got.a0_result==(toolbox?0u:(ordinal/256u)%2u));
            REQUIRE(got.flags==(toolbox?0u:ordinal/512u));
            REQUIRE(got.auto_pop==(toolbox?(ordinal-2048u)/1024u:0u));
            REQUIRE(got.reserved==(toolbox?((ordinal-2048u)/512u)%2u:0u));
        }
    }
    REQUIRE(cs_trap_decode(0xA000u,NULL)==CS_TRAP_ARGUMENT);
}
static void directory(void)
{
    struct { uint32_t before[4]; cs_trap_gateway g; uint32_t after[4]; } guarded;
    memset(&guarded,0x5A,sizeof guarded); cs_trap_config c=config();
    REQUIRE(cs_trap_init(&guarded.g,&c)==CS_TRAP_OK);
    for(unsigned i=0;i<4;++i) REQUIRE(guarded.before[i]==0x5A5A5A5Au && guarded.after[i]==0x5A5A5A5Au);
    cs_trap_gateway independent=guarded.g;
    for(unsigned i=0;i<512;++i) {
        uint32_t value=0;
        REQUIRE(cs_trap_get(&guarded.g,(uint16_t)i,&value)==CS_TRAP_OK);
        REQUIRE(value==(i==70?c.get_token:(i==71?c.set_token:c.unsupported_token)));
        REQUIRE(cs_trap_set(&guarded.g,(uint16_t)i,0x10000u+i*2u)==CS_TRAP_OK);
    }
    for(unsigned i=0;i<512;++i) {
        uint32_t value=0; REQUIRE(cs_trap_get(&guarded.g,(uint16_t)i,&value)==CS_TRAP_OK);
        REQUIRE(value==0x10000u+i*2u);
        REQUIRE(independent.entries[i]==(i==70?c.get_token:(i==71?c.set_token:c.unsupported_token)));
    }
    REQUIRE(cs_trap_set(&guarded.g,511,c.rom_base+65534u)==CS_TRAP_OK);
    REQUIRE(cs_trap_set(&guarded.g,0,c.ram_end-2u)==CS_TRAP_OK);
    c.rom_base=0xFF0000u;c.get_token=0xFFFFFAu;c.set_token=0xFFFFFCu;c.unsupported_token=0xFFFFFEu;
    REQUIRE(cs_trap_init(&guarded.g,&c)==CS_TRAP_OK);
}
static void calls(void)
{
    cs_trap_gateway g; cs_trap_config c=config(); cs_trap_report r;
    REQUIRE(cs_trap_init(&g,&c)==CS_TRAP_OK);
    /* Intrinsics can be selected through another OS entry; target is frozen before Set. */
    REQUIRE(cs_trap_set(&g,10,c.get_token)==CS_TRAP_OK);
    REQUIRE(cs_trap_set(&g,11,c.set_token)==CS_TRAP_OK);
    for(unsigned n=0;n<512;++n) for(unsigned flag=0;flag<2;++flag) {
        cs_trap_frame f=frame(),expected=f;
        f.d[0]=UINT32_C(0xFFFF0000)+n;expected.d[0]=f.d[0];
        if(flag) expected.a[0]=g.entries[n];
        expected.sr=(uint16_t)(0xA710u+(n==0?4u:0u));
        REQUIRE(cs_trap_call(&g,(uint16_t)(0xA00Au+flag*256u),&f,&r)==CS_TRAP_OK);
        REQUIRE(memcmp(&f,&expected,sizeof f)==0 && r.target==c.get_token && r.pc==f.pc);
    }
    cs_trap_frame f=frame(); f.d[0]=70;f.a[0]=0x11000u;
    REQUIRE(cs_trap_call(&g,0xA00Bu,&f,&r)==CS_TRAP_OK && g.entries[70]==0x11000u);
    cs_trap_frame saved=f;
    REQUIRE(cs_trap_call(&g,0xA146u,&f,&r)==CS_TRAP_NEEDS_EXECUTION && r.target==0x11000u);
    REQUIRE(memcmp(&f,&saved,sizeof f)==0);
    f.a[0]=c.get_token;
    REQUIRE(cs_trap_direct(&g,c.set_token,&f,&r)==CS_TRAP_OK);
    f.a[0]=0;f.sr=0xA71Fu;
    REQUIRE(cs_trap_direct(&g,c.get_token,&f,&r)==CS_TRAP_OK && f.a[0]==c.get_token && f.sr==0xA71Fu);
    for(unsigned high=0;high<8;++high) {
        cs_trap_frame bad=frame(),unchanged=bad;cs_trap_gateway snapshot=g;
        cs_trap_result result=cs_trap_call(&g,(uint16_t)(0xA846u+high*256u),&bad,&r);
        REQUIRE(result==CS_TRAP_UNSUPPORTED_FORMAT && r.decoded.number==70u+((high%2u)*256u));
        REQUIRE(memcmp(&bad,&unchanged,sizeof bad)==0);
        REQUIRE(memcmp(&g,&snapshot,sizeof g)==0);
    }
    f=frame();saved=f;
    REQUIRE(cs_trap_call(&g,0xA346u,&f,&r)==CS_TRAP_UNSUPPORTED_FLAGS && r.decoded.flags==1);
    REQUIRE(memcmp(&f,&saved,sizeof f)==0);
}
static void failures(void)
{
    cs_trap_gateway g,old; cs_trap_config c=config();cs_trap_report r;
    REQUIRE(cs_trap_init(&g,&c)==CS_TRAP_OK);old=g;
    uint32_t value=0x12345678u;
    for(uint32_t n=512;n<65536u;++n) {
        REQUIRE(cs_trap_get(&g,(uint16_t)n,&value)==CS_TRAP_NUMBER && value==0x12345678u);
        REQUIRE(cs_trap_set(&g,(uint16_t)n,c.get_token)==CS_TRAP_NUMBER);
    }
    for(unsigned i=0;i<8;++i) {
        uint32_t addresses[]={0u,1u,0xFFFFu,0x20000u,0x3FFFFEu,0x410000u,0x1000000u,0xFFFFFFFFu};
        REQUIRE(cs_trap_set(&g,42,addresses[i])==CS_TRAP_ADDRESS);
    }
    REQUIRE(memcmp(&g,&old,sizeof g)==0);
    for(unsigned i=0;i<8;++i) {
        cs_trap_config bad=c;
        if(i==0)bad.rom_base=0;
        if(i==1)bad.rom_base=0xFF0002u;
        if(i==2)bad.ram_begin=c.rom_base;
        if(i==3)bad.ram_end=bad.ram_begin+65538u;
        if(i==4)bad.ram_begin=1;
        if(i==5)bad.ram_end=0x1000002u;
        if(i==6)bad.set_token=bad.get_token;
        if(i==7)bad.get_token|=1;
        REQUIRE(cs_trap_init(&g,&bad)==CS_TRAP_ADDRESS && memcmp(&g,&old,sizeof g)==0);
    }
    cs_trap_frame f=frame(),saved=f;
    REQUIRE(cs_trap_call(&g,0xA000u,&f,&r)==CS_TRAP_UNIMPLEMENTED && r.status==CS_TRAP_UNIMPLEMENTED);
    REQUIRE(cs_trap_call(&g,0xF146u,&f,&r)==CS_TRAP_NOT_TRAP && r.decoded.word==0xF146u);
    REQUIRE(memcmp(&f,&saved,sizeof f)==0 && memcmp(&g,&old,sizeof g)==0);
    f.d[0]=0xA146u;saved=f;
    REQUIRE(cs_trap_call(&g,0xA146u,&f,&r)==CS_TRAP_NUMBER && memcmp(&f,&saved,sizeof f)==0);
    REQUIRE(cs_trap_call(NULL,0xA146u,&f,&r)==CS_TRAP_ARGUMENT);
    REQUIRE(cs_trap_call(&g,0xA146u,NULL,&r)==CS_TRAP_ARGUMENT);
    REQUIRE(cs_trap_call(&g,0xA146u,&f,NULL)==CS_TRAP_ARGUMENT);
    g.magic=0;
    REQUIRE(cs_trap_get(&g,0,&value)==CS_TRAP_STATE);
    REQUIRE(cs_trap_call(&g,0xA146u,&f,&r)==CS_TRAP_STATE && memcmp(&f,&saved,sizeof f)==0);
}
int main(int argc,char **argv)
{
    REQUIRE(argc==2);
    if(strcmp(argv[1],"decode")==0)decode();
    else if(strcmp(argv[1],"directory")==0)directory();
    else if(strcmp(argv[1],"calls")==0)calls();
    else if(strcmp(argv[1],"failures")==0)failures();
    else if(strcmp(argv[1],"probe")==0) {
        cs_trap_gateway g;cs_service_probe_result r;
        REQUIRE(cs_service_probe(&g,&r)==1 && r.checks==10 && r.status==0 && r.signature==0x53564331u);
        REQUIRE(cs_service_probe(NULL,&r)==0 && cs_service_probe(&g,NULL)==0);
    } else REQUIRE(0);
    return 0;
}
