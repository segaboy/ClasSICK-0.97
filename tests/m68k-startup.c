/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/cpu/m68k-guard.h"
static unsigned checks;
#define REQUIRE(e) do {++checks;if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
typedef struct {
    uint8_t code[64],data[64],stack[64];
    cs_guest_region regions[3];cs_guest_map map;cs_trap_gateway gateway;
    cs_trap_frame frame;cs_m68k_bus_report report;cs_m68k_stack_limits limits;
} fixture;
typedef struct {cs_trap_frame frame;cs_trap_gateway gateway;uint8_t code[64],data[64],stack[64];} saved;
static int allow(const void *context,const cs_m68k_guard_event *event)
{(void)context;(void)event;return 1;}
static const cs_m68k_guard permission={allow,NULL,0};
static void w16(uint8_t *p,uint16_t value){p[0]=(uint8_t)(value>>8);p[1]=(uint8_t)value;}
static void w32(uint8_t *p,uint32_t value){w16(p,(uint16_t)(value>>16));w16(p+2,(uint16_t)value);}
static uint32_t r32(const uint8_t *p)
{return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->stack,0xa5,sizeof x->stack);memset(x->data,0x6d,sizeof x->data);
    for(unsigned i=0;i<64;i+=2)w16(x->code+i,0x4e71);
    x->regions[0]=(cs_guest_region){x->code,NULL,0x401000,64,5};
    x->regions[1]=(cs_guest_region){x->data,x->data,0x1800,64,3};
    x->regions[2]=(cs_guest_region){x->stack,x->stack,0x300000,64,3};
    x->map=(cs_guest_map){x->regions,3};x->limits=(cs_m68k_stack_limits){0x300000,64};
    cs_trap_config config={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&config)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i){x->frame.d[i]=0x00100100u+i;x->frame.a[i]=0x1800u+2u*i;}
    x->frame.pc=0x401000;x->frame.a[7]=0x300020;x->frame.sr=0x271f;
}
static cs_m68k_result step(fixture *x,const cs_m68k_guard *guard)
{return cs_m68k_bus_step_guarded(&x->map,&x->gateway,&x->frame,&x->report,&x->limits,guard);}
static void take(const fixture *x,saved *s)
{s->frame=x->frame;s->gateway=x->gateway;memcpy(s->code,x->code,64);memcpy(s->data,x->data,64);memcpy(s->stack,x->stack,64);}
static void unchanged(const fixture *x,const saved *s)
{
    REQUIRE(memcmp(&x->frame,&s->frame,sizeof s->frame)==0);
    REQUIRE(memcmp(&x->gateway,&s->gateway,sizeof s->gateway)==0);
    REQUIRE(memcmp(x->code,s->code,64)==0);REQUIRE(memcmp(x->data,s->data,64)==0);REQUIRE(memcmp(x->stack,s->stack,64)==0);
    REQUIRE(x->report.cpu.service_called==0);
}
static void failed(fixture *x,const cs_m68k_guard *guard,cs_m68k_result expected,int bounded)
{
    saved before;take(x,&before);
    cs_m68k_result result=bounded?step(x,guard):cs_m68k_bus_step(&x->map,&x->gateway,&x->frame,&x->report);
    REQUIRE(result==expected);REQUIRE(x->report.cpu.status==expected);unchanged(x,&before);
}
static void success(fixture *x,const saved *before,const cs_trap_frame *expected,int slot,uint32_t value)
{
    REQUIRE(step(x,&permission)==CS_M68K_OK);
    REQUIRE(memcmp(&x->frame,expected,sizeof *expected)==0);
    REQUIRE(memcmp(&x->gateway,&before->gateway,sizeof before->gateway)==0);
    REQUIRE(memcmp(x->code,before->code,64)==0);REQUIRE(memcmp(x->data,before->data,64)==0);
    for(unsigned i=0;i<64;++i){
        uint8_t byte=before->stack[i];
        if(slot>=0 && i>=(unsigned)slot && i<(unsigned)slot+4u)byte=(uint8_t)(value>>(24u-8u*(i-(unsigned)slot)));
        REQUIRE(x->stack[i]==byte);
    }
    REQUIRE(x->report.cpu.service_called==0 && x->report.branch_taken==0 && x->report.subroutine==0);
}
static void address_modes(void)
{
    /* Literal interface encodings and independently evaluated results. */
    static const struct {uint16_t ea,extension;unsigned bytes;uint32_t value;} cases[]={
        {0x10,0,0,0x1800},{0x2a,0xfffa,2,0x17fe},
        {0x33,0x10fc,2,0x1903},{0x33,0x9804,2,0x300c},
        {0x38,0xff81,2,0xffffff81u},{0x39,0,4,0xf1234567u},
        {0x3a,0xfff8,2,0x400ffa},{0x3b,0x1804,2,0x501107}
    };
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];++i)for(unsigned dest=0;dest<7;++dest){
        fixture x;setup(&x);w16(x.code,(uint16_t)(0x41c0u+dest*512u+cases[i].ea));
        if(cases[i].bytes==2)w16(x.code+2,cases[i].extension);
        if(cases[i].bytes==4)w32(x.code+2,cases[i].value);
        saved before;take(&x,&before);cs_trap_frame expected=x.frame;
        expected.a[dest]=cases[i].value;expected.pc+=2u+cases[i].bytes;
        success(&x,&before,&expected,-1,0);
        setup(&x);w16(x.code,(uint16_t)(0x4840u+cases[i].ea));
        if(cases[i].bytes==2)w16(x.code+2,cases[i].extension);
        if(cases[i].bytes==4)w32(x.code+2,cases[i].value);
        take(&x,&before);expected=x.frame;expected.a[7]-=4;expected.pc+=2u+cases[i].bytes;
        success(&x,&before,&expected,28,cases[i].value);
    }
    for(unsigned reg=0;reg<8;++reg){
        fixture x;setup(&x);w16(x.code,(uint16_t)(0x4850u+reg));
        saved before;take(&x,&before);cs_trap_frame expected=x.frame;expected.pc+=2;expected.a[7]-=4;
        success(&x,&before,&expected,28,before.frame.a[reg]);
    }
    fixture x;setup(&x);w16(x.code,0x4fd1);x.frame.a[1]=0x300040;
    saved before;take(&x,&before);cs_trap_frame expected=x.frame;expected.pc+=2;expected.a[7]=0x300040;
    success(&x,&before,&expected,-1,0);
    setup(&x);w16(x.code,0x4fd1);x.frame.a[1]=0x300042;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4fd1);x.frame.a[1]=0x300001;failed(&x,&permission,CS_M68K_ADDRESS,1);
    /* DATA24 never masks a computed address. */
    setup(&x);w16(x.code,0x41f9);w32(x.code+2,0xff001801u);
    cs_m68k_bus_options options={CS_M68K_DATA24,NULL,NULL};
    REQUIRE(cs_m68k_bus_step_options(&x.map,&x.gateway,&x.frame,&x.report,&options)==CS_M68K_OK);
    REQUIRE(x.frame.a[0]==0xff001801u && x.frame.sr==0x271f);
}
static void frames(void)
{
    static const struct {uint16_t displacement;uint32_t sp;} offsets[]={
        {0,0x30001c},{0xfff8,0x300014},{0xffe4,0x300000},{4,0x300020},{36,0x300040}
    };
    for(unsigned reg=0;reg<7;++reg)for(unsigned i=0;i<sizeof offsets/sizeof offsets[0];++i){
        fixture x;setup(&x);w16(x.code,(uint16_t)(0x4e50u+reg));w16(x.code+2,offsets[i].displacement);
        saved before;take(&x,&before);cs_trap_frame expected=x.frame;expected.pc+=4;
        expected.a[reg]=0x30001c;expected.a[7]=offsets[i].sp;
        success(&x,&before,&expected,28,before.frame.a[reg]);
    }
    for(unsigned reg=0;reg<7;++reg)for(unsigned offset=0;offset<=60;offset+=2){
        fixture x;setup(&x);w16(x.code,(uint16_t)(0x4e58u+reg));x.frame.a[reg]=0x300000u+offset;
        w32(x.stack+offset,0xfffedcbau);saved before;take(&x,&before);cs_trap_frame expected=x.frame;
        expected.a[reg]=0xfffedcbau;expected.a[7]=0x300004u+offset;expected.pc+=2;
        success(&x,&before,&expected,-1,0);
    }
    for(unsigned ccr=0;ccr<32;++ccr)for(unsigned instruction=0;instruction<4;++instruction){
        fixture x;setup(&x);x.frame.sr=(uint16_t)(0x2700u|ccr);
        uint16_t opcode=instruction==0?0x41d1:instruction==1?0x4851:instruction==2?0x4e56:0x4e5e;
        w16(x.code,opcode);w16(x.code+2,0xfff8);x.frame.a[6]=0x300008;w32(x.stack+8,0x12345678);
        REQUIRE(step(&x,&permission)==CS_M68K_OK);REQUIRE(x.frame.sr==(uint16_t)(0x2700u|ccr));
    }
}
static void invalid(void)
{
    fixture x;
    static const uint16_t bad_ea[]={0,7,8,15,24,31,32,39,60,61,62,63};
    for(unsigned i=0;i<sizeof bad_ea/sizeof bad_ea[0];++i)for(unsigned pea=0;pea<2;++pea){
        setup(&x);w16(x.code,(uint16_t)((pea?0x4840u:0x41c0u)+bad_ea[i]));failed(&x,&permission,CS_M68K_OPCODE,1);
    }
    for(unsigned bits=1;bits<8;++bits){
        setup(&x);w16(x.code,0x41f0);w16(x.code+2,(uint16_t)(bits<<8));failed(&x,&permission,CS_M68K_OPCODE,1);
    }
    static const uint16_t unsupported[]={0x4e57,0x4e5f,0x480e};
    for(unsigned i=0;i<sizeof unsupported/sizeof unsupported[0];++i){setup(&x);w16(x.code,unsupported[i]);failed(&x,&permission,CS_M68K_OPCODE,1);}
    /* Checked arithmetic refuses wrapped base-plus-displacement/index. */
    setup(&x);w16(x.code,0x41e8);w16(x.code+2,0xffff);x.frame.a[0]=0;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x41e8);w16(x.code+2,1);x.frame.a[0]=UINT32_MAX;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4870);w16(x.code+2,0x0800);x.frame.d[0]=0x80000000u;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x41f0);w16(x.code+2,0x0801);x.frame.a[0]=0x80000000u;x.frame.d[0]=0x7fffffffu;failed(&x,&permission,CS_M68K_ADDRESS,1);
    /* Complete extension span; the second half cannot be fetched from a gap. */
    static const uint16_t extensions[]={0x41e8,0x4879,0x4e56};
    for(unsigned i=0;i<3;++i){setup(&x);w16(x.code+62,extensions[i]);x.frame.pc+=62;failed(&x,&permission,CS_M68K_ADDRESS,1);}
    setup(&x);w16(x.code+60,0x4879);x.frame.pc+=60;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,0xffe2);failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,38);failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,1);failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,0x8000);failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4850);x.frame.a[7]=0x300000;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e5e);x.frame.a[6]=0x30003e;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e5e);x.frame.a[6]=0x300001;failed(&x,&permission,CS_M68K_ADDRESS,1);
    setup(&x);w16(x.code,0x4e5e);x.frame.a[6]=0x1800;failed(&x,&permission,CS_M68K_ADDRESS,1);
    /* Unbounded API still validates the actual complete memory operand. */
    setup(&x);w16(x.code,0x4850);x.frame.a[7]=2;failed(&x,&permission,CS_M68K_ADDRESS,0);
    setup(&x);w16(x.code,0x4850);x.frame.a[7]=0x300021;failed(&x,&permission,CS_M68K_ADDRESS,0);
    setup(&x);w16(x.code,0x4850);x.regions[2].flags=1;x.regions[2].writable=NULL;failed(&x,&permission,CS_M68K_ADDRESS,0);
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,0);x.regions[2].size=30;failed(&x,&permission,CS_M68K_ADDRESS,0);
    setup(&x);w16(x.code,0x4e5e);x.frame.a[6]=0x30003e;failed(&x,&permission,CS_M68K_ADDRESS,0);
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,0xffe0);x.regions[2].base=0;x.frame.a[7]=32;
    failed(&x,&permission,CS_M68K_ADDRESS,0);
    /* Late sequential PC failure happens after a push was staged. */
    setup(&x);x.regions[0].base=0xffffc0;x.frame.pc=0xfffffc;w16(x.code+60,0x4e56);w16(x.code+62,0);
    failed(&x,&permission,CS_M68K_ADDRESS,1);
}
typedef struct {cs_m68k_guard_kind kind;} refusal;
static int deny(const void *context,const cs_m68k_guard_event *event)
{return event->kind!=((const refusal *)context)->kind;}
static int no_operand_read(const void *context,const cs_m68k_guard_event *event)
{(void)context;return event->kind!=CS_M68K_GUARD_READ && event->kind!=CS_M68K_GUARD_TARGET;}
static int deny_proposed_link(const void *context,const cs_m68k_guard_event *event)
{
    (void)context;
    if(event->kind!=CS_M68K_GUARD_COMMIT)return 1;
    return !(event->before->a[6]==0x180c && event->before->a[7]==0x300020 &&
        event->proposed->a[6]==0x30001c && event->proposed->a[7]==0x300014 && event->proposed->pc==0x401004);
}
static void guards(void)
{
    static const uint16_t opcodes[]={0x41f9,0x4879,0x4e56,0x4e5e};
    static const cs_m68k_guard_kind common[]={CS_M68K_GUARD_OPCODE,CS_M68K_GUARD_INSTRUCTION,CS_M68K_GUARD_COMMIT};
    fixture x;
    for(unsigned instruction=0;instruction<4;++instruction)for(unsigned event=0;event<3;++event){
        setup(&x);w16(x.code,opcodes[instruction]);w32(x.code+2,0x1800);if(instruction==2)w16(x.code+2,0xfff8);
        x.frame.a[6]=0x300008;w32(x.stack+8,0x12345678);
        const refusal rule={common[event]};const cs_m68k_guard guard={deny,&rule,sizeof rule};failed(&x,&guard,CS_M68K_POLICY,1);
    }
    for(unsigned instruction=0;instruction<3;++instruction){
        setup(&x);w16(x.code,opcodes[instruction]);w32(x.code+2,0x1800);
        const refusal rule={CS_M68K_GUARD_EXTENSION};const cs_m68k_guard guard={deny,&rule,sizeof rule};failed(&x,&guard,CS_M68K_POLICY,1);
    }
    for(unsigned instruction=1;instruction<3;++instruction){
        setup(&x);w16(x.code,opcodes[instruction]);w32(x.code+2,0x1800);if(instruction==2)w16(x.code+2,0xfff8);
        const refusal rule={CS_M68K_GUARD_WRITE};const cs_m68k_guard guard={deny,&rule,sizeof rule};failed(&x,&guard,CS_M68K_POLICY,1);
    }
    setup(&x);w16(x.code,0x4e5e);x.frame.a[6]=0x300008;
    const refusal rule={CS_M68K_GUARD_READ};const cs_m68k_guard guard={deny,&rule,sizeof rule};failed(&x,&guard,CS_M68K_POLICY,1);
    const cs_m68k_guard no_read={no_operand_read,NULL,0};
    for(unsigned instruction=0;instruction<2;++instruction){
        setup(&x);w16(x.code,opcodes[instruction]);w32(x.code+2,0xffffffffu);REQUIRE(step(&x,&no_read)==CS_M68K_OK);
    }
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,0xfff8);
    const cs_m68k_guard proposal={deny_proposed_link,NULL,0};failed(&x,&proposal,CS_M68K_POLICY,1);
    /* Admitted execution still goes through the same transaction. */
    setup(&x);w16(x.code,0x4e56);w16(x.code+2,0xfff8);x.regions[0].flags=1;
    REQUIRE(cs_m68k_bus_step_admitted(&x.map,&x.gateway,&x.frame,&x.report,&x.limits,&permission,0x401020,0x30003c)==CS_M68K_OK);
    REQUIRE(x.frame.a[6]==0x30001c && x.frame.a[7]==0x300014 && r32(x.stack+28)==0x180c);
}
static void composed(void)
{
    static const uint8_t program[]={0x4e,0x56,0xff,0xf8,0x41,0xee,0xff,0xfc,0x48,0x50,0x20,0x1f,0x4e,0x5e};
    fixture x;setup(&x);memcpy(x.code,program,sizeof program);
    for(unsigned i=0;i<5;++i)REQUIRE(step(&x,&permission)==CS_M68K_OK);
    REQUIRE(x.frame.pc==0x40100e && x.frame.a[0]==0x300018 && x.frame.d[0]==0x300018);
    REQUIRE(x.frame.a[6]==0x180c && x.frame.a[7]==0x300020);
    REQUIRE(r32(x.stack+28)==0x180c && r32(x.stack+16)==0x300018);
    REQUIRE(x.frame.sr==0x2710);
}
int main(void)
{
    address_modes();frames();invalid();guards();composed();
    printf("M68K startup: %u checks passed\n",checks);return 0;
}
