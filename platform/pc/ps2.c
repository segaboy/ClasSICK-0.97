/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "ps2.h"
static void increment(uint32_t *value) { if(*value!=UINT32_MAX) ++*value; }
static uint32_t fail(cs_ps2 *s,uint32_t result) { s->result=result; return result; }
static void advance(cs_ps2 *s,uint64_t now)
{
    ++s->phase; s->since=now; s->polls=0;
    if(s->phase==CS_PS2_PHASE_READY) s->result=CS_PS2_READY;
}
uint32_t cs_ps2_begin(cs_ps2 *s)
{
    if(s==NULL) return CS_PS2_ARGUMENT;
    s->since=0; s->last_tick=0; s->phase=0; s->result=CS_PS2_ACTIVE;
    s->polls=0; s->started=0; s->configuration=0; s->retries=0;
    s->drained=0; s->auxiliary=0; s->errors=0; s->dropped=0; s->resends=0; s->events=0;
    s->held=0; s->release=0; s->extended=0; s->pause=0; s->last_byte=0;
    s->port_before=CS_PS2_PORT_UNREAD; s->port_after=CS_PS2_PORT_UNREAD;
    return CS_PS2_ACTIVE;
}
static void clear_keys(cs_ps2 *s)
{
    s->held=0; s->release=0; s->extended=0; s->pause=0;
}
static uint32_t scan(cs_ps2 *s,uint8_t byte,cs_input_queue *q)
{
    uint32_t key,bit;
    cs_input_event event;
    cs_input_result result;
    if(byte==0 || byte==0xFF || byte==0xAA || byte==0xFC || byte==0xFD
            || byte==0xFA || byte==0xFE) {
        increment(&s->errors); clear_keys(s); return CS_PS2_READY;
    }
    if(s->pause!=0) { --s->pause; return CS_PS2_READY; }
    if(byte==0xE1) { s->pause=7; s->release=0; s->extended=0; return CS_PS2_READY; }
    if(byte==0xE0) { s->extended=1; s->release=0; return CS_PS2_READY; }
    if(byte==0xF0) { s->release=1; return CS_PS2_READY; }
    key=byte==0x29?CS_KEY_SPACE:(byte==0x76?CS_KEY_ESCAPE:0u);
    if(s->extended!=0 || key==0) { s->extended=0; s->release=0; return CS_PS2_READY; }
    bit=key==CS_KEY_SPACE?1u:2u;
    event.source=3; event.key=key;
    event.action=s->release!=0?CS_KEY_RELEASE:CS_KEY_PRESS;
    event.repeat=s->release==0 && (s->held&bit)!=0?1u:0u;
    s->release=0;
    if(event.action==CS_KEY_RELEASE) {
        if((s->held&bit)==0) return CS_PS2_READY;
        s->held&=~bit;
    } else s->held|=bit;
    result=cs_input_push(q,&event);
    if(result==CS_INPUT_ERR_FULL) increment(&s->dropped);
    else if(result!=CS_INPUT_OK) return fail(s,CS_PS2_QUEUE);
    else increment(&s->events);
    return CS_PS2_READY;
}
uint32_t cs_ps2_poll(cs_ps2 *s,const cs_ps2_io *io,uint64_t now,cs_input_queue *q)
{
    uint8_t status,byte=0,command=0;
    uint16_t port=0;
    uint32_t phase,limit;
    int drain,reply,ack;
    if(s==NULL || io==NULL || io->read==NULL || io->write==NULL || q==NULL)
        return CS_PS2_ARGUMENT;
    if(s->phase>CS_PS2_PHASE_READY || s->result>CS_PS2_MACHINE) return fail(s,CS_PS2_STATE);
    if(s->result>CS_PS2_READY) return s->result;
    if(s->started!=0 && now<s->last_tick) return fail(s,CS_PS2_STATE);
    if(s->started==0) { s->started=1; s->since=now; }
    s->last_tick=now; phase=s->phase;
    if(phase<CS_PS2_PHASE_READY) {
        limit=phase==25?CS_PS2_BAT_TICKS:(phase==10?CS_PS2_TEST_TICKS:CS_PS2_SHORT_TICKS);
        if(s->polls>=CS_PS2_PHASE_POLLS || now-s->since>=limit) {
            /* SPEC-0016: a missing D0 reply leaves the port unread; startup continues. */
            if(phase==8 || phase==15) { advance(s,now); return s->result; }
            return fail(s,CS_PS2_TIMEOUT);
        }
        ++s->polls;
    }
    status=io->read(io->context,0x64);
    drain=phase==2 || phase==13;
    reply=phase==4 || phase==8 || phase==10 || phase==15 || phase==19 || phase==21 || phase==25;
    ack=phase==24 || phase==27 || phase==29 || phase==31 || phase==33;
    if(drain || reply || ack || phase==CS_PS2_PHASE_READY) {
        if((status&1u)==0) {
            if(drain) { s->retries=0; advance(s,now); }
            return s->result;
        }
        if(drain && s->retries>=64u) return fail(s,CS_PS2_CONTROLLER);
        byte=io->read(io->context,0x60); s->last_byte=byte;
        if(drain) { ++s->retries; increment(&s->drained); return s->result; }
        if((status&0xC0u)!=0) {
            increment(&s->errors);
            if(phase==CS_PS2_PHASE_READY) { clear_keys(s); return CS_PS2_READY; }
            return fail(s,CS_PS2_CONTROLLER);
        }
        if((status&0x20u)!=0) { increment(&s->auxiliary); return s->result; }
        if(phase==CS_PS2_PHASE_READY) return scan(s,byte,q);
        if(phase==4) s->configuration=(byte&4u)|0x30u;
        else if(phase==8) s->port_before=byte;
        else if(phase==15) {
            /* SPEC-0016: self-test must preserve output-port System Reset and Gate A20. */
            s->port_after=byte;
            if(s->port_before<CS_PS2_PORT_UNREAD && ((s->port_before^byte)&3u)!=0)
                return fail(s,CS_PS2_MACHINE);
        }
        else if(phase==19) { if((byte&0x73u)!=0x30u) return fail(s,CS_PS2_CONTROLLER); }
        else if(phase==10) { if(byte!=0x55) return fail(s,CS_PS2_CONTROLLER); }
        else if(phase==21) { if(byte!=0) return fail(s,CS_PS2_CONTROLLER); }
        else if(phase==25) { if(byte!=0xAA) return fail(s,CS_PS2_DEVICE); }
        else if(ack) {
            if(byte==0xFE) {
                if(s->retries>=2u) return fail(s,CS_PS2_DEVICE);
                ++s->retries; increment(&s->resends); --s->phase;
                s->since=now; s->polls=0; return CS_PS2_ACTIVE;
            }
            if(byte==0xFC || byte==0xFD || byte==0 || byte==0xFF) return fail(s,CS_PS2_DEVICE);
            if(byte!=0xFA) return fail(s,CS_PS2_PROTOCOL);
            s->retries=0;
        }
        advance(s,now); return s->result;
    }
    if((status&2u)!=0) return CS_PS2_ACTIVE;
    /* Original table-free sequence: no jump or function-pointer tables. */
    if(phase==0 || phase==11) command=0xAD;
    else if(phase==1 || phase==12) command=0xA7;
    else if(phase==3 || phase==18) command=0x20;
    else if(phase==5 || phase==16) command=0x60;
    else if(phase==7 || phase==14) command=0xD0;
    else if(phase==9) command=0xAA;
    else if(phase==20) command=0xAB;
    else if(phase==22) command=0xAE;
    else {
        port=0x60;
        if(phase==6 || phase==17) command=(uint8_t)s->configuration;
        else if(phase==23) command=0xFF;
        else if(phase==26) command=0xF5;
        else if(phase==28) command=0xF0;
        else if(phase==30) command=2;
        else if(phase==32) command=0xF4;
        else return fail(s,CS_PS2_STATE);
    }
    io->write(io->context,port==0?0x64:port,command);
    advance(s,now); return s->result;
}
