/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_TEST_UART_FIXTURE_H
#define CLASSICK_TEST_UART_FIXTURE_H
#include "../platform/pc/uart.h"
#include <string.h>
/* Independent register model and observable access transcript, no real ports. */
typedef struct {
    uint8_t regs[8],dll,dlm,lsr,iir;
    unsigned operations,reads,writes,violations,bad_offset,bad_read,lsr_reads;
    unsigned startup_count,thre_period; /* nonzero: THRE only on every Nth LSR read */
    uint8_t offsets[32],values[32],directions[32];
    unsigned char output[8192]; size_t output_count;
} uart_fixture;
static void uf_init(uart_fixture *f)
{ memset(f,0,sizeof *f); f->regs[7]=0xD3; f->lsr=0x60; f->iir=0xC1; f->bad_offset=99; }
static void uf_access(uart_fixture *f,unsigned offset,unsigned writing,uint8_t value)
{
    ++f->operations;
    if(offset>7) { ++f->violations; return; }
    if(f->startup_count<20) {
        unsigned i=f->startup_count++; f->offsets[i]=(uint8_t)offset;
        f->directions[i]=(uint8_t)writing; f->values[i]=value;
    }
}
static uint8_t uf_read(void *context,uint16_t port)
{
    uart_fixture *f=context; unsigned offset=(unsigned)port-0x3F8u; uint8_t v=0;
    ++f->reads;
    if(offset<8) {
        if(offset==5) {
            ++f->lsr_reads; v=f->lsr;
            if(f->thre_period!=0 && f->lsr_reads%f->thre_period!=0) v=(uint8_t)(v&~0x60u);
        }
        else if(offset==2) v=f->iir;
        else if((f->regs[3]&0x80u)!=0 && offset<2) v=offset?f->dlm:f->dll;
        else v=f->regs[offset];
    }
    if(offset==f->bad_offset && f->reads==f->bad_read) v^=0xFFu;
    uf_access(f,offset,0,v); return v;
}
static void uf_write(void *context,uint16_t port,uint8_t v)
{
    uart_fixture *f=context; unsigned offset=(unsigned)port-0x3F8u;
    ++f->writes; uf_access(f,offset,1,v);
    if(offset>=8) return;
    if((f->regs[3]&0x80u)!=0 && offset<2) { if(offset) f->dlm=v; else f->dll=v; }
    else if(offset==0) {
        if((f->lsr&0x20u)==0 || f->output_count>=sizeof f->output) ++f->violations;
        else f->output[f->output_count++]=v;
    } else if(offset!=2) f->regs[offset]=v;
}
static cs_uart_io uf_io(uart_fixture *f)
{ cs_uart_io io; io.read=uf_read; io.write=uf_write; io.context=f; return io; }
#endif
