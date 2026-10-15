/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/cpu/m68k-guard.h"
#define REQUIRE(e) do {if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
typedef struct {
    uint8_t code[64];
    union { uint8_t b[256]; cs_m68k_bus_report r; uint64_t q; } data;
    uint8_t stack[64];
    cs_guest_region regions[3];cs_guest_map map;cs_trap_gateway gateway;cs_trap_frame frame;
    cs_m68k_bus_report report;cs_m68k_stack_limits limits;
} fixture;
typedef struct { cs_trap_frame f;cs_trap_gateway g;uint8_t c[64],d[256],s[64]; } snapshot;
/* Pure deny rule: immutable context, no logging, no counters. */
enum { MA=1u,MB=2u,MW=4u,MD=8u };
typedef struct { cs_m68k_guard_kind kind;unsigned flags;uint32_t address,bytes,d0;uint16_t word; } rule;
static int deny_rule(const void *context,const cs_m68k_guard_event *e)
{
    const rule *r=context;
    if(!r || !e || e->kind!=r->kind) return 1;
    if((r->flags&MA) && e->address!=r->address) return 1;
    if((r->flags&MB) && e->bytes!=r->bytes) return 1;
    if((r->flags&MW) && e->word!=r->word) return 1;
    if((r->flags&MD) && (!e->proposed || e->proposed->d[0]!=r->d0)) return 1;
    return 0;
}
static int permit_all(const void *context,const cs_m68k_guard_event *e) {(void)context;(void)e;return 1;}
static const cs_m68k_guard permit={permit_all,NULL,0};
static rule R(cs_m68k_guard_kind kind,unsigned flags,uint32_t address,uint32_t bytes,uint16_t word,uint32_t d0)
{rule r;memset(&r,0,sizeof r);r.kind=kind;r.flags=flags;r.address=address;r.bytes=bytes;r.word=word;r.d0=d0;return r;}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->data.b,0xCC,256);memset(x->stack,0xCC,64);
    for(unsigned i=0;i<32;++i) {x->code[i*2]=0x4E;x->code[i*2+1]=0x71;}
    x->regions[0]=(cs_guest_region){x->code,0,0x401000,64,5};
    x->regions[1]=(cs_guest_region){x->data.b,x->data.b,0x1800,256,3};
    x->regions[2]=(cs_guest_region){x->stack,x->stack,0x300000,64,3};
    x->map=(cs_guest_map){x->regions,3};
    cs_trap_config c={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&c)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i) {x->frame.d[i]=0xABCD0100u+i;x->frame.a[i]=0x1800;}
    x->frame.pc=0x401000;x->frame.a[7]=0x300020;x->frame.sr=0x2700;
    x->limits=(cs_m68k_stack_limits){0x300000,64};
}
static void word(uint8_t *p,uint16_t v) {p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void longword(uint8_t *p,uint32_t v) {word(p,(uint16_t)(v>>16));word(p+2,(uint16_t)v);}
static uint32_t get_long(const uint8_t *p)
{return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|(uint32_t)p[3];}
static unsigned size_field(unsigned width) {return width==1?1u:width==2?3u:2u;}
static uint16_t move_word(unsigned width,unsigned sm,unsigned sr,unsigned dm,unsigned dr)
{return (uint16_t)(size_field(width)*4096u+dr*512u+dm*64u+sm*8u+sr);}
static cs_m68k_result step(fixture *x,const cs_m68k_guard *g)
{return cs_m68k_bus_step_guarded(&x->map,&x->gateway,&x->frame,&x->report,&x->limits,g);}
static void take(const fixture *x,snapshot *s)
{
    memset(s,0,sizeof *s);s->f=x->frame;s->g=x->gateway;
    memcpy(s->c,x->code,64);memcpy(s->d,x->data.b,256);memcpy(s->s,x->stack,64);
}
static int same(const fixture *x,const snapshot *s)
{
    return memcmp(&s->f,&x->frame,sizeof s->f)==0 && memcmp(&s->g,&x->gateway,sizeof s->g)==0 &&
        memcmp(s->c,x->code,64)==0 && memcmp(s->d,x->data.b,256)==0 && memcmp(s->s,x->stack,64)==0;
}
/* Failure with no RAM/frame/gateway effect and no service call. */
static int blocked(fixture *x,const cs_m68k_guard *g,cs_m68k_result expected)
{
    snapshot s;take(x,&s);
    cs_m68k_result r=step(x,g);
    return r==expected && same(x,&s) && x->report.cpu.service_called==0;
}
static int denied(fixture *x,rule r)
{cs_m68k_guard g={deny_rule,&r,sizeof r};return blocked(x,&g,CS_M68K_POLICY);}
static cs_m68k_result with_rule(fixture *x,rule r)
{cs_m68k_guard g={deny_rule,&r,sizeof r};return step(x,&g);}
static int staged_state(const void *context,const cs_m68k_guard_event *e)
{
    (void)context;
    if(e->kind!=CS_M68K_GUARD_COMMIT) return 1;
    return !(e->before->a[0]==0x1800 && e->proposed->a[0]==0x1804 &&
        e->before->pc==0x401000 && e->proposed->pc==0x401002 &&
        e->before->sr==0x2700 && e->proposed->sr==0x2708);
}
static void events(void)
{
    fixture x;
    /* MOVEQ #1,D0: opcode, instruction class, commit. */
    setup(&x);word(x.code,0x7001);
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.d[0]==1 && x.frame.pc==0x401002 && x.frame.sr==0x2700);
    setup(&x);word(x.code,0x7001);REQUIRE(denied(&x,R(CS_M68K_GUARD_OPCODE,MA|MB,0x401000,2,0,0)));
    setup(&x);word(x.code,0x7001);REQUIRE(denied(&x,R(CS_M68K_GUARD_INSTRUCTION,MW,0,0,0x7001,0)));
    setup(&x);word(x.code,0x7001);
    REQUIRE(with_rule(&x,R(CS_M68K_GUARD_INSTRUCTION,MW,0,0,0x7002,0))==CS_M68K_OK && x.frame.d[0]==1);
    setup(&x);word(x.code,0x7001);REQUIRE(denied(&x,R(CS_M68K_GUARD_COMMIT,MD,0,0,0,1)));
    setup(&x);word(x.code,0x7001);REQUIRE(denied(&x,R(CS_M68K_GUARD_COMMIT,MA|MB,0x401002,0,0,0)));
    setup(&x);word(x.code,0x7002);
    REQUIRE(with_rule(&x,R(CS_M68K_GUARD_COMMIT,MD,0,0,0,1))==CS_M68K_OK && x.frame.d[0]==2);
    setup(&x);REQUIRE(denied(&x,R(CS_M68K_GUARD_INSTRUCTION,MW,0,0,0x4E71,0)));
    /* MOVE.L #imm,D0: four-byte immediate is one whole extension span. */
    setup(&x);word(x.code,move_word(4,7,4,0,0));longword(x.code+2,0x12345678);
    REQUIRE(denied(&x,R(CS_M68K_GUARD_EXTENSION,MA|MB,0x401002,4,0,0)));
    REQUIRE(with_rule(&x,R(CS_M68K_GUARD_EXTENSION,MB,0,2,0,0))==CS_M68K_OK &&
        x.frame.d[0]==0x12345678 && x.frame.pc==0x401006);
    /* MOVE.L abs.L,D0: absolute extension and source read. */
    setup(&x);word(x.code,move_word(4,7,1,0,0));longword(x.code+2,0x1800);longword(x.data.b,0x01020304);
    REQUIRE(denied(&x,R(CS_M68K_GUARD_EXTENSION,MA|MB,0x401002,4,0,0)));
    REQUIRE(denied(&x,R(CS_M68K_GUARD_READ,MA|MB,0x1800,4,0,0)));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.d[0]==0x01020304);
    /* MOVE.L D0,(A0): destination write. */
    setup(&x);word(x.code,move_word(4,0,0,2,0));x.frame.d[0]=0x11223344;
    REQUIRE(denied(&x,R(CS_M68K_GUARD_WRITE,MA|MB,0x1800,4,0,0)));
    REQUIRE(with_rule(&x,R(CS_M68K_GUARD_WRITE,MA,0x1804,0,0,0))==CS_M68K_OK && get_long(x.data.b)==0x11223344);
    /* Taken BRA.S; untaken BEQ.S produces no target denial. */
    setup(&x);word(x.code,0x6004);REQUIRE(denied(&x,R(CS_M68K_GUARD_TARGET,MA,0x401006,0,0,0)));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.pc==0x401006);
    setup(&x);word(x.code,0x6704);
    REQUIRE(with_rule(&x,R(CS_M68K_GUARD_TARGET,0,0,0,0,0))==CS_M68K_OK && x.frame.pc==0x401002);
    /* BSR.S target denial leaves stack and A7 unchanged. */
    setup(&x);word(x.code,0x6104);REQUIRE(denied(&x,R(CS_M68K_GUARD_TARGET,MA,0x401006,0,0,0)));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.pc==0x401006 && x.frame.a[7]==0x30001C &&
        get_long(x.stack+0x1C)==0x401002);
    /* JSR/JMP abs.L. */
    setup(&x);word(x.code,0x4EB9);longword(x.code+2,0x401010);
    REQUIRE(denied(&x,R(CS_M68K_GUARD_TARGET,MA,0x401010,0,0,0)));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.pc==0x401010 && x.frame.a[7]==0x30001C &&
        get_long(x.stack+0x1C)==0x401006);
    setup(&x);word(x.code,0x4EF9);longword(x.code+2,0x401010);
    REQUIRE(denied(&x,R(CS_M68K_GUARD_TARGET,MA,0x401010,0,0,0)));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.pc==0x401010 && x.frame.a[7]==0x300020);
    /* RTS: stack read and target deny independently; TARGET is not OPCODE. */
    setup(&x);word(x.code,0x4E75);longword(x.stack+0x20,0x401010);
    REQUIRE(denied(&x,R(CS_M68K_GUARD_READ,MA|MB,0x300020,4,0,0)));
    REQUIRE(denied(&x,R(CS_M68K_GUARD_TARGET,MA,0x401010,0,0,0)));
    REQUIRE(denied(&x,R(CS_M68K_GUARD_INSTRUCTION,MW,0,0,0x4E75,0)));
    REQUIRE(with_rule(&x,R(CS_M68K_GUARD_OPCODE,MA,0x401010,0,0,0))==CS_M68K_OK &&
        x.frame.pc==0x401010 && x.frame.a[7]==0x300024);
}
static void atomicity(void)
{
    fixture x;rule commit=R(CS_M68K_GUARD_COMMIT,0,0,0,0,0);
    /* Pending store, flags, PC and postincrement withheld. */
    setup(&x);word(x.code,move_word(4,0,0,3,0));x.frame.d[0]=0x80000000;
    REQUIRE(denied(&x,commit));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.a[0]==0x1804 && get_long(x.data.b)==0x80000000 &&
        x.frame.sr==0x2708 && x.frame.pc==0x401002);
    setup(&x);word(x.code,move_word(4,3,0,0,1));longword(x.data.b,0);
    REQUIRE(denied(&x,commit));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.d[1]==0 && x.frame.sr==0x2704 && x.frame.a[0]==0x1804);
    setup(&x);word(x.code,move_word(4,4,0,2,1));x.frame.a[0]=0x1804;x.frame.a[1]=0x1810;
    REQUIRE(denied(&x,commit));
    REQUIRE(denied(&x,R(CS_M68K_GUARD_WRITE,MA,0x1810,0,0,0)));
    /* Late extension/write denial after staged source increment. */
    setup(&x);word(x.code,move_word(4,3,0,7,1));longword(x.code+2,0x1810);longword(x.data.b,0x55667788);
    REQUIRE(denied(&x,R(CS_M68K_GUARD_EXTENSION,MA|MB,0x401002,4,0,0)));
    REQUIRE(denied(&x,R(CS_M68K_GUARD_WRITE,MA|MB,0x1810,4,0,0)));
    REQUIRE(denied(&x,commit));
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.a[0]==0x1804 && get_long(x.data.b+0x10)==0x55667788 &&
        x.frame.pc==0x401006);
    /* MOVEA and MOVEQ commits. */
    setup(&x);word(x.code,move_word(4,0,0,1,1));x.frame.d[0]=0x1234;REQUIRE(denied(&x,commit));
    setup(&x);word(x.code,0x70FF);REQUIRE(denied(&x,commit));
    /* Control-flow commits leave stack and A7 untouched. */
    setup(&x);word(x.code,0x6104);REQUIRE(denied(&x,commit));
    setup(&x);word(x.code,0x4EB9);longword(x.code+2,0x401010);REQUIRE(denied(&x,commit));
    REQUIRE(denied(&x,R(CS_M68K_GUARD_WRITE,MA|MB,0x30001C,4,0,0)));
    setup(&x);word(x.code,0x4E75);longword(x.stack+0x20,0x401010);REQUIRE(denied(&x,commit));
    setup(&x);word(x.code,0x6004);REQUIRE(denied(&x,commit));
    /* The final hook sees both incoming and complete staged control/CCR state. */
    {
        const cs_m68k_guard state={staged_state,NULL,0};
        setup(&x);word(x.code,move_word(4,0,0,3,0));x.frame.d[0]=0x80000000;
        REQUIRE(blocked(&x,&state,CS_M68K_POLICY));
        x.frame.d[0]=1;REQUIRE(step(&x,&state)==CS_M68K_OK && x.frame.a[0]==0x1804);
    }
}
static void control(void)
{
    fixture x;
    /* Register-only MOVEA.L D0,A7 must keep A7 even and bounded. */
    const uint32_t bad[]={0x300011,0x400000,0x2FFFFE,0x300042,0x0};
    for(unsigned i=0;i<5;++i) {
        setup(&x);word(x.code,move_word(4,0,0,1,7));x.frame.d[0]=bad[i];
        REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    }
    setup(&x);word(x.code,move_word(4,0,0,1,7));x.frame.d[0]=0x300010;
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.a[7]==0x300010);
    /* Initial S/T/I mismatch fails MODE; CCR bits are free. */
    const uint16_t modes[]={0x2000,0x0700,0xA700,0x2600,0x8700};
    for(unsigned i=0;i<5;++i) {setup(&x);x.frame.sr=modes[i];REQUIRE(blocked(&x,&permit,CS_M68K_MODE));}
    setup(&x);x.frame.sr=0x271F;word(x.code,0x7000);
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.sr==0x2714);
    /* Strict24: high address bits are never masked. */
    setup(&x);word(x.code,move_word(4,7,1,0,0));longword(x.code+2,0xFF001800);
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    setup(&x);word(x.code,move_word(4,0,0,7,1));longword(x.code+2,0x01001800);
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    setup(&x);word(x.code,0x4EF9);longword(x.code+2,0xFF401010);
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    /* Map permissions survive permit-all. */
    setup(&x);x.regions[1].writable=0;x.regions[1].flags=CS_GUEST_READ;
    word(x.code,move_word(4,0,0,2,0));REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    setup(&x);x.frame.pc=0x1800;x.data.b[0]=0x4E;x.data.b[1]=0x71;
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    setup(&x);word(x.code,move_word(4,7,1,0,0));longword(x.code+2,0x2000);
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    setup(&x);word(x.code,0x4EF9);longword(x.code+2,0x1800);
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    setup(&x);x.regions[2].writable=0;x.regions[2].flags=CS_GUEST_READ;word(x.code,0x6104);
    REQUIRE(blocked(&x,&permit,CS_M68K_ADDRESS));
    /* Unknown, F-line and TRAP opcodes. */
    const uint16_t ops[]={0x4AFC,0xF000,0xFFFF,0x4E40,0x4E4F,0x4E76,0x4E73};
    for(unsigned i=0;i<7;++i) {setup(&x);word(x.code,ops[i]);REQUIRE(blocked(&x,&permit,CS_M68K_OPCODE));}
}
static void services(void)
{
    /* Cover every bit combination of the forbidden A-line class. */
    for(uint32_t opcode=0xA000;opcode<=0xAFFF;++opcode) {
        fixture x;setup(&x);word(x.code,(uint16_t)opcode);
        REQUIRE(blocked(&x,&permit,CS_M68K_POLICY));
    }
    const uint16_t words[]={0xA046,0xA047,0xA9F4,0xA000,0xA800,0xAFFF,0xA01F,0xA9A0};
    for(unsigned i=0;i<8;++i) {
        fixture x;setup(&x);word(x.code,words[i]);x.frame.d[0]=0x46;x.frame.a[0]=0x1800;
        REQUIRE(blocked(&x,&permit,CS_M68K_POLICY));
        REQUIRE(with_rule(&x,R(CS_M68K_GUARD_OPCODE,MA,0x7FFFFFFE,0,0,0))==CS_M68K_POLICY);
        snapshot s;take(&x,&s);rule none=R(CS_M68K_GUARD_COMMIT,MA,0x7FFFFFFE,0,0,0);
        cs_m68k_guard g={deny_rule,&none,sizeof none};
        REQUIRE(step(&x,&g)==CS_M68K_POLICY && same(&x,&s) && x.report.cpu.service_called==0);
        /* Also from a nonzero CCR and after a prior successful step. */
        setup(&x);word(x.code,0x4E71);word(x.code+2,words[i]);x.frame.sr=0x271F;
        REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.pc==0x401002);
        REQUIRE(blocked(&x,&permit,CS_M68K_POLICY));
    }
}
static int report_kept(const fixture *x,const cs_m68k_bus_report *saved)
{return memcmp(saved,&x->report,sizeof *saved)==0;}
static void aliases(void)
{
    fixture x;cs_m68k_bus_report saved;snapshot s;rule none=R(CS_M68K_GUARD_COMMIT,MA,0x7FFFFFFE,0,0,0);
    cs_m68k_guard g={deny_rule,&none,sizeof none};
    /* Missing policy and context extent mismatches. */
    const cs_m68k_guard bad[]={{0,&none,sizeof none},{deny_rule,&none,0},{deny_rule,NULL,sizeof none},
        {permit_all,NULL,4},{0,NULL,0}};
    for(unsigned i=0;i<5;++i) {
        setup(&x);memset(&x.report,0x5A,sizeof x.report);saved=x.report;take(&x,&s);
        REQUIRE(step(&x,&bad[i])==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
    }
    setup(&x);memset(&x.report,0x5A,sizeof x.report);saved=x.report;take(&x,&s);
    REQUIRE(step(&x,NULL)==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
    /* Frame/report overlap. */
    {
        union { cs_trap_frame f;cs_m68k_bus_report r;uint64_t q; } u;uint8_t before[sizeof u];
        setup(&x);memset(&u,0,sizeof u);u.f=x.frame;memcpy(before,&u,sizeof u);
        REQUIRE(cs_m68k_bus_step_guarded(&x.map,&x.gateway,&u.f,&u.r,&x.limits,&g)==CS_M68K_ARGUMENT &&
            memcmp(before,&u,sizeof u)==0);
    }
    /* Report inside guest backing. */
    setup(&x);take(&x,&s);
    REQUIRE(cs_m68k_bus_step_guarded(&x.map,&x.gateway,&x.frame,&x.data.r,&x.limits,&g)==CS_M68K_ARGUMENT &&
        same(&x,&s));
    /* Context inside writable backing, and context overlapping the report. */
    setup(&x);memset(&x.report,0x5A,sizeof x.report);saved=x.report;take(&x,&s);
    {
        cs_m68k_guard c={permit_all,x.data.b,8};
        REQUIRE(step(&x,&c)==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
        cs_m68k_guard t={permit_all,x.stack,8};
        REQUIRE(step(&x,&t)==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
        cs_m68k_guard r={permit_all,&x.report,8};
        REQUIRE(step(&x,&r)==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
        cs_m68k_guard f={permit_all,&x.frame,8};
        REQUIRE(step(&x,&f)==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
    }
    /* Distinct guest addresses backed by overlapping native spans (all live). */
    setup(&x);x.regions[2]=(cs_guest_region){x.data.b+128,x.data.b+128,0x300000,64,3};
    memset(&x.report,0x5A,sizeof x.report);saved=x.report;take(&x,&s);
    REQUIRE(step(&x,&g)==CS_M68K_ARGUMENT && report_kept(&x,&saved) && same(&x,&s));
    setup(&x);x.regions[2]=(cs_guest_region){x.data.b+128,x.data.b+128,0x300000,64,3};
    x.regions[1].size=128;memset(&x.report,0x5A,sizeof x.report);
    REQUIRE(step(&x,&g)==CS_M68K_OK && x.frame.pc==0x401002);
    /* Report overlapping the gateway / limits. */
    setup(&x);take(&x,&s);
    {
        union { cs_trap_gateway gw;cs_m68k_bus_report r;uint64_t q; } u;uint8_t before[sizeof u];
        memset(&u,0,sizeof u);u.gw=x.gateway;memcpy(before,&u,sizeof u);
        REQUIRE(cs_m68k_bus_step_guarded(&x.map,&u.gw,&x.frame,&u.r,&x.limits,&g)==CS_M68K_ARGUMENT &&
            memcmp(before,&u,sizeof u)==0 && same(&x,&s));
    }
    /* Success cases: valid envelopes still execute. */
    setup(&x);word(x.code,0x7005);
    REQUIRE(step(&x,&g)==CS_M68K_OK && x.frame.d[0]==5 && x.frame.pc==0x401002);
    REQUIRE(step(&x,&permit)==CS_M68K_OK && x.frame.pc==0x401004);
    {
        static const rule ro={CS_M68K_GUARD_COMMIT,MA,0x7FFFFFFE,0,0,0};
        cs_m68k_guard c={deny_rule,&ro,sizeof ro};
        REQUIRE(step(&x,&c)==CS_M68K_OK && x.frame.pc==0x401006);
    }
}
int main(int argc,char **argv)
{
    if(argc!=2) {fprintf(stderr,"usage: m68k-guard events|atomicity|control|services|aliases\n");return 2;}
    if(strcmp(argv[1],"events")==0) events();
    else if(strcmp(argv[1],"atomicity")==0) atomicity();
    else if(strcmp(argv[1],"control")==0) control();
    else if(strcmp(argv[1],"services")==0) services();
    else if(strcmp(argv[1],"aliases")==0) aliases();
    else {fprintf(stderr,"unknown suite: %s\n",argv[1]);return 2;}
    puts("ok");
    return 0;
}
