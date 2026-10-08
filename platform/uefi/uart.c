/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "uart.h"
#include "../pc/acpi.h"
#include "../pc/pmtimer.h"
typedef struct {
    cs_uart transport;
    const cs_uart_io *io;
    unsigned char *trace;
    uint32_t result,armed,frames,ready,turns,flush,records;
} logger;
static void increment(uint32_t *v) { if(*v!=UINT32_MAX) ++*v; }
static uint32_t get(const unsigned char *p)
{ return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static void put(unsigned char *p,uint64_t v,unsigned n)
{ for(unsigned i=0;i<n;++i) { p[i]=(unsigned char)v; v>>=8; } }
static void snapshot(logger *d)
{
    const cs_uart *s=&d->transport;
    put(d->trace,CS_UART_TRACE_MAGIC,4); put(d->trace+4,1,4); put(d->trace+8,d->result,4);
    put(d->trace+12,d->armed?s->result:CS_UART_NOT_STARTED,4); put(d->trace+16,s->phase,4);
    put(d->trace+20,s->base,4); put(d->trace+24,s->divisor,4); put(d->trace+28,s->count,4);
    put(d->trace+32,s->head,4); put(d->trace+36,s->sent,4); put(d->trace+40,s->dropped_records,4);
    put(d->trace+44,s->dropped_bytes,4); put(d->trace+48,s->lsr,4); put(d->trace+52,s->line_bits,4);
    put(d->trace+56,s->started,4); put(d->trace+60,s->closing,4); put(d->trace+64,d->turns,4);
    put(d->trace+68,d->flush,4); put(d->trace+72,d->records,4); put(d->trace+76,s->temt,4);
    put(d->trace+80,s->last,8);
    for(unsigned i=88;i<128;++i) d->trace[i]=0;
}
static size_t literal(unsigned char *p,const char *text)
{ size_t n=0; while(text[n]!='\0') { p[n]=(unsigned char)text[n]; ++n; } return n; }
static size_t hex(unsigned char *p,uint32_t v)
{
    for(unsigned i=0;i<8;++i) { uint32_t digit=(v>>(28u-4u*i))&15u; p[i]=(unsigned char)(digit<10?48u+digit:55u+digit); }
    return 8;
}
static void send(logger *d,const unsigned char *p,size_t n)
{ increment(&d->records); (void)cs_uart_enqueue(&d->transport,p,n); }
static void observe(void *context,uint64_t ticks,const unsigned char *trace,uint32_t final)
{
    logger *d=context;
    unsigned char line[80];
    size_t n;
    d->result=get(trace+8);
    if(d->armed==0) {
        if(final!=0) { d->result=get(trace+8); snapshot(d); return; }
        d->armed=1; n=literal(line,"ClasSICK 0.97 UART v1\r\n"); send(d,line,n);
    }
    if(final==0) increment(&d->turns);
    if(d->ready==0 && get(trace+12)==CS_PS2_READY) {
        d->ready=1; n=literal(line,"keyboard ready\r\n"); send(d,line,n);
    }
    if(get(trace+32)!=d->frames) {
        d->frames=get(trace+32); n=literal(line,"frame="); n+=hex(line+n,get(trace+76));
        n+=literal(line+n," sec="); n+=hex(line+n,get(trace+24));
        n+=literal(line+n," space="); n+=hex(line+n,get(trace+36)); n+=literal(line+n,"\r\n"); send(d,line,n);
    }
    if(final==1) {
        d->result=get(trace+8); n=literal(line,"result="); n+=hex(line+n,d->result);
        n+=literal(line+n," kbd="); n+=hex(line+n,get(trace+12)); n+=literal(line+n,"\r\n"); send(d,line,n);
        (void)cs_uart_finish(&d->transport);
    } else if(final==0) (void)cs_uart_poll(&d->transport,d->io,ticks);
    snapshot(d);
}
uint32_t cs_native_uart_loop(const cs_uefi_handoff *h,uint32_t ready,
    const cs_native_devices *dev,const cs_ps2_io *keyboard,const cs_uart_io *io,
    uint32_t base,uint32_t divisor,uint32_t seconds,unsigned char *keyboard_trace,unsigned char *trace)
{
    logger d;
    cs_acpi_pm_timer timer;
    cs_pmtimer counter;
    uint32_t delta,result;
    uint64_t start;
    if(trace==NULL) return CS_KBD_ARGUMENT;
    for(unsigned i=0;i<CS_UART_TRACE_BYTES;++i) trace[i]=0;
    /* Safe default state also covers rejected UART configuration, without I/O. */
    (void)cs_uart_begin(&d.transport,trace+128,0x3F8,12);
    d.io=io; d.trace=trace; d.result=CS_KBD_ARGUMENT; d.armed=0; d.frames=0; d.ready=0;
    d.turns=0; d.flush=0; d.records=0;
    if(io==NULL || io->read==NULL || io->write==NULL
            || cs_uart_begin(&d.transport,trace+128,base,divisor)==CS_UART_ARGUMENT
            || keyboard_trace==NULL) { snapshot(&d); return CS_KBD_ARGUMENT; }
    result=cs_native_keyboard_observed(h,ready,dev,keyboard,seconds,keyboard_trace,observe,&d);
    d.result=result;
    if(d.armed!=0 && d.transport.result<CS_UART_DONE) {
        if(cs_acpi_find_pm_timer(cs_native_read,(void *)(uintptr_t)dev->memory,h->rsdp,&timer)!=CS_ACPI_OK
                || cs_pmtimer_init(&counter,timer.bits,dev->port(dev->port_context,timer.port))!=CS_PMTIMER_OK)
            d.transport.result=CS_UART_STATE;
        else {
            start=d.transport.last;
            for(unsigned i=0;i<CS_UART_POLLS && d.transport.result<CS_UART_DONE;++i) {
                increment(&d.flush);
                if(cs_pmtimer_sample(&counter,dev->port(dev->port_context,timer.port),&delta)!=CS_PMTIMER_OK
                        || counter.ticks>=CS_UART_TICKS || counter.ticks>UINT64_MAX-start) {
                    d.transport.result=CS_UART_TIMEOUT; break;
                }
                (void)cs_uart_poll(&d.transport,io,start+counter.ticks);
            }
            if(d.transport.result<CS_UART_DONE) d.transport.result=CS_UART_TIMEOUT;
        }
    }
    snapshot(&d); return result;
}
