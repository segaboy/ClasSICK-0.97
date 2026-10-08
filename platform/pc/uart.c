/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "uart.h"
static void add(uint32_t *v,uint32_t n) { *v=n>UINT32_MAX-*v?UINT32_MAX:*v+n; }
static int valid(const cs_uart *s)
{
    return s->bytes!=NULL && s->base!=0 && s->base<=65528 && s->divisor!=0
        && s->divisor<=65535 && s->phase<=20 && s->head<512 && s->count<=512
        && s->result<=7 && s->started<=1 && s->closing<=1 && s->waiting<=1
        && (s->result>=3 || (s->phase<20?s->result==0:s->result==1 || s->result==2))
        && (s->result!=2 || (s->closing==1 && s->count==0 && s->temt==1));
}
uint32_t cs_uart_begin(cs_uart *s,unsigned char *bytes,uint32_t base,uint32_t divisor)
{
    if(s==NULL || bytes==NULL || base==0 || base>65528 || divisor==0 || divisor>65535)
        return CS_UART_ARGUMENT;
    s->bytes=bytes; s->base=base; s->divisor=divisor; s->result=0; s->phase=0;
    s->since=0; s->last=0; s->head=0; s->count=0; s->sent=0;
    s->dropped_records=0; s->dropped_bytes=0; s->lsr=0; s->line_bits=0;
    s->started=0; s->closing=0; s->polls=0; s->scratch=0; s->temt=0; s->waiting=0;
    return CS_UART_ACTIVE;
}
uint32_t cs_uart_enqueue(cs_uart *s,const unsigned char *p,size_t n)
{
    if(s==NULL || (n!=0 && p==NULL) || n>512) return CS_UART_ARGUMENT;
    if(!valid(s)) return CS_UART_STATE;
    if(s->result>2) return s->result;
    if(s->closing!=0 || s->result==2) return CS_UART_STATE;
    if(n>512u-s->count) { add(&s->dropped_records,1); add(&s->dropped_bytes,(uint32_t)n); return CS_UART_FULL; }
    if(s->count==0) s->waiting=0;
    for(size_t i=0;i<n;++i) s->bytes[(s->head+s->count+(uint32_t)i)%512u]=p[i];
    s->count+=(uint32_t)n; return s->result;
}
uint32_t cs_uart_finish(cs_uart *s)
{
    if(s==NULL) return CS_UART_ARGUMENT;
    if(!valid(s)) return CS_UART_STATE;
    if(s->result>2) return s->result;
    if(s->closing==0 && s->count==0) s->waiting=0;
    s->closing=1; return s->result;
}
uint32_t cs_uart_poll(cs_uart *s,const cs_uart_io *io,uint64_t now)
{
    uint32_t offset=0,value=0,expected=0;
    int reading=0,checking=0;
    if(s==NULL || io==NULL || io->read==NULL || io->write==NULL) return CS_UART_ARGUMENT;
    if(!valid(s)) { s->result=CS_UART_STATE; return s->result; }
    if(s->result>=2) return s->result;
    if(s->started!=0 && now<s->last) { s->result=CS_UART_STATE; return s->result; }
    s->last=now;
    if(s->started==0) { s->started=1; s->since=now; s->polls=0; }
    if(s->phase==20 && (s->count!=0 || s->closing!=0) && s->waiting==0) {
        s->waiting=1; s->since=now; s->polls=0;
    }
    if(s->phase<20 || s->count!=0 || s->closing!=0) {
        if(now-s->since>=CS_UART_TICKS || s->polls>=CS_UART_POLLS) {
            s->result=CS_UART_TIMEOUT; return s->result;
        }
        ++s->polls;
    } else { s->since=now; s->polls=0; s->waiting=0; }
    if(s->phase<20) {
        switch(s->phase) {
        case 0: offset=3; value=3; break;
        case 1: offset=1; break;
        case 2: offset=2; value=7; break;
        case 3: offset=4; value=3; break;
        case 4: offset=7; reading=1; break;
        case 5: offset=7; value=0xA5; break;
        case 6: offset=7; reading=1; checking=1; expected=0xA5; break;
        case 7: offset=7; value=0x5A; break;
        case 8: offset=7; reading=1; checking=1; expected=0x5A; break;
        case 9: offset=7; value=s->scratch; break;
        case 10: offset=3; value=0x83; break;
        case 11: value=s->divisor&255u; break;
        case 12: offset=1; value=s->divisor>>8; break;
        case 13: reading=1; checking=1; expected=s->divisor&255u; break;
        case 14: offset=1; reading=1; checking=1; expected=s->divisor>>8; break;
        case 15: offset=3; value=3; break;
        case 16: offset=3; reading=1; checking=1; expected=3; break;
        case 17: offset=1; reading=1; checking=1; break;
        case 18: offset=4; reading=1; checking=1; expected=3; break;
        default: offset=2; reading=1; checking=1; expected=0xC0; break;
        }
        if(reading) {
            value=io->read(io->context,(uint16_t)(s->base+offset));
            if(s->phase==4) s->scratch=value;
            if(s->phase==19) value&=0xC0u;
            if(checking && value!=expected) { s->result=CS_UART_DEVICE; return s->result; }
        } else io->write(io->context,(uint16_t)(s->base+offset),(uint8_t)value);
        ++s->phase;
        if(s->phase==20) { s->result=CS_UART_READY; s->since=now; s->polls=0; }
        return s->result;
    }
    s->lsr=io->read(io->context,(uint16_t)(s->base+5u));
    s->line_bits|=s->lsr&0x9Eu; s->temt=(s->lsr>>6)&1u;
    if(s->line_bits!=0) { s->result=CS_UART_LINE; return s->result; }
    if(s->count!=0 && (s->lsr&0x20u)!=0) {
        io->write(io->context,(uint16_t)s->base,s->bytes[s->head]);
        s->head=(s->head+1u)%512u; --s->count; add(&s->sent,1); s->temt=0;
        s->since=now; s->polls=0;
    } else if(s->closing!=0 && s->count==0 && s->temt!=0) s->result=CS_UART_DONE;
    return s->result;
}
