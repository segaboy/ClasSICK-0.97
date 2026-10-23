/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/window-session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"check %u line %d\n",checks,__LINE__); exit(1); } } while (0)
static cs_mac_window_session session;
static uint8_t canvas[CS_MAC_WINDOW_BYTES+32],old_canvas[sizeof canvas];
static uint8_t old_session[sizeof session];
typedef struct { unsigned enters,leaves,bad,masked; uint32_t image; } context;
static context ctx;
static uint32_t hash(void)
{
    uint32_t h=UINT32_C(2166136261);
    for(size_t i=0;i<(size_t)CS_MAC_WINDOW_BYTES;++i)h=(h^canvas[i])*UINT32_C(16777619);
    return h;
}
static uint32_t enter(void *cookie)
{
    context *c=cookie;if(c->masked)c->bad++;
    c->masked=1;c->enters++;c->image=hash();return UINT32_C(0xD00D0C0C);
}
static void leave(void *cookie,uint32_t token)
{
    context *c=cookie;
    if(c->masked!=1 || token!=UINT32_C(0xD00D0C0C) || c->image!=hash())c->bad++;
    c->masked=0;c->leaves++;
}
static void refused(cs_mac_window_session *s,uint8_t *p,size_t n,const cs_foreground_hooks *h)
{
    CHECK(cs_mac_window_begin(s,p,n,h)==CS_MAC_WINDOW_ARGUMENT);
    CHECK(memcmp(&session,old_session,sizeof session)==0);
    CHECK(memcmp(canvas,old_canvas,sizeof canvas)==0);
    CHECK(ctx.enters==0 && ctx.leaves==0);
}
static void sample(int32_t dx,int32_t dy,uint32_t buttons,uint32_t counter,int32_t x,int32_t y)
{
    CHECK(cs_inbox_push(&session.inbox,1,dx,dy,buttons)==CS_INBOX_OK);
    CHECK(cs_foreground_step(&session.foreground)==CS_FG_DELIVERED);
    CHECK(session.input.counter.presses==counter);
    CHECK(session.input.window.x==x && session.input.window.y==y);
    CHECK(ctx.masked==0 && ctx.bad==0 && ctx.enters==ctx.leaves);
}
int main(void)
{
    cs_foreground_hooks hooks={enter,leave,&ctx},bad;
    uint8_t saved_hooks[sizeof hooks];memcpy(saved_hooks,&hooks,sizeof hooks);
    memset(&session,0xA5,sizeof session);memset(canvas,0xCC,sizeof canvas);
    memcpy(old_session,&session,sizeof session);memcpy(old_canvas,canvas,sizeof canvas);
    refused(NULL,canvas,sizeof canvas,&hooks);refused(&session,NULL,sizeof canvas,&hooks);
    refused(&session,canvas,sizeof canvas,NULL);refused(&session,canvas,21887,&hooks);
    bad=hooks;bad.enter=NULL;refused(&session,canvas,sizeof canvas,&bad);
    bad=hooks;bad.leave=NULL;refused(&session,canvas,sizeof canvas,&bad);
    refused((cs_mac_window_session *)(void *)((uint8_t *)&session+1),canvas,sizeof canvas,&hooks);
    _Alignas(cs_foreground_hooks) uint8_t raw_hooks[sizeof hooks+1];
    memcpy(raw_hooks+1,&hooks,sizeof hooks);
    refused(&session,canvas,sizeof canvas,(const cs_foreground_hooks *)(const void *)(raw_hooks+1));
    refused(&session,(uint8_t *)(uintptr_t)(UINTPTR_MAX-100u),sizeof canvas,&hooks);
    refused(&session,(uint8_t *)(void *)&session,sizeof canvas,&hooks);
    refused(&session,canvas,sizeof canvas,(const cs_foreground_hooks *)(const void *)&session);
    refused(&session,canvas,sizeof canvas,(const cs_foreground_hooks *)(const void *)canvas);
    CHECK(memcmp(&hooks,saved_hooks,sizeof hooks)==0);
    CHECK(cs_mac_window_begin(&session,canvas,sizeof canvas,&hooks)==CS_MAC_WINDOW_OK);
    CHECK(session.magic==CS_MAC_WINDOW_MAGIC && session.phase==CS_MAC_WINDOW_READY);
    CHECK(session.stage==CS_MAC_WINDOW_STAGE_READY && session.cause==0);
    CHECK(session.surface.storage==canvas && session.surface.storage_size==21888);
    CHECK(session.cursor.surface==&session.surface && session.cursor.payload==session.payload);
    CHECK(session.input.cursor==&session.cursor && session.input.art==&session.art);
    CHECK(session.capture.owner==&session.input && session.foreground.inbox==&session.inbox);
    CHECK(session.foreground.capture==&session.capture && session.foreground.hooks.cookie==&ctx);
    CHECK(session.foreground.phase==CS_FG_ACTIVE && session.foreground.epoch==1);
    CHECK(session.inbox.phase==CS_INBOX_WAIT && session.inbox.epoch==1);
    CHECK(session.capture.phase==CS_CAPTURE_WAIT && session.capture.epoch==1);
    CHECK(ctx.enters==1 && ctx.leaves==1 && !ctx.bad && !ctx.masked);
    CHECK(hash()==UINT32_C(2123700856)); /* Pre-worker TEST-0074 PC framebuffer oracle. */
    CHECK(session.input.pointer.x==256 && session.input.pointer.y==171);
    CHECK(cs_foreground_step(&session.foreground)==CS_FG_EMPTY); /* No fake baseline. */
    CHECK(cs_inbox_baseline(&session.inbox,1,0)==CS_INBOX_OK);
    CHECK(cs_foreground_step(&session.foreground)==CS_FG_BASELINED);
    sample(-156,-91,0,0,80,70);sample(0,0,1,1,80,70);
    sample(20,10,1,1,100,80);sample(20,10,1,1,120,90);
    sample(0,0,0,1,120,90);sample(0,30,1,2,120,90);
    CHECK(session.input.window.pattern==1);
    for(unsigned i=0;i<16u;++i)CHECK(cs_inbox_push(&session.inbox,1,0,0,0)==CS_INBOX_OK);
    CHECK(cs_inbox_push(&session.inbox,1,0,0,0)==CS_INBOX_LOST);
    CHECK(cs_foreground_step(&session.foreground)==CS_FG_RESET);
    CHECK(session.inbox.phase==CS_INBOX_WAIT && session.capture.phase==CS_CAPTURE_WAIT);
    CHECK(session.input.counter.presses==2);
    CHECK(cs_inbox_baseline(&session.inbox,1,1)==CS_INBOX_OK);
    CHECK(cs_foreground_step(&session.foreground)==CS_FG_BASELINED);
    sample(0,0,1,2,120,90);sample(0,0,0,2,120,90);sample(0,0,1,3,120,90);
    CHECK(cs_foreground_stop(&session.foreground)==CS_FG_STOPPED);
    CHECK(cs_foreground_restart(&session.foreground,&session.input)==CS_FG_OK);
    CHECK(session.inbox.epoch==2 && session.capture.epoch==2 && session.foreground.epoch==2);
    CHECK(session.inbox.phase==CS_INBOX_WAIT && session.capture.phase==CS_CAPTURE_WAIT);
    CHECK(ctx.bad==0 && ctx.masked==0 && ctx.enters==ctx.leaves);
    for(size_t i=21888;i<sizeof canvas;++i)CHECK(canvas[i]==0xCC);
    CHECK(memcmp(&hooks,saved_hooks,sizeof hooks)==0);
    printf("Mac RAM bootstrap: %u checks; initial PC pixels match; no hardware/boot.\n",checks);
    return 0;
}
