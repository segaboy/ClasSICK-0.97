/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k.h"
static int mode(uint16_t sr) { return (sr&0xA000u)==0x2000u; }
static int span(uint32_t base,uint32_t size)
{ return (base&1u)==0 && (size&1u)==0 && size!=0 && base<0x1000000u && size<=0x1000000u-base; }
static int contains(uint32_t base,uint32_t size,uint32_t at,uint32_t bytes)
{ return at>=base && bytes<=size && at-base<=size-bytes && (at&1u)==0; }
static void begin(cs_m68k_report *r,const cs_trap_frame *f,uint16_t word)
{
    r->status=CS_M68K_OK;r->pc=f->pc;r->word=word;r->service_called=0;
    r->service.status=CS_TRAP_OK;r->service.target=0;r->service.pc=f->pc;
    r->service.decoded.word=0;r->service.decoded.number=0;r->service.decoded.toolbox=0;
    r->service.decoded.a0_result=0;r->service.decoded.auto_pop=0;
    r->service.decoded.reserved=0;r->service.decoded.flags=0;
}
static cs_m68k_result finish(cs_m68k_report *r,cs_m68k_result status)
{ r->status=status;return status; }
cs_m68k_result cs_m68k_aline(cs_trap_gateway *g,uint16_t word,cs_trap_frame *f,cs_m68k_report *r)
{
    if(g==0 || f==0 || r==0) return CS_M68K_ARGUMENT;
    begin(r,f,word);
    if((word&0xF000u)!=0xA000u) return finish(r,CS_M68K_OPCODE);
    if((f->pc&1u)!=0 || f->pc>0xFFFFFCu) return finish(r,CS_M68K_ADDRESS);
    if(!mode(f->sr)) return finish(r,CS_M68K_MODE);
    r->service_called=1;
    if(cs_trap_call(g,word,f,&r->service)!=CS_TRAP_OK) return finish(r,CS_M68K_SERVICE);
    f->pc+=2;return CS_M68K_OK;
}
static uint32_t read32(const uint8_t *p)
{ return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }
static void write32(uint8_t *p,uint32_t v)
{ p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v; }
cs_m68k_result cs_m68k_step(const cs_m68k_memory *m,cs_trap_gateway *g,cs_trap_frame *f,cs_m68k_report *r)
{
    if(m==0 || g==0 || f==0 || r==0) return CS_M68K_ARGUMENT;
    begin(r,f,0);
    if(m->code==0 || m->stack==0 || !span(m->code_base,m->code_size) || !span(m->stack_base,m->stack_size) ||
        !(m->code_base+m->code_size<=m->stack_base || m->stack_base+m->stack_size<=m->code_base))
        return finish(r,CS_M68K_ADDRESS);
    if(!contains(m->code_base,m->code_size,f->pc,2) || f->pc>0xFFFFFCu) return finish(r,CS_M68K_ADDRESS);
    if(!mode(f->sr)) return finish(r,CS_M68K_MODE);
    const uint8_t *p=m->code+(f->pc-m->code_base);
    uint16_t word=(uint16_t)(((uint16_t)p[0]<<8)|p[1]);r->word=word;
    if((word&0xF100u)==0x7000u) {
        unsigned reg=(word>>9)&7u;uint32_t value=word&0xFFu;
        if(value>=128u) value|=0xFFFFFF00u;
        f->d[reg]=value;f->sr=(uint16_t)((f->sr&0xFFF0u)|(value==0?4u:((value&0x80000000u)?8u:0u)));
        f->pc+=2;return CS_M68K_OK;
    }
    if(word==0x4E71u) {f->pc+=2;return CS_M68K_OK;}
    if(word==0x4E75u) {
        if(!contains(m->stack_base,m->stack_size,f->a[7],4)) return finish(r,CS_M68K_ADDRESS);
        uint32_t target=read32(m->stack+(f->a[7]-m->stack_base));
        if(!contains(m->code_base,m->code_size,target,2)) return finish(r,CS_M68K_ADDRESS);
        f->pc=target;f->a[7]+=4;return CS_M68K_OK;
    }
    if((word&0xF000u)==0xA000u) {
        if(f->a[7]<6 || !contains(m->stack_base,m->stack_size,f->a[7]-6,6)) return finish(r,CS_M68K_ADDRESS);
        uint32_t at=f->a[7]-6;
        cs_m68k_result status=cs_m68k_aline(g,word,f,r);
        if(status!=CS_M68K_OK) return status;
        uint8_t *saved=m->stack+(at-m->stack_base);
        saved[0]=(uint8_t)(f->sr>>8);saved[1]=(uint8_t)f->sr;write32(saved+2,f->pc);
        return CS_M68K_OK;
    }
    return finish(r,CS_M68K_OPCODE);
}
