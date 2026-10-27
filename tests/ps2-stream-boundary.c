/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original F4 stream-boundary regression. */
#include <stdio.h>
#include <string.h>
#include "../platform/pc/ps2-bus.h"

typedef struct {
    uint8_t b[32],aux[32]; unsigned head,tail,prefix,nack,silent,status,data,writes,bad;
    uint8_t wv[16];
} ctl;
static void put(ctl *c,uint8_t v,uint8_t aux){ c->b[c->tail]=v; c->aux[c->tail++]=aux; }
static uint8_t rd(void *p,uint16_t port)
{
    ctl *c=p;
    if(port==0x64){ ++c->status; return (uint8_t)(c->head<c->tail?1u|(c->aux[c->head]?0x20u:0u):0u); }
    if(port!=0x60 || c->head>=c->tail){ ++c->bad; return 0; }
    ++c->data; return c->b[c->head++];
}
static void wr(void *p,uint16_t port,uint8_t v)
{
    ctl *c=p;
    if(c->writes>=16){ ++c->bad; return; }
    c->wv[c->writes++]=v;
    if(port==0x64){ if(v==0xD4)c->prefix=1; else if(v!=0xAE)++c->bad; return; }
    if(port!=0x60 || !c->prefix || v!=0xF4){ ++c->bad; return; }
    c->prefix=0;
    if(c->silent) return;
    if(c->nack){ --c->nack; put(c,0xFE,1); return; }
    /* ACK, then immediate asymmetric reports with one keyboard byte inside */
    put(c,0xFA,1); put(c,0x19,1); put(c,0x29,0); put(c,0xFE,1); put(c,0x03,1);
    put(c,0x28,1); put(c,0x05,1); put(c,0xFD,1);
}

static int fails;
#define CK(x) do{ if(!(x)){ fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); ++fails; } }while(0)

static cs_ps2_bus b;
static cs_input_queue q;
static cs_input_record rec[8];
static cs_ps2_mouse_packet got[4];
static unsigned ngot;

static uint32_t run(ctl *c,unsigned polls,uint64_t step)
{
    cs_ps2_io io={rd,wr,c}; uint32_t r=CS_PS2_ACTIVE; unsigned i;
    CK(cs_input_init(&q,rec,8)==CS_INPUT_OK);
    CK(cs_ps2_bus_begin(&b)==CS_PS2_ACTIVE);
    b.keyboard.phase=CS_PS2_PHASE_READY; b.keyboard.result=CS_PS2_READY;
    b.phase=20; b.started=1; ngot=0;
    for(i=0;i<polls;++i){
        cs_ps2_bus_delivery d; unsigned s=c->status,dr=c->data,w=c->writes;
        r=cs_ps2_bus_poll_delivery(&b,&io,(uint64_t)(i+1)*step,&q,&d);
        CK(c->status-s==1u); /* every live turn here samples status once */
        CK(c->status-s<=1u && c->data-dr<=1u && c->writes-w<=1u);
        CK(c->data==dr || c->writes==w);
        if(d.kind==CS_PS2_DELIVERY_PACKET && ngot<4) got[ngot++]=d.packet;
        if(r>CS_PS2_READY) break;
    }
    return r;
}

int main(void)
{
    ctl c;
    memset(&c,0,sizeof c);
    CK(run(&c,40,1)==CS_PS2_READY && b.phase==CS_PS2_BUS_READY);
    CK(c.writes==3 && c.wv[0]==0xAE && c.wv[1]==0xD4 && c.wv[2]==0xF4 && !c.bad);
    CK(ngot==2 && b.packets==2 && b.errors==0 && b.auxiliary==7);
    CK(got[0].dx==-2 && got[0].dy==3 && got[0].buttons==1);
    CK(got[1].dx==5 && got[1].dy==-3 && got[1].buttons==0);
    CK(c.data==8 && c.head==c.tail && q.count==1 && b.left.presses==1 && b.left.down==0);

    memset(&c,0,sizeof c); c.nack=1;
    CK(run(&c,60,1)==CS_PS2_READY);
    CK(c.writes==5 && c.wv[3]==0xD4 && c.wv[4]==0xF4 && b.resends==1 && !c.bad);
    CK(ngot==2 && b.packets==2 && q.count==1 && c.head==c.tail);

    memset(&c,0,sizeof c); c.nack=3;
    CK(run(&c,60,1)==CS_PS2_DEVICE && c.writes==7 && b.resends==2 && ngot==0);

    memset(&c,0,sizeof c); c.silent=1;
    CK(run(&c,3,1)==CS_PS2_ACTIVE && b.phase==23 && c.writes==3);
    {
        cs_ps2_io io={rd,wr,&c}; unsigned before_status=c.status,before_data=c.data;
        CK(cs_ps2_bus_poll(&b,&io,CS_PS2_SHORT_TICKS+10u,&q)==CS_PS2_TIMEOUT && c.writes==3);
        CK(c.status==before_status && c.data==before_data);
    }
    puts(fails?"FAIL":"PS2 stream boundary: PASS");
    return fails!=0;
}
