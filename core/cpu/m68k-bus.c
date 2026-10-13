/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-bus.h"
typedef struct {
    const cs_guest_map *map; cs_trap_frame frame; cs_m68k_bus_report *report;
    uint32_t cursor; uint8_t *store; uint8_t data[6]; unsigned store_bytes;
    const cs_m68k_bus_options *options;
} transaction;
static void copy_frame(cs_trap_frame *to,const cs_trap_frame *from)
{
    for(unsigned i=0;i<8;++i) {to->d[i]=from->d[i];to->a[i]=from->a[i];}
    to->pc=from->pc;to->sr=from->sr;
}
static void begin(cs_m68k_bus_report *r,const cs_trap_frame *f)
{
    r->cpu.status=CS_M68K_OK;r->cpu.pc=f->pc;r->cpu.word=0;r->cpu.service_called=0;
    r->cpu.service.status=CS_TRAP_OK;r->cpu.service.target=0;r->cpu.service.pc=f->pc;
    r->cpu.service.decoded.word=0;r->cpu.service.decoded.number=0;
    r->cpu.service.decoded.toolbox=0;r->cpu.service.decoded.a0_result=0;
    r->cpu.service.decoded.auto_pop=0;r->cpu.service.decoded.reserved=0;
    r->cpu.service.decoded.flags=0;r->memory=CS_GUEST_OK;r->address=0;
    r->access=0;r->bytes=0;r->branch_taken=0;r->subroutine=0;
}
static cs_m68k_result done(cs_m68k_bus_report *r,cs_m68k_result status)
{ r->cpu.status=status;return status; }
static cs_m68k_result read_value(transaction *t,uint32_t at,unsigned bytes,
    uint32_t access,uint32_t *value)
{
    cs_m68k_bus_report *r=t->report;r->address=at;r->bytes=bytes;r->access=access;
    r->memory=cs_guest_read(t->map,at,bytes,access,value);
    return r->memory==CS_GUEST_OK?CS_M68K_OK:CS_M68K_ADDRESS;
}
static cs_m68k_result extension(transaction *t,unsigned bytes,uint32_t *value)
{
    cs_m68k_result status=read_value(t,t->cursor,bytes,CS_GUEST_READ|CS_GUEST_EXECUTE,value);
    if(status==CS_M68K_OK) t->cursor+=bytes;
    return status;
}
static uint32_t mask(unsigned width)
{ return width==1?0xFFu:width==2?0xFFFFu:UINT32_MAX; }
static void flags(cs_trap_frame *f,uint32_t value,unsigned width)
{
    value&=mask(width);
    uint32_t sign=width==1?0x80u:width==2?0x8000u:0x80000000u;
    f->sr=(uint16_t)((f->sr&0xFFF0u)|(value==0?4u:((value&sign)?8u:0u)));
}
static cs_m68k_result effective(transaction *t,unsigned mode,unsigned reg,unsigned width,uint32_t *at)
{
    uint32_t step=width==1 && reg==7?2u:width;
    if(mode>=2 && mode<=4) {
        if(mode==4) t->frame.a[reg]-=step;
        *at=t->frame.a[reg];
        if(mode==3) t->frame.a[reg]+=step;
        return CS_M68K_OK;
    }
    if(mode==7 && reg==1) return extension(t,4,at);
    return CS_M68K_OPCODE;
}
static cs_m68k_result source(transaction *t,unsigned mode,unsigned reg,unsigned width,uint32_t *value)
{
    if(mode==0) {*value=t->frame.d[reg]&mask(width);return CS_M68K_OK;}
    if(mode==1) {
        if(width==1) return CS_M68K_OPCODE;
        *value=t->frame.a[reg]&mask(width);return CS_M68K_OK;
    }
    if(mode==7 && reg==4) {
        cs_m68k_result status=extension(t,width==4?4u:2u,value);
        if(status==CS_M68K_OK) *value&=mask(width);
        return status;
    }
    uint32_t at;cs_m68k_result status=effective(t,mode,reg,width,&at);
    if(status!=CS_M68K_OK) return status;
    if(t->options!=0 && t->options->data_policy==CS_M68K_DATA24) at&=0xFFFFFFu;
    return read_value(t,at,width,CS_GUEST_READ,value);
}
static cs_m68k_result preflight_store(transaction *t,uint32_t at,unsigned width,uint32_t value)
{
    cs_m68k_bus_report *r=t->report;cs_guest_view view;
    r->address=at;r->bytes=width;r->access=CS_GUEST_WRITE;
    if(width!=1 && (at&1u)!=0) {r->memory=CS_GUEST_ADDRESS;return CS_M68K_ADDRESS;}
    r->memory=cs_guest_window(t->map,at,width,CS_GUEST_WRITE,&view);
    if(r->memory!=CS_GUEST_OK) return CS_M68K_ADDRESS;
    t->store=view.writable;t->store_bytes=width;
    for(unsigned i=0;i<width;++i) t->data[i]=(uint8_t)(value>>((width-1u-i)*8u));
    return CS_M68K_OK;
}
static cs_m68k_result move(transaction *t,uint16_t word)
{
    unsigned kind=word>>12,width=kind==1?1u:kind==2?4u:2u;
    unsigned sm=(word>>3)&7u,sr=word&7u,dm=(word>>6)&7u,dr=(word>>9)&7u;
    if(width==1 && (sm==1 || dm==1)) return CS_M68K_OPCODE;
    uint32_t value;cs_m68k_result status=source(t,sm,sr,width,&value);
    if(status!=CS_M68K_OK) return status;
    if(dm==1) {
        if(width==2 && (value&0x8000u)!=0) value|=0xFFFF0000u;
        t->frame.a[dr]=value;return CS_M68K_OK;
    }
    if(dm==0) t->frame.d[dr]=(t->frame.d[dr]&~mask(width))|value;
    else {
        uint32_t at;status=effective(t,dm,dr,width,&at);
        if(status!=CS_M68K_OK) return status;
        if(t->options!=0 && t->options->data_policy==CS_M68K_DATA24) at&=0xFFFFFFu;
        status=preflight_store(t,at,width,value);
        if(status!=CS_M68K_OK) return status;
    }
    flags(&t->frame,value,width);return CS_M68K_OK;
}
static int condition(unsigned c,uint16_t sr)
{
    int n=(sr&8u)!=0,z=(sr&4u)!=0,v=(sr&2u)!=0,carry=(sr&1u)!=0;
    switch(c) {
        case 0:case 1:return 1;
        case 2:return !carry && !z;case 3:return carry || z;
        case 4:return !carry;case 5:return carry;case 6:return !z;case 7:return z;
        case 8:return !v;case 9:return v;case 10:return !n;case 11:return n;
        case 12:return n==v;case 13:return n!=v;
        case 14:return !z && n==v;default:return z || n!=v;
    }
}
static cs_m68k_result target(transaction *t,uint32_t at)
{
    uint32_t unused;
    return read_value(t,at,2,CS_GUEST_READ|CS_GUEST_EXECUTE,&unused);
}
static cs_m68k_result branch(transaction *t,uint16_t word)
{
    unsigned c=(word>>8)&15u;uint32_t displacement=word&0xFFu,sign=0x80u,range=0x100u;
    if(displacement==0) {
        cs_m68k_result status=extension(t,2,&displacement);
        if(status!=CS_M68K_OK) return status;
        sign=0x8000u;range=0x10000u;
    }
    if(!condition(c,t->frame.sr)) {t->frame.pc=t->cursor;return CS_M68K_OK;}
    uint32_t at=t->frame.pc+2;
    if((displacement&sign)!=0) {
        uint32_t magnitude=range-displacement;
        if(at<magnitude) {t->report->memory=CS_GUEST_ADDRESS;return CS_M68K_ADDRESS;}
        at-=magnitude;
    } else {
        if(displacement>0xFFFFFFu-at) {t->report->memory=CS_GUEST_ADDRESS;return CS_M68K_ADDRESS;}
        at+=displacement;
    }
    cs_m68k_result status=target(t,at);
    if(status!=CS_M68K_OK) return status;
    if(c==1) {
        if(t->frame.a[7]<4) {t->report->memory=CS_GUEST_ADDRESS;return CS_M68K_ADDRESS;}
        status=preflight_store(t,t->frame.a[7]-4,4,t->cursor);
        if(status!=CS_M68K_OK) return status;
        t->frame.a[7]-=4;t->report->subroutine=1;
    }
    t->frame.pc=at;t->report->branch_taken=1;return CS_M68K_OK;
}
static cs_m68k_result return_instruction(transaction *t)
{
    uint32_t at;cs_m68k_result status=read_value(t,t->frame.a[7],4,CS_GUEST_READ,&at);
    if(status!=CS_M68K_OK) return status;
    status=target(t,at);
    if(status!=CS_M68K_OK) return status;
    t->frame.pc=at;t->frame.a[7]+=4;return CS_M68K_OK;
}
static cs_m68k_result aline(transaction *t,cs_trap_gateway *g,uint16_t word)
{
    cs_m68k_bus_report *r=t->report;cs_guest_view view;
    if(t->frame.a[7]<6 || (t->frame.a[7]&1u)!=0) {
        r->memory=CS_GUEST_ADDRESS;return CS_M68K_ADDRESS;
    }
    r->address=t->frame.a[7]-6;r->bytes=6;r->access=CS_GUEST_WRITE;
    r->memory=cs_guest_window(t->map,r->address,6,CS_GUEST_WRITE,&view);
    if(r->memory!=CS_GUEST_OK) return CS_M68K_ADDRESS;
    cs_m68k_result status;
    if(t->options!=0 && t->options->dispatch!=0) {
        r->cpu.service_called=1;
        cs_trap_result result=t->options->dispatch(t->options->context,word,&t->frame,&r->cpu.service);
        status=result==CS_TRAP_OK?CS_M68K_OK:CS_M68K_SERVICE;
        if(status==CS_M68K_OK) t->frame.pc+=2;
    } else status=cs_m68k_aline(g,word,&t->frame,&r->cpu);
    if(status!=CS_M68K_OK) return status;
    t->store=view.writable;t->store_bytes=6;
    t->data[0]=(uint8_t)(t->frame.sr>>8);t->data[1]=(uint8_t)t->frame.sr;
    for(unsigned i=0;i<4;++i) t->data[i+2]=(uint8_t)(t->frame.pc>>((3u-i)*8u));
    return CS_M68K_OK;
}
cs_m68k_result cs_m68k_bus_step_options(const cs_guest_map *m,cs_trap_gateway *g,
    cs_trap_frame *f,cs_m68k_bus_report *r,const cs_m68k_bus_options *options)
{
    if(m==0 || g==0 || f==0 || r==0) return CS_M68K_ARGUMENT;
    if(options!=0 && options->data_policy!=CS_M68K_STRICT && options->data_policy!=CS_M68K_DATA24)
        return CS_M68K_ARGUMENT;
    begin(r,f);r->memory=cs_guest_validate(m);
    if(r->memory!=CS_GUEST_OK) return done(r,CS_M68K_ADDRESS);
    if((f->pc&1u)!=0 || f->pc>0xFFFFFCu) return done(r,CS_M68K_ADDRESS);
    if((f->sr&0xA000u)!=0x2000u) return done(r,CS_M68K_MODE);
    transaction t;t.map=m;t.report=r;t.cursor=f->pc;t.store=0;t.store_bytes=0;
    t.options=options;
    copy_frame(&t.frame,f);
    uint32_t fetched;cs_m68k_result status=extension(&t,2,&fetched);
    if(status!=CS_M68K_OK) return done(r,status);
    uint16_t word=(uint16_t)fetched;r->cpu.word=word;
    unsigned kind=word>>12;
    if(kind>=1 && kind<=3) {status=move(&t,word);t.frame.pc=t.cursor;}
    else if(kind==6) status=branch(&t,word);
    else if((word&0xF100u)==0x7000u) {
        uint32_t value=word&0xFFu;
        if(value>=128) value|=0xFFFFFF00u;
        t.frame.d[(word>>9)&7u]=value;flags(&t.frame,value,4);t.frame.pc=t.cursor;
    }
    else if(word==0x4E71u) t.frame.pc=t.cursor;
    else if(word==0x4E75u) status=return_instruction(&t);
    else if(kind==10) status=aline(&t,g,word);
    else status=CS_M68K_OPCODE;
    if(status!=CS_M68K_OK) return done(r,status);
    /* A-line's successful service is the final fallible operation. Other
     * instructions still check their computed continuation before any write. */
    if(t.frame.pc>0xFFFFFEu || (t.frame.pc&1u)!=0) return done(r,CS_M68K_ADDRESS);
    for(unsigned i=0;i<t.store_bytes;++i) t.store[i]=t.data[i];
    copy_frame(f,&t.frame);return done(r,CS_M68K_OK);
}
cs_m68k_result cs_m68k_bus_step(const cs_guest_map *m,cs_trap_gateway *g,
    cs_trap_frame *f,cs_m68k_bus_report *r)
{return cs_m68k_bus_step_options(m,g,f,r,0);}
