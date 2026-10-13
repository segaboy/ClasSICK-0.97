/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../personality/system1/toolbox.h"
#include "../apps/toolbox-probe/program.h"
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}} while(0)
enum { RECORDS=7 };
typedef struct {unsigned mode;uint32_t result;} handler_config;
typedef struct {
    uint8_t code[512],tokens[8],identities[32],data[512],stack[256];
    cs_guest_region regions[5];cs_guest_map map;cs_trap_gateway gateway;
    cs_m68k_session session;cs_tb_runtime toolbox;cs_trap_frame frame;cs_m68k_bus_report report;
    cs_tb_record records[RECORDS];cs_tb_argument mixed[4],spans[3],one[1];handler_config handler;
} fixture;
static void word(uint8_t *p,uint16_t v) {p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void longword(uint8_t *p,uint32_t v) {word(p,(uint16_t)(v>>16));word(p+2,(uint16_t)v);}
static uint32_t read_long(const uint8_t *p)
{return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static unsigned mixed(void *context,const cs_tb_input *in,cs_tb_reply *out)
{
    handler_config *c=context;out->result=in->value[0]^in->value[1]^in->value[2]^in->value[3];
    if(c->mode==1) out->bytes=1;
    return c->mode==2?1u:0u;
}
static unsigned copy(void *context,const cs_tb_input *in,cs_tb_reply *out)
{
    handler_config *c=context;out->bytes=in->length[1];out->result=c->result;
    for(uint32_t i=0;i<out->bytes;++i) out->output[i]=(uint8_t)(in->span[0][i]^in->span[2][0]);
    if(c->mode==1) ++out->bytes;
    return c->mode==2?1u:0u;
}
static unsigned literal(void *context,const cs_tb_input *in,cs_tb_reply *out)
{handler_config *c=context;(void)in;out->result=c->result;return c->mode==2?1u:0u;}
static unsigned scatter(void *context,const cs_tb_input *in,cs_tb_reply *out)
{
    handler_config *c=context;out->bytes=in->length[0]+in->length[1];out->result=0;
    for(uint32_t i=0;i<out->bytes;++i) out->output[i]=(uint8_t)(c->result+i);
    return 0;
}
static cs_tb_argument scalar(cs_tb_type type)
{cs_tb_argument a={type,0,0,0,0};return a;}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->data,0xCC,sizeof x->data);memset(x->stack,0xCC,sizeof x->stack);
    for(unsigned i=0;i<sizeof x->code;i+=2) word(x->code+i,0x4E71);
    for(unsigned i=0;i<sizeof x->tokens;i+=2) word(x->tokens+i,0x4E71);
    for(unsigned i=0;i<sizeof x->identities;i+=2) word(x->identities+i,0x4E71);
    x->regions[0]=(cs_guest_region){x->code,0,0x6000,sizeof x->code,5};
    x->regions[1]=(cs_guest_region){x->tokens,0,0x400180,sizeof x->tokens,5};
    x->regions[2]=(cs_guest_region){x->identities,0,0x400300,sizeof x->identities,5};
    x->regions[3]=(cs_guest_region){x->data,x->data,0x1800,sizeof x->data,3};
    x->regions[4]=(cs_guest_region){x->stack,x->stack,0x8000,sizeof x->stack,3};
    x->map=(cs_guest_map){x->regions,5};
    cs_trap_config gateway={0x400000,0x6000,0x7000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&gateway)==CS_TRAP_OK);
    cs_m68k_call_config config={0x8000,sizeof x->stack,0x400180,4};
    REQUIRE(cs_m68k_session_init(&x->session,&x->map,&x->gateway,0,&config)==CS_M68K_SESSION_OK);
    x->mixed[0]=scalar(CS_TB_WORD);x->mixed[1]=scalar(CS_TB_LONG);
    x->mixed[2]=scalar(CS_TB_BOOL);x->mixed[3]=scalar(CS_TB_CHAR);x->one[0]=scalar(CS_TB_WORD);
    x->spans[0]=(cs_tb_argument){CS_TB_SPAN,4,1,2,0};
    x->spans[1]=(cs_tb_argument){CS_TB_SPAN,4,2,2,0};
    x->spans[2]=(cs_tb_argument){CS_TB_STRING,0,1,1,0};
    x->records[0]=(cs_tb_record){0xA8F0,CS_TB_STACK,CS_TB_RETURNS,0x400300,4,CS_TB_LONG,x->mixed,mixed,&x->handler};
    x->records[1]=(cs_tb_record){0xA8F1,CS_TB_STACK,CS_TB_RETURNS,0x400302,3,CS_TB_BOOL,x->spans,copy,&x->handler};
    x->records[2]=(cs_tb_record){0xA8F2,CS_TB_STACK,CS_TB_RETURNS,0x400304,1,CS_TB_NONE,x->one,literal,&x->handler};
    x->records[3]=(cs_tb_record){0xA9F4,CS_TB_STACK,CS_TB_EXITS,0x400306,0,CS_TB_NONE,0,0,0};
    x->records[4]=(cs_tb_record){0xA9F0,CS_TB_STACK,CS_TB_CONTINUES,0x400308,1,CS_TB_NONE,x->one,0,0};
    x->records[5]=(cs_tb_record){0xA96E,CS_TB_REGISTER,CS_TB_RETURNS,0x40030A,0,CS_TB_NONE,0,0,0};
    x->records[6]=(cs_tb_record){0xA8F3,CS_TB_STACK,CS_TB_RETURNS,0x40030C,0,CS_TB_CHAR,0,literal,&x->handler};
    REQUIRE(cs_tb_init(&x->toolbox,&x->session,x->records,RECORDS,0)==CS_TB_OK);
    for(unsigned i=0;i<8;++i) {x->frame.d[i]=0xABCD0000u+i;x->frame.a[i]=0x1800;}
    x->frame.pc=0x6000;x->frame.a[7]=0x8080;x->frame.a[5]=0x1810;x->frame.sr=0x271F;
    x->handler.result=1;
}
static cs_m68k_result step(fixture *x)
{cs_m68k_stack_limits limits={0x8000,sizeof x->stack};return cs_tb_step(&x->toolbox,&x->frame,&x->report,&limits);}
static void input_mixed(fixture *x,uint16_t b,uint16_t ch)
{
    word(x->code,0xA8F0);word(x->stack+128,ch);word(x->stack+130,b);
    longword(x->stack+132,0x89ABCDEF);word(x->stack+136,0x1234);
}
static void input_spans(fixture *x)
{
    word(x->code,0xA8F1);longword(x->stack+128,0x1830);longword(x->stack+132,0x1820);
    longword(x->stack+136,0x1808);x->data[0x30]=3;
    for(unsigned i=0;i<4;++i) x->data[8+i]=(uint8_t)(0x80u+i);
}
static void atomic(fixture *x,cs_tb_result expected)
{
    cs_trap_frame f=x->frame;cs_trap_gateway g=x->gateway;uint8_t data[512],stack[256];
    memcpy(data,x->data,sizeof data);memcpy(stack,x->stack,sizeof stack);
    REQUIRE(step(x)==CS_M68K_SERVICE && x->toolbox.last.status==expected);
    REQUIRE(!memcmp(&f,&x->frame,sizeof f) && !memcmp(&g,&x->gateway,sizeof g));
    REQUIRE(!memcmp(data,x->data,sizeof data) && !memcmp(stack,x->stack,sizeof stack));
}
static void frames(void)
{
    for(unsigned b=0;b<256;++b) {
        fixture x;setup(&x);input_mixed(&x,(uint16_t)((b<<8)|0xFFu),0xA542);cs_trap_frame f=x.frame;
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x6002 && x.frame.a[7]==0x808A);
        REQUIRE(read_long(x.stack+138)==(0x89ABCDEFu^0x1234u^(b&1u)^0x42u));
        REQUIRE(x.toolbox.last.parameters==10 && x.toolbox.last.normalized_sp==0x807C && x.toolbox.last.invoked==1);
        f.pc+=2;f.a[7]+=10;REQUIRE(!memcmp(&f,&x.frame,sizeof f));
        REQUIRE(read_long(x.stack+124)==0x6002 && x.stack[122]==0x27 && x.stack[123]==0x1F);
    }
    const cs_tb_type types[]={CS_TB_WORD,CS_TB_LONG,CS_TB_BOOL,CS_TB_CHAR};
    for(unsigned t=0;t<4;++t) for(unsigned zero=0;zero<2;++zero) {
        fixture x;setup(&x);word(x.code,0xA8F3);x.records[6].result=types[t];
        x.handler.result=zero?0u:0xDEADBEEFu;
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[7]==0x8080);
        uint32_t result;unsigned bytes=types[t]==CS_TB_LONG?4u:2u;
        REQUIRE(cs_guest_read(&x.map,0x8080,bytes,1,&result)==CS_GUEST_OK);
        REQUIRE(result==(types[t]==CS_TB_LONG?x.handler.result:types[t]==CS_TB_BOOL?(zero?0u:0x100u):
            types[t]==CS_TB_CHAR?(x.handler.result&0xFFu):(x.handler.result&0xFFFFu)));
    }
}
static void admission(void)
{
    for(unsigned test=0;test<26;++test) {
        fixture x;setup(&x);cs_tb_runtime out;memset(&out,0xA5,sizeof out);cs_tb_runtime before=out;
        cs_trap_gateway gate=x.gateway;size_t count=RECORDS;
        switch(test) {
            case 0:count=0;break;case 1:count=17;break;
            case 2:x.records[0].word=0xACF0;break;case 3:x.records[0].word=0xAAF0;break;
            case 4:x.records[0].word=0xA046;break;case 5:x.records[0].word=0xA846;break;
            case 6:x.records[1].identity=x.records[0].identity;break;
            case 7:x.records[1].word=x.records[0].word;break;
            case 8:x.records[0].identity=0x400301;break;case 9:x.records[0].identity=0x6000;break;
            case 10:x.records[0].identity=0x400180;break;case 11:x.records[0].identity=0x400100;break;
            case 12:x.records[0].count=9;break;case 13:x.records[0].arguments=0;break;
            case 14:x.records[0].result=CS_TB_STRING;break;case 15:x.records[0].convention=(cs_tb_convention)9;break;
            case 16:x.records[0].completion=CS_TB_EXITS;break;case 17:x.records[0].handler=0;break;
            case 18:x.spans[0].alignment=3;break;case 19:x.spans[1].bytes=17;break;
            case 20:x.spans[0].access=4;break;case 21:x.mixed[0].bytes=2;break;
            case 22:x.records[5].handler=literal;break;case 23:x.records[4].handler=literal;break;
            case 24:word(x.identities,0x4E75);break;
            default:x.regions[4].flags=7;break;
        }
        REQUIRE(cs_tb_init(&out,&x.session,x.records,count,0)==CS_TB_CONFIG);
        REQUIRE(!memcmp(&out,&before,sizeof out) && !memcmp(&x.gateway,&gate,sizeof gate));
    }
    fixture x;setup(&x);cs_tb_runtime before=x.toolbox;cs_trap_gateway g=x.gateway;
    x.session.active=1;REQUIRE(cs_tb_init(&x.toolbox,&x.session,x.records,RECORDS,0)==CS_TB_CONTEXT);
    REQUIRE(!memcmp(&before,&x.toolbox,sizeof before) && !memcmp(&g,&x.gateway,sizeof g));x.session.active=0;
    REQUIRE(cs_tb_init(0,&x.session,x.records,RECORDS,0)==CS_TB_ARGUMENT);
    REQUIRE(cs_trap_set(&x.gateway,0xF0,0x6020)==CS_TRAP_OK);g=x.gateway;
    REQUIRE(cs_tb_init(&x.toolbox,&x.session,x.records,RECORDS,0)==CS_TB_SIGNATURE && !memcmp(&g,&x.gateway,sizeof g));
}
static void spans(void)
{
    fixture x;setup(&x);input_spans(&x);REQUIRE(step(&x)==CS_M68K_OK);
    for(unsigned i=0;i<4;++i) REQUIRE(x.data[0x20+i]==(uint8_t)((0x80u+i)^3u));
    REQUIRE(x.stack[140]==1 && x.stack[141]==0 && x.frame.a[7]==0x808C);
    REQUIRE(x.toolbox.last.outputs==4 && x.data[0x1F]==0xCC && x.data[0x24]==0xCC);
    setup(&x);input_spans(&x);x.data[0x30]=255;REQUIRE(step(&x)==CS_M68K_OK);
    setup(&x);input_spans(&x);x.data[0x30]=0;REQUIRE(step(&x)==CS_M68K_OK);
    for(unsigned test=0;test<12;++test) {
        setup(&x);input_spans(&x);cs_tb_result expected=CS_TB_ADDRESS;
        switch(test) {
            case 0:longword(x.stack+136,0x1809);break;
            case 1:longword(x.stack+132,0x19FE);break;
            case 2:longword(x.stack+128,0x19FF);x.data[511]=1;break;
            case 3:longword(x.stack+136,0xFF001808);x.toolbox.options.data_policy=CS_M68K_DATA24;break;
            case 4:longword(x.stack+132,0x6000);break;
            case 5:longword(x.stack+128,0x9000);break;
            case 6:longword(x.stack+132,0x8000);expected=CS_TB_ALIAS;break;
            case 7:longword(x.stack+132,0x1808);expected=CS_TB_ALIAS;break;
            case 8:longword(x.stack+128,0x180A);expected=CS_TB_ALIAS;break;
            case 9:x.spans[0].kept=1;expected=CS_TB_RETAINED;break;
            case 10:x.spans[0].kept=2;expected=CS_TB_CONFIG;break;
            default:x.regions[3].flags=1;x.regions[3].writable=0;break;
        }
        atomic(&x,expected);REQUIRE(x.toolbox.last.invoked==0);
    }
    setup(&x);input_spans(&x);x.spans[0].alignment=1;longword(x.stack+136,0x1809);REQUIRE(step(&x)==CS_M68K_OK);
    setup(&x);input_spans(&x);x.spans[0].bytes=16;x.spans[1].bytes=16;
    REQUIRE(step(&x)==CS_M68K_OK && x.toolbox.last.outputs==16);
    for(unsigned i=0;i<16;++i) REQUIRE(x.data[0x20+i]==(uint8_t)(x.data[8+i]^3u));
    setup(&x);input_spans(&x);x.spans[0].access=2;x.records[1].handler=scatter;x.handler.result=0x90;
    REQUIRE(step(&x)==CS_M68K_OK && x.toolbox.last.outputs==8);
    for(unsigned i=0;i<4;++i) REQUIRE(x.data[8+i]==0x90u+i && x.data[0x20+i]==0x94u+i);
    setup(&x);input_spans(&x);x.spans[0].access=2;x.spans[0].bytes=16;x.spans[1].bytes=16;
    atomic(&x,CS_TB_CONFIG);
    setup(&x);word(x.code,0xA8F2);x.one[0]=(cs_tb_argument){CS_TB_STRING,0,1,1,0};
    longword(x.stack+128,0xFFFFFF);x.regions[3].base=0xFFFF00;x.regions[3].size=256;x.data[255]=0;
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[7]==0x8084);
    x.frame.pc=0x6000;x.frame.a[7]=0x8080;x.data[255]=1;atomic(&x,CS_TB_ADDRESS);
}
static void failures(void)
{
    fixture x;
    for(unsigned test=0;test<10;++test) {
        setup(&x);input_mixed(&x,0x100,0x42);cs_tb_result expected=CS_TB_ADDRESS;
        switch(test) {
            case 0:x.frame.a[7]=0x8004;break;case 1:x.frame.a[7]=0x80FC;break;
            case 2:x.frame.a[7]=0x8081;break;case 3:x.frame.a[7]=0xFF008080;break;
            case 4:x.handler.mode=1;expected=CS_TB_REPLY;break;
            case 5:x.handler.mode=2;expected=CS_TB_HANDLER;break;
            case 6:x.toolbox.interrupt_context=1;expected=CS_TB_CONTEXT;break;
            case 7:x.toolbox.interrupt_context=2;expected=CS_TB_CONTEXT;break;
            case 8:x.session.calls.config.stack_base=0xFFFFFFFE;expected=CS_TB_CONFIG;break;
            default:x.frame.pc=0x61FE;word(x.code+510,0xA8F0);break;
        }
        atomic(&x,expected);
    }
    for(unsigned mode=1;mode<3;++mode) {
        setup(&x);input_spans(&x);x.handler.mode=mode;atomic(&x,mode==1?CS_TB_REPLY:CS_TB_HANDLER);
        REQUIRE(x.toolbox.last.invoked==1);
    }
    setup(&x);input_mixed(&x,0,0);x.frame.sr=0;
    cs_trap_frame before=x.frame;REQUIRE(step(&x)==CS_M68K_MODE && !memcmp(&before,&x.frame,sizeof before));
    setup(&x);x.frame.pc=0x6001;before=x.frame;
    REQUIRE(step(&x)==CS_M68K_ADDRESS && !memcmp(&before,&x.frame,sizeof before));
}
static void completions(void)
{
    fixture x;setup(&x);word(x.code,0xACF2);word(x.stack+128,0x2468);longword(x.stack+130,0x6040);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x6040 && x.frame.a[7]==0x8086 && x.report.branch_taken==1);
    for(unsigned test=0;test<7;++test) {
        setup(&x);word(x.code,0xACF2);word(x.stack+128,0x2468);longword(x.stack+130,0x6040);
        cs_tb_result expected=CS_TB_ADDRESS;
        if(test==0) longword(x.stack+130,0x6041);
        else if(test==1) longword(x.stack+130,0xFF006040);
        else if(test==2) longword(x.stack+130,0x8000);
        else if(test==3) {longword(x.stack+130,0x400180);expected=CS_TB_CONTEXT;}
        else if(test==4) {word(x.code,0xACF0);expected=CS_TB_FLAGS;}
        else if(test==5) {word(x.code,0xADF4);expected=CS_TB_FLAGS;}
        else {longword(x.stack+130,0x400300);expected=CS_TB_DIRECT;}
        atomic(&x,expected);
    }
    setup(&x);word(x.code,0xA9F4);atomic(&x,CS_TB_CONTEXT);
    setup(&x);word(x.code,0xA9F0);atomic(&x,CS_TB_CONTINUATION);
    setup(&x);word(x.code,0xA96E);atomic(&x,CS_TB_CONVENTION);
}
static cs_m68k_session_result run(fixture *x,uint32_t budget,cs_m68k_session_report *r)
{
    cs_m68k_session_request q={0x6000,0x1810,budget,CS_M68K_CALL_ORDINARY};x->frame.a[7]=0x8100;
    return cs_m68k_session_run_using(&x->session,&q,&x->frame,r,cs_tb_step,&x->toolbox);
}
typedef struct {fixture *x;unsigned entered;} nested_context;
static cs_m68k_result nested(void *context,cs_trap_frame *f,cs_m68k_bus_report *r,const cs_m68k_stack_limits *limits)
{
    nested_context *c=context;fixture *x=c->x;uint32_t pc=f->pc;
    cs_m68k_result result=cs_tb_step(&x->toolbox,f,r,limits);
    if(result==CS_M68K_OK && pc==0x6000 && x->session.calls.depth==1) {
        cs_m68k_call_request q={0x6040,0x1810,16,CS_M68K_PROC0,CS_M68K_CALL_ORDINARY};cs_m68k_call_report call;
        cs_m68k_call_result called=cs_m68k_session_call(&x->session,&q,f,&call);c->entered=call.entered;
        if(called!=CS_M68K_CALL_OK) {r->cpu.status=CS_M68K_SERVICE;r->memory=call.memory;return CS_M68K_SERVICE;}
    }
    return result;
}
static void sessions(void)
{
    fixture x;setup(&x);const uint8_t bytes[]={CS_TOOLBOX_PROGRAM_BYTES};REQUIRE(sizeof bytes==30);
    memcpy(x.code,bytes,sizeof bytes);x.frame.a[7]=0x8100;cs_trap_frame before=x.frame;cs_m68k_session_report r;
    REQUIRE(run(&x,8,&r)==CS_M68K_SESSION_RETURNED && r.steps==8);
    REQUIRE(r.guest.d[0]==(0x1234u^0x89ABCDEFu^1u^0x42u) && r.guest.a[7]==0x8100 && r.guest.pc==0x400180);
    REQUIRE(!memcmp(&x.frame,&before,sizeof before) && x.session.calls.depth==0);
    REQUIRE(run(&x,7,&r)==CS_M68K_SESSION_BUDGET && r.steps==7 && !memcmp(&x.frame,&before,sizeof before));
    setup(&x);word(x.code,0xA9F4);x.frame.a[7]=0x8100;before=x.frame;
    REQUIRE(run(&x,1,&r)==CS_M68K_SESSION_EXITED && r.steps==1 && r.exit_code==0 && r.cpu==CS_M68K_OK);
    REQUIRE(r.guest.pc==0x6002 && r.guest.a[7]==0x80FC && !memcmp(&x.frame,&before,sizeof before));
    word(x.code,0x4E75);REQUIRE(run(&x,1,&r)==CS_M68K_SESSION_RETURNED);
    setup(&x);word(x.code,0x613E);word(x.code+2,0x4E75);word(x.code+64,0x3F3C);word(x.code+66,0x2468);word(x.code+68,0xACF2);
    REQUIRE(run(&x,4,&r)==CS_M68K_SESSION_RETURNED && r.steps==4);
    for(unsigned exits=0;exits<2;++exits) {
        setup(&x);word(x.code+2,0x4E75);
        if(exits) {word(x.code+64,0x13FC);word(x.code+66,0xB);longword(x.code+68,0x1808);word(x.code+72,0xA9F4);}
        else word(x.code+64,0x4E75);
        cs_m68k_session_request q={0x6000,0x1810,16,CS_M68K_CALL_ORDINARY};x.frame.a[7]=0x8100;before=x.frame;
        nested_context c={&x,0};
        REQUIRE(cs_m68k_session_run_using(&x.session,&q,&x.frame,&r,nested,&c)==
            (exits?CS_M68K_SESSION_CPU:CS_M68K_SESSION_RETURNED));
        REQUIRE(c.entered==1 && !memcmp(&before,&x.frame,sizeof before) && x.session.calls.depth==0);
        if(exits) REQUIRE(x.data[8]==0xB && x.toolbox.last.status==CS_TB_CONTEXT && r.exit_code==0);
    }
}
static void bindings(void)
{
    fixture x;setup(&x);input_mixed(&x,0,0);REQUIRE(cs_trap_set(&x.gateway,0xF0,0x6040)==CS_TRAP_OK);
    atomic(&x,CS_TB_PATCH);REQUIRE(x.report.cpu.service.status==CS_TRAP_NEEDS_EXECUTION);
    REQUIRE(cs_trap_set(&x.gateway,0xF0,0x400302)==CS_TRAP_OK);atomic(&x,CS_TB_SIGNATURE);
    setup(&x);word(x.code,0xA8FF);atomic(&x,CS_TB_UNKNOWN);
    REQUIRE(x.toolbox.last.convention==CS_TB_CONVENTION_NONE && x.toolbox.last.completion==CS_TB_COMPLETION_NONE);
    setup(&x);word(x.code,0xAAF0);atomic(&x,CS_TB_FLAGS);
    setup(&x);x.frame.pc=0x400300;atomic(&x,CS_TB_DIRECT);
    setup(&x);word(x.code,0xA146);x.frame.d[0]=0xF0;REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0x400300);
    fixture y;setup(&y);input_mixed(&y,0x100,0x42);cs_trap_gateway gx=x.gateway;
    REQUIRE(step(&y)==CS_M68K_OK && !memcmp(&x.gateway,&gx,sizeof gx));
    setup(&x);word(x.code,0xA047);x.frame.d[0]=0xF0;x.frame.a[0]=0x6040;
    REQUIRE(step(&x)==CS_M68K_OK && x.gateway.entries[0xF0]==0x6040);
    x.frame.pc=0x6000;input_mixed(&x,0,0);atomic(&x,CS_TB_PATCH);
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(!strcmp(argv[1],"frames")) frames();else if(!strcmp(argv[1],"admission")) admission();
    else if(!strcmp(argv[1],"spans")) spans();else if(!strcmp(argv[1],"failures")) failures();
    else if(!strcmp(argv[1],"completions")) completions();else if(!strcmp(argv[1],"sessions")) sessions();
    else if(!strcmp(argv[1],"bindings")) bindings();else return 2;
    printf("toolbox %s PASS\n",argv[1]);return 0;
}
