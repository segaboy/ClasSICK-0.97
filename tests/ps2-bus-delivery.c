/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original controller receipt oracle. */
#include <stdio.h>
#include <stdlib.h>
#include "../platform/pc/ps2-bus.h"
#include "ps2-bus-fixture.h"
#ifdef CS_BUS_BASELINE
uint32_t cs_baseline_bus_begin(cs_ps2_bus*);
uint32_t cs_baseline_bus_poll(cs_ps2_bus*,const cs_ps2_io*,uint64_t,cs_input_queue*);
#define OLD_BEGIN cs_baseline_bus_begin
#define OLD_POLL cs_baseline_bus_poll
#else
#define OLD_BEGIN cs_ps2_bus_begin
#define OLD_POLL cs_ps2_bus_poll
#endif
static unsigned checks,turns;
#define CHECK(x) do { ++checks; if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
typedef struct { bus_fixture hw; cs_ps2_io io; cs_ps2_bus bus;
    cs_input_queue queue; cs_input_record records[8]; } lane;
typedef struct { lane current,old; uint64_t tick; } pair;
static void init(pair *p)
{
    memset(p,0,sizeof(*p)); bf_init(&p->current.hw); bf_init(&p->old.hw);
    p->current.io=bf_io(&p->current.hw); p->old.io=bf_io(&p->old.hw);
    CHECK(cs_input_init(&p->current.queue,p->current.records,8)==CS_INPUT_OK);
    CHECK(cs_input_init(&p->old.queue,p->old.records,8)==CS_INPUT_OK);
    CHECK(cs_ps2_bus_begin(&p->current.bus)==OLD_BEGIN(&p->old.bus));
}
static void equal(const pair *p)
{
    const lane *a=&p->current,*b=&p->old;
    CHECK(memcmp(&a->bus,&b->bus,sizeof(a->bus))==0); /* zeroed, pointer-free */
    CHECK(memcmp(&a->hw.key,&b->hw.key,sizeof(a->hw.key))==0);
    CHECK(a->hw.mouse==b->hw.mouse && a->hw.prefix==b->hw.prefix && a->hw.ready==b->hw.ready);
    CHECK(a->hw.config==b->hw.config && a->hw.mouse_writes==b->hw.mouse_writes);
    CHECK(a->queue.capacity==b->queue.capacity && a->queue.head==b->queue.head
        && a->queue.count==b->queue.count && a->queue.next_sequence==b->queue.next_sequence);
    CHECK(memcmp(a->records,b->records,sizeof(a->records))==0);
    CHECK(!a->hw.key.violations && !b->hw.key.violations);
}
static uint32_t step(pair *p,uint32_t kind,int32_t x,int32_t y,uint32_t buttons)
{
    cs_ps2_bus_delivery event; lane *a=&p->current;
    unsigned reads=a->hw.key.status_reads,data=a->hw.key.data_reads;
    unsigned writes=a->hw.key.writes+a->hw.mouse_writes;
    uint32_t before=a->bus.result; memset(&event,0xA5,sizeof(event)); p->tick+=1000u;
    uint32_t r=cs_ps2_bus_poll_delivery(&a->bus,&a->io,p->tick,&a->queue,&event);
    CHECK(r==OLD_POLL(&p->old.bus,&p->old.io,p->tick,&p->old.queue)); equal(p);
    CHECK(event.kind==kind && event.packet.dx==x && event.packet.dy==y && event.packet.buttons==buttons);
    CHECK(a->hw.key.status_reads-reads==(before>CS_PS2_READY?0u:1u));
    CHECK(a->hw.key.data_reads-data<=1u && a->hw.key.writes+a->hw.mouse_writes-writes<=1u);
    CHECK(a->hw.key.data_reads==data || a->hw.key.writes+a->hw.mouse_writes==writes);
    ++turns; return r;
}
static void start(pair *p)
{
    init(p);
    for(unsigned n=0;n<200u && p->current.bus.result==CS_PS2_ACTIVE;++n)
        (void)step(p,CS_PS2_DELIVERY_NONE,0,0,0);
    CHECK(p->current.bus.result==CS_PS2_READY && p->current.hw.mouse_writes==14u);
    CHECK(p->current.bus.left.presses==0u);
}
static void byte(pair *p,uint8_t b,uint8_t flags,uint32_t kind,int32_t x,int32_t y,uint32_t buttons)
{
    pf_byte(&p->current.hw.key,b,flags); pf_byte(&p->old.hw.key,b,flags);
    CHECK(step(p,kind,x,y,buttons)==CS_PS2_READY);
}
static void packet(pair *p,uint8_t head,uint8_t x,uint8_t y,int32_t dx,int32_t dy,uint32_t buttons)
{
    byte(p,head,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(p,x,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(p,y,0x20,CS_PS2_DELIVERY_PACKET,dx,dy,buttons);
}
static void traffic(void)
{
    pair p; start(&p); CHECK(step(&p,CS_PS2_DELIVERY_NONE,0,0,0)==CS_PS2_READY);
    packet(&p,0x39,0,255,-256,-1,1); packet(&p,0x39,0,255,-256,-1,1);
    CHECK(step(&p,CS_PS2_DELIVERY_NONE,0,0,0)==CS_PS2_READY);
    p.current.bus.packets=p.old.bus.packets=UINT32_MAX;
    p.current.bus.errors=p.old.bus.errors=UINT32_MAX;
    p.current.bus.auxiliary=p.old.bus.auxiliary=UINT32_MAX;
    p.current.bus.left.presses=p.old.bus.left.presses=UINT32_MAX;
    packet(&p,0x08,255,255,255,255,0); packet(&p,0x09,0,0,0,0,1);
    byte(&p,0x29,0,CS_PS2_DELIVERY_NONE,0,0,0); /* existing Set2 keyboard profile */
    byte(&p,0xF0,0,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(&p,0x29,0,CS_PS2_DELIVERY_NONE,0,0,0);
    CHECK(p.current.queue.count==2u);
    byte(&p,0x29,0x40,CS_PS2_DELIVERY_NONE,0,0,0); /* keyboard error only */
    byte(&p,0,0x20,CS_PS2_DELIVERY_LOST,0,0,0); CHECK(p.current.bus.synced==0u);
    packet(&p,0x09,0,0,0,0,1); CHECK(p.current.bus.synced==1u);
    packet(&p,0x09,0,0,0,0,1); /* identical reports still fresh */
    byte(&p,0x48,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(&p,0,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(&p,0,0x20,CS_PS2_DELIVERY_LOST,0,0,0);
    byte(&p,0x08,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(&p,0,0xA0,CS_PS2_DELIVERY_LOST,0,0,0); /* corrupt partial report */
    CHECK(step(&p,CS_PS2_DELIVERY_NONE,0,0,0)==CS_PS2_READY);
    CHECK(p.current.bus.packets==UINT32_MAX && p.current.bus.errors==UINT32_MAX
        && p.current.bus.auxiliary==UINT32_MAX && p.current.bus.left.presses==UINT32_MAX);
    /* Internal left-state failure consumes a report but publishes no success. */
    p.current.bus.synced=p.old.bus.synced=1;
    p.current.bus.left.down=p.old.bus.left.down=2;
    byte(&p,0x09,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    byte(&p,0,0x20,CS_PS2_DELIVERY_NONE,0,0,0);
    pf_byte(&p.current.hw.key,0,0x20); pf_byte(&p.old.hw.key,0,0x20);
    CHECK(step(&p,CS_PS2_DELIVERY_NONE,0,0,0)==CS_PS2_STATE);
    CHECK(step(&p,CS_PS2_DELIVERY_NONE,0,0,0)==CS_PS2_STATE);
}
static void refusals(void)
{
    pair p; start(&p); lane *a=&p.current; cs_ps2_bus saved=a->bus;
    cs_ps2_bus_delivery event,seed; memset(&seed,0xA5,sizeof(seed)); event=seed;
    unsigned reads=a->hw.key.status_reads,data=a->hw.key.data_reads,writes=a->hw.mouse_writes;
    CHECK(cs_ps2_bus_poll_delivery(NULL,&a->io,p.tick,&a->queue,&event)==CS_PS2_ARGUMENT);
    CHECK(cs_ps2_bus_poll_delivery(&a->bus,NULL,p.tick,&a->queue,&event)==CS_PS2_ARGUMENT);
    CHECK(cs_ps2_bus_poll_delivery(&a->bus,&a->io,p.tick,NULL,&event)==CS_PS2_ARGUMENT);
    CHECK(cs_ps2_bus_poll_delivery(&a->bus,&a->io,p.tick,&a->queue,NULL)==CS_PS2_ARGUMENT);
    cs_ps2_io bad=a->io; bad.read=NULL;
    CHECK(cs_ps2_bus_poll_delivery(&a->bus,&bad,p.tick,&a->queue,&event)==CS_PS2_ARGUMENT);
    bad=a->io; bad.write=NULL;
    CHECK(cs_ps2_bus_poll_delivery(&a->bus,&bad,p.tick,&a->queue,&event)==CS_PS2_ARGUMENT);
    CHECK(memcmp(&seed,&event,sizeof(seed))==0 && memcmp(&saved,&a->bus,sizeof(saved))==0);
    CHECK(reads==a->hw.key.status_reads && data==a->hw.key.data_reads && writes==a->hw.mouse_writes);
    CHECK(cs_ps2_bus_poll_delivery(&a->bus,&a->io,p.tick-1u,&a->queue,&event)==CS_PS2_STATE);
    CHECK(OLD_POLL(&p.old.bus,&p.old.io,p.tick-1u,&p.old.queue)==CS_PS2_STATE); equal(&p);
    CHECK(event.kind==0u && event.packet.dx==0 && event.packet.dy==0 && event.packet.buttons==0u);
    CHECK(reads==a->hw.key.status_reads && data==a->hw.key.data_reads);
    CHECK(step(&p,CS_PS2_DELIVERY_NONE,0,0,0)==CS_PS2_STATE);
    init(&p); p.current.bus.phase=p.old.bus.phase=25;
    event=seed;
    CHECK(cs_ps2_bus_poll_delivery(&p.current.bus,&p.current.io,0,&p.current.queue,&event)==CS_PS2_STATE);
    CHECK(OLD_POLL(&p.old.bus,&p.old.io,0,&p.old.queue)==CS_PS2_STATE); equal(&p);
    CHECK(event.kind==0u && !event.packet.dx && !event.packet.dy && !event.packet.buttons);
    CHECK(!p.current.hw.key.status_reads);
}
int main(void)
{
    _Static_assert(sizeof(cs_ps2_bus_delivery)==16,"supported receipt layout");
    traffic(); refusals(); printf("%u delivery checks/%u paired polls PASS\n",checks,turns); return 0;
}
