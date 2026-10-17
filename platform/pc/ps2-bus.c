/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "ps2-bus.h"
static void increment(uint32_t *value) { if(*value!=UINT32_MAX) ++*value; }
static uint32_t fail(cs_ps2_bus *b,uint32_t result) { b->result=result; return result; }
static void enter(cs_ps2_bus *b,uint32_t phase,uint64_t now)
{
    b->phase=phase; b->since=now; b->polls=0;
    if(phase==CS_PS2_BUS_READY) b->result=CS_PS2_READY;
}
uint32_t cs_ps2_bus_begin(cs_ps2_bus *b)
{
    if(b==NULL) return CS_PS2_ARGUMENT;
    (void)cs_ps2_begin(&b->keyboard);
    (void)cs_ps2_mouse_begin(&b->decoder);
    (void)cs_mouse_button_begin(&b->left,0);
    b->report.dx=0; b->report.dy=0; b->report.buttons=0;
    b->since=0; b->last_tick=0; b->phase=0; b->result=CS_PS2_ACTIVE;
    b->polls=0; b->retries=0; b->drained=0; b->packets=0; b->errors=0;
    b->auxiliary=0; b->resends=0; b->configuration=0; b->last_byte=0;
    b->started=0; b->synced=1; /* startup buttons are a released baseline */
    return CS_PS2_ACTIVE;
}
/* Write phases: one 0x64 command or one 0x60 mouse payload (always after D4). */
static int command(uint32_t phase,uint16_t *port,uint8_t *value)
{
    *port=0x64;
    if(phase==1) *value=0xAD;
    else if(phase==2) *value=0xA7;
    else if(phase==4) *value=0xA9;
    else if(phase==6) *value=0xA8;
    else if(phase==7 || phase==10 || phase==15 || phase==20) *value=0xD4;
    else if(phase==18) *value=0x20;
    else if(phase==23) *value=0xAE;
    else {
        *port=0x60;
        if(phase==8) *value=0xF5;
        else if(phase==11) *value=0xFF;
        else if(phase==16) *value=0xF6;
        else if(phase==21) *value=0xF4;
        else return 0;
    }
    return 1;
}
static int awaits_aux(uint32_t phase)
{
    return phase==9 || phase==12 || phase==13 || phase==14 || phase==17 || phase==22;
}
/* One AUX byte in an AUX wait phase: ACK, BAT or ID. */
static uint32_t answer(cs_ps2_bus *b,uint32_t phase,uint8_t byte,uint64_t now)
{
    if(phase==13) {
        if(byte!=0xAA) return fail(b,CS_PS2_DEVICE);
        enter(b,14,now); return b->result;
    }
    if(phase==14) {
        if(byte!=0) return fail(b,CS_PS2_DEVICE);
        enter(b,15,now); return b->result;
    }
    if(byte==0xFE) {
        /* Resend the same mouse byte, D4 prefix included: 3 sends at most. */
        if(b->retries>=2u) return fail(b,CS_PS2_DEVICE);
        ++b->retries; increment(&b->resends);
        enter(b,phase-2u,now); return b->result;
    }
    if(byte==0xFC || byte==0xFD || byte==0 || byte==0xFF) return fail(b,CS_PS2_DEVICE);
    if(byte!=0xFA) return fail(b,CS_PS2_PROTOCOL);
    b->retries=0; enter(b,phase+1u,now); return b->result;
}
/* Recovery after corruption or a dropped report: cancel left, unknown baseline. */
static void lose(cs_ps2_bus *b)
{
    (void)cs_mouse_button_sample(&b->left,0);
    b->synced=0;
}
static uint32_t ready(cs_ps2_bus *b,const cs_ps2_io *io,uint8_t status,cs_input_queue *q)
{
    cs_ps2_mouse_packet packet;
    uint8_t byte;
    uint32_t result;
    int fed;
    if((status&1u)==0) return CS_PS2_READY;
    byte=io->read(io->context,0x60); b->last_byte=byte;
    if((status&0x20u)==0) {
        result=cs_ps2_receive(&b->keyboard,status,byte,q);
        return result>CS_PS2_READY?fail(b,result):CS_PS2_READY;
    }
    increment(&b->auxiliary);
    if((status&0xC0u)!=0) {
        increment(&b->errors);
        (void)cs_ps2_mouse_feed(&b->decoder,byte,1,&packet);
        lose(b); return CS_PS2_READY;
    }
    /* FA/FE/AA are report payload here. Sync after unreported loss is unprovable. */
    fed=cs_ps2_mouse_feed(&b->decoder,byte,0,&packet);
    if(fed==CS_PS2_MOUSE_MORE) return CS_PS2_READY;
    if(fed==CS_PS2_MOUSE_DROPPED) { increment(&b->errors); lose(b); return CS_PS2_READY; }
    if(fed!=CS_PS2_MOUSE_PACKET) return fail(b,CS_PS2_STATE);
    b->report=packet; increment(&b->packets);
    if(b->synced==0) { b->left.down=packet.buttons&1u; b->synced=1; return CS_PS2_READY; }
    if(cs_mouse_button_sample(&b->left,packet.buttons&1u)>CS_MOUSE_CHANGED) return fail(b,CS_PS2_STATE);
    return CS_PS2_READY;
}
uint32_t cs_ps2_bus_poll(cs_ps2_bus *b,const cs_ps2_io *io,uint64_t now,cs_input_queue *q)
{
    uint8_t status,byte,value=0;
    uint16_t port=0;
    uint32_t phase,result,limit;
    if(b==NULL || io==NULL || io->read==NULL || io->write==NULL || q==NULL)
        return CS_PS2_ARGUMENT;
    if(b->phase>CS_PS2_BUS_READY || b->result>CS_PS2_MACHINE || b->synced>1u)
        return fail(b,CS_PS2_STATE);
    if(b->result>CS_PS2_READY) return b->result;
    if((b->phase==CS_PS2_BUS_READY)!=(b->result==CS_PS2_READY)) return fail(b,CS_PS2_STATE);
    if(b->started!=0 && now<b->last_tick) return fail(b,CS_PS2_STATE);
    if(b->started==0) { b->started=1; b->since=now; }
    b->last_tick=now; phase=b->phase;
    if(phase==0) {
        /* Unchanged SPEC-0012/0016 keyboard startup, one delegated turn. */
        result=cs_ps2_poll(&b->keyboard,io,now,q);
        if(result>CS_PS2_READY) return fail(b,result);
        if(result==CS_PS2_READY) enter(b,1,now);
        return CS_PS2_ACTIVE;
    }
    if(phase<CS_PS2_BUS_READY) {
        limit=phase==13?CS_PS2_BAT_TICKS:CS_PS2_SHORT_TICKS;
        if(b->polls>=CS_PS2_PHASE_POLLS || now-b->since>=limit) return fail(b,CS_PS2_TIMEOUT);
        ++b->polls;
    }
    status=io->read(io->context,0x64);
    if(phase==CS_PS2_BUS_READY) return ready(b,io,status,q);
    if((status&1u)!=0) {
        if(phase==3) {
            if(b->retries>=64u) return fail(b,CS_PS2_CONTROLLER);
            (void)io->read(io->context,0x60);
            ++b->retries; increment(&b->drained); return b->result;
        }
        byte=io->read(io->context,0x60); b->last_byte=byte;
        if((status&0x20u)!=0) increment(&b->auxiliary);
        /* Untrusted controller: no cleanup writes. */
        if((status&0xC0u)!=0) { increment(&b->errors); return fail(b,CS_PS2_CONTROLLER); }
        if((status&0x20u)!=0) {
            if(awaits_aux(phase)) return answer(b,phase,byte,now);
            return b->result;
        }
        if(phase==5) {
            if(byte!=0) return fail(b,CS_PS2_CONTROLLER);
            enter(b,6,now); return b->result;
        }
        if(phase==19) {
            if((byte&0x73u)!=0x10u) return fail(b,CS_PS2_CONTROLLER);
            b->configuration=byte; enter(b,20,now); return b->result;
        }
        /* Keyboard byte: prefix/FIFO preserved, AUX deadline untouched. */
        result=cs_ps2_receive(&b->keyboard,status,byte,q);
        return result>CS_PS2_READY?fail(b,result):b->result;
    }
    if(phase==3) { b->retries=0; enter(b,4,now); return b->result; }
    if(!command(phase,&port,&value)) return b->result;
    if((status&2u)!=0) return b->result;
    io->write(io->context,port,value);
    enter(b,phase+1u,now); return b->result;
}
