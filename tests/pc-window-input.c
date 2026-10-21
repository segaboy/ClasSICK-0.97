/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original report/owner handoff oracle. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pc/window-input.h"
#include "../platform/pc/ps2-bus.h"
#include "ps2-bus-fixture.h"
static unsigned checks,frames,packets;
#define CHECK(x) do { ++checks; if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
typedef struct {
    cs_window_input owner; cs_cursor_state cursor; cs_cursor_bits bits;
    cs_cursor_art art; cs_surface surface; unsigned char payload[1024];
    unsigned char *frame; size_t bytes;
} fixture;
static void begin(fixture *f,cs_surface_format format,uint32_t held)
{
    const cs_window_config window={80,70,224,144,18};
    const cs_product_state product={97u,0u,16u,6u,1u,NULL};
    cs_cursor_token token;
    size_t stride=format==CS_SURFACE_MONO1_MSB?71u:2567u;
    int32_t width=format==CS_SURFACE_MONO1_MSB?512:640;
    int32_t height=format==CS_SURFACE_MONO1_MSB?342:480;
    memset(f,0,sizeof(*f)); f->bytes=stride*(size_t)height+13u;
    f->frame=malloc(f->bytes+32u); CHECK(f->frame!=NULL);
    memset(f->frame,0xA5,f->bytes+32u);
    f->bits.mask[0]=f->bits.image[0]=0x80u;
    f->art.bits=&f->bits; f->art.width=f->art.height=1u;
    CHECK(cs_surface_init(&f->surface,f->frame+16u,f->bytes,width,height,stride,format)==CS_SURFACE_OK);
    CHECK(cs_cursor_init(&f->cursor)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&f->cursor,&f->surface,f->payload,sizeof(f->payload),&token)==CS_CURSOR_OK);
    CHECK(cs_window_input_begin(&f->owner,&f->cursor,token,&f->art,&window,&product,held)==CS_WINDOW_INPUT_OK);
}
static void compare(const fixture *a,const fixture *b)
{
    const cs_window_input *x=&a->owner,*y=&b->owner;
    CHECK(x->pointer.x==y->pointer.x && x->pointer.y==y->pointer.y);
    CHECK(x->pointer.down==y->pointer.down && x->pointer.suppress==y->pointer.suppress);
    CHECK(x->counter.presses==y->counter.presses && x->counter.down==y->counter.down);
    CHECK(x->window.x==y->window.x && x->window.y==y->window.y);
    CHECK(x->window.dragging==y->window.dragging && x->window.pattern==y->window.pattern);
    CHECK(x->last.flags==y->last.flags && x->last.buttons==y->last.buttons);
    CHECK(x->token.binding==y->token.binding && x->token.revision==y->token.revision);
    CHECK(a->bytes==b->bytes && memcmp(a->frame,b->frame,a->bytes+32u)==0); ++frames;
    for(unsigned i=0;i<16u;++i) CHECK(a->frame[i]==0xA5u && a->frame[a->bytes+16u+i]==0xA5u);
}
static void end(fixture *f)
{
    CHECK(cs_cursor_hide(&f->cursor,f->owner.token)==CS_CURSOR_OK);
    CHECK(cs_cursor_unbind(&f->cursor,f->owner.token)==CS_CURSOR_OK); free(f->frame);
}
/* Author a complete saturated test fixture, including its rendered receipt.
   Changing the counter alone would create invalid owner state. */
static void saturate(fixture *f)
{
    cs_window_input *o=&f->owner; cs_window_scene_effect effect;
    o->counter.presses=UINT32_MAX; o->product.inputs=UINT32_MAX;
    CHECK(cs_window_scene_compose(o->cursor,o->token,o->art,&o->product,
        &o->rendered_window,&o->window,o->rendered_inputs,o->pointer.x,o->pointer.y,&effect)==0);
    o->token=effect.token; o->render=effect; o->rendered_inputs=UINT32_MAX;
}
/* Authored raw bytes and expected logical samples; no adapter arithmetic in
   this oracle. Every partial byte must leave owner and frame untouched. */
typedef struct { unsigned char raw[3]; int32_t x,y; uint32_t buttons; } row;
static void report(fixture *a,fixture *b,cs_ps2_mouse *decoder,const row *r)
{
    cs_ps2_mouse_packet p={INT32_MIN,INT32_MAX,99u},sentinel=p;
    cs_pointer_sample expected={r->x,r->y,r->buttons};
    cs_window_input saved=a->owner;
    for(unsigned i=0;i<3u;++i) {
        int rc=cs_ps2_mouse_feed(decoder,r->raw[i],0u,&p);
        if(i<2u) {
            CHECK(rc==CS_PS2_MOUSE_MORE && memcmp(&p,&sentinel,sizeof(p))==0);
            CHECK(memcmp(&saved,&a->owner,sizeof(saved))==0); compare(a,b);
        } else {
            CHECK(rc==CS_PS2_MOUSE_PACKET);
            CHECK(cs_pc_window_packet(&a->owner,&p)==cs_window_input_sample(&b->owner,&expected));
            ++packets; compare(a,b);
        }
    }
}
static void trace(cs_surface_format format)
{
    static const row rows[]={
        {{0x09,0,0},0,0,1}, /* held at begin: no press */
        {{0x08,0,0},0,0,0}, {{0x09,0,0},0,0,1}, /* content */
        {{0x09,0,0},0,0,1}, {{0x18,100,96},-156,-96,0},
        {{0x09,0,0},0,0,1}, /* title at100,75 */
        {{0x29,200,136},200,120,1},
        {{0x18,246,30},-10,-30,0}, /* final release motion */
        {{0x0A,0,0},0,0,2}, {{0x0C,0,0},0,0,4}, /* not LEFT */
        {{0x38,0,0},-256,256,0}, {{0x38,0,0},-256,256,0},
        {{0x08,255,255},255,-255,0}, {{0x08,255,255},255,-255,0},
        {{0x38,128,128},-128,128,0}, {{0x09,0,0},0,0,1}
    };
    fixture a,b; cs_ps2_mouse decoder; cs_ps2_mouse_packet p={0,0,0};
    begin(&a,format,1u); begin(&b,format,1u);
    CHECK(cs_ps2_mouse_begin(&decoder)==CS_PS2_MOUSE_MORE); compare(&a,&b);
    for(unsigned i=0;i<sizeof(rows)/sizeof(rows[0]);++i) {
        report(&a,&b,&decoder,&rows[i]);
        if(i==0u) CHECK(a.owner.counter.presses==0u);
        if(i==2u || i==3u) CHECK(a.owner.counter.presses==1u && a.owner.window.pattern==1u);
        if(i==5u) CHECK(a.owner.window.dragging && a.owner.counter.presses==2u);
        if(i==6u) CHECK(a.owner.window.x==280 && a.owner.window.y==190);
        if(i==7u) CHECK(a.owner.window.x==270 && a.owner.window.y==160 && !a.owner.window.dragging);
        if(i==9u) CHECK(a.owner.counter.presses==2u);
        if(i==11u) CHECK(a.owner.pointer.x==0 && a.owner.pointer.y==341);
        if(i==13u) CHECK(a.owner.pointer.x==510 && a.owner.pointer.y==0);
    }
    /* Enter the moved title, then lose a report during an active drag. */
    const row title={{0x38,164,219},-92,37,0};
    const row drag={{0x29,20,246},20,10,1};
    report(&a,&b,&decoder,&title);
    CHECK(a.owner.pointer.x==290 && a.owner.pointer.y==165);
    CHECK(a.owner.window.x==270 && a.owner.window.y==160);
    report(&a,&b,&decoder,&rows[0]);
    CHECK(a.owner.window.dragging);
    report(&a,&b,&decoder,&drag);
    CHECK(a.owner.window.x==288 && a.owner.window.y==170); /* right edge clamps */
    uint32_t count=a.owner.counter.presses,pattern=a.owner.window.pattern;
    /* Known corrupt byte cancels exactly at the owner boundary. */
    CHECK(cs_ps2_mouse_feed(&decoder,0u,1u,&p)==CS_PS2_MOUSE_DROPPED);
    CHECK(cs_window_input_cancel(&a.owner)==cs_window_input_cancel(&b.owner)); compare(&a,&b);
    CHECK(a.owner.pointer.suppress==1u && !a.owner.window.dragging);
    CHECK(a.owner.counter.presses==count && a.owner.window.pattern==pattern);
    report(&a,&b,&decoder,&rows[0]); CHECK(a.owner.pointer.down==0u);
    report(&a,&b,&decoder,&rows[1]);
    saturate(&a); saturate(&b); compare(&a,&b);
    report(&a,&b,&decoder,&rows[2]);
    CHECK(a.owner.counter.presses==UINT32_MAX && a.owner.pointer.down==1u && a.owner.window.dragging);
    report(&a,&b,&decoder,&rows[4]);
    /* Overflow and untagged input are loss, not a stale packet replay. */
    CHECK(cs_ps2_mouse_feed(&decoder,0x48u,0u,&p)==CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&decoder,0u,0u,&p)==CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&decoder,0u,0u,&p)==CS_PS2_MOUSE_DROPPED);
    CHECK(cs_window_input_cancel(&a.owner)==cs_window_input_cancel(&b.owner)); compare(&a,&b);
    CHECK(cs_ps2_mouse_feed(&decoder,0u,0u,&p)==CS_PS2_MOUSE_DROPPED);
    CHECK(cs_window_input_cancel(&a.owner)==cs_window_input_cancel(&b.owner)); compare(&a,&b);
    end(&a); end(&b);
}
static void refusals(void)
{
    static const cs_ps2_mouse_packet invalid[]={
        {-257,0,0},{256,0,0},{0,-257,0},{0,256,0},{INT32_MIN,INT32_MIN,0},{0,0,8},{0,0,UINT32_MAX}
    };
    fixture f; begin(&f,CS_SURFACE_MONO1_MSB,0u);
    cs_window_input saved=f.owner; unsigned char *copy=malloc(f.bytes+32u); CHECK(copy!=NULL);
    memcpy(copy,f.frame,f.bytes+32u);
    for(unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) {
        CHECK(cs_pc_window_packet(&f.owner,&invalid[i])==CS_WINDOW_INPUT_ARGUMENT);
        CHECK(memcmp(&f.owner,&saved,sizeof(saved))==0 && memcmp(copy,f.frame,f.bytes+32u)==0);
    }
    CHECK(cs_pc_window_packet(NULL,&invalid[0])==CS_WINDOW_INPUT_ARGUMENT);
    CHECK(cs_pc_window_packet(&f.owner,NULL)==CS_WINDOW_INPUT_ARGUMENT);
    cs_ps2_mouse_packet valid={0,0,0};
    f.owner.valid=0; saved=f.owner;
    CHECK(cs_pc_window_packet(&f.owner,&valid)==CS_WINDOW_INPUT_STATE);
    CHECK(memcmp(&f.owner,&saved,sizeof(saved))==0 && memcmp(copy,f.frame,f.bytes+32u)==0);
    f.owner.valid=CS_WINDOW_INPUT_VALID; free(copy); end(&f);
}
static void bus_trace(cs_surface_format format)
{
    static const row rows[]={
        {{0x09,0,0},0,0,1},{{0x09,0,0},0,0,1},{{0x08,0,0},0,0,0},
        {{0x18,100,96},-156,-96,0},{{0x09,0,0},0,0,1},
        {{0x29,120,216},120,40,1},{{0x28,20,246},20,10,0},
        {{0x09,0,0},0,0,1},{{0x09,10,0},10,0,1},
        {{0x08,0,0},0,0,0},{{0x09,0,0},0,0,1},{{0x08,0,0},0,0,0}
    };
    bus_fixture hw; cs_ps2_bus bus; cs_ps2_bus_delivery event;
    cs_input_queue queue; cs_input_record storage[8]; uint64_t now=0;
    bf_init(&hw); cs_ps2_io io=bf_io(&hw);
    CHECK(cs_input_init(&queue,storage,8)==CS_INPUT_OK);
    CHECK(cs_ps2_bus_begin(&bus)==CS_PS2_ACTIVE);
    for(unsigned i=0;i<200u && bus.result==CS_PS2_ACTIVE;++i) {
        now+=1000u;
        CHECK(cs_ps2_bus_poll_delivery(&bus,&io,now,&queue,&event)<=CS_PS2_READY);
        CHECK(event.kind==CS_PS2_DELIVERY_NONE);
    }
    CHECK(bus.result==CS_PS2_READY && !hw.key.violations);
    bus.packets=bus.errors=bus.auxiliary=bus.left.presses=UINT32_MAX;
    fixture a,b; begin(&a,format,0); begin(&b,format,0);
    for(unsigned n=0;n<sizeof(rows)/sizeof(rows[0]);++n) {
        if(n==8u) { /* Known loss during the second title drag. */
            CHECK(a.owner.window.dragging && a.owner.counter.presses==3u);
            pf_byte(&hw.key,0,0x20); now+=1000u;
            CHECK(cs_ps2_bus_poll_delivery(&bus,&io,now,&queue,&event)==CS_PS2_READY);
            CHECK(event.kind==CS_PS2_DELIVERY_LOST);
            CHECK(cs_window_input_cancel(&a.owner)==cs_window_input_cancel(&b.owner)); compare(&a,&b);
            CHECK(!a.owner.window.dragging && a.owner.pointer.suppress==1u);
        }
        for(unsigned i=0;i<3u;++i) {
            pf_byte(&hw.key,rows[n].raw[i],0x20); now+=1000u;
            CHECK(cs_ps2_bus_poll_delivery(&bus,&io,now,&queue,&event)==CS_PS2_READY);
            CHECK(event.kind==(i==2u?CS_PS2_DELIVERY_PACKET:CS_PS2_DELIVERY_NONE));
            if(i==2u) {
                const cs_pointer_sample sample={rows[n].x,rows[n].y,rows[n].buttons};
                CHECK(cs_pc_window_packet(&a.owner,&event.packet)==cs_window_input_sample(&b.owner,&sample));
                ++packets;
            }
            compare(&a,&b);
        }
        if(n==1u) CHECK(a.owner.counter.presses==1u && a.owner.window.pattern==1u);
        if(n==6u) CHECK(a.owner.window.x==220 && a.owner.window.y==120 && !a.owner.window.dragging);
        if(n==8u) CHECK(a.owner.counter.presses==3u && !a.owner.window.dragging);
    }
    CHECK(a.owner.counter.presses==4u && !a.owner.window.dragging);
    CHECK(bus.packets==UINT32_MAX && bus.errors==UINT32_MAX && bus.auxiliary==UINT32_MAX);
    CHECK(!hw.key.violations); end(&a); end(&b);
}
int main(void)
{
    trace(CS_SURFACE_MONO1_MSB); trace(CS_SURFACE_RGBA8); refusals();
    bus_trace(CS_SURFACE_MONO1_MSB); bus_trace(CS_SURFACE_RGBA8);
    printf("%u checks, %u frame comparisons, %u packets PASS\n",checks,frames,packets); return 0;
}
