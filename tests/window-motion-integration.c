/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Lead-authored SPEC-0097 check against the real pointer/window/render graph. */
#define main strict_foreground_test_main
#include "window-foreground-integration.c"
#undef main
#include <string.h>
static cs_window_input expected;
static uint8_t expected_pixels[sizeof pixels];
static unsigned merges;
static void init_graph(uint32_t held)
{
    static cs_surface s;cs_cursor_token token;
    cs_window_config cfg={80,70,224,144,18};
    cs_product_state product={97,0,16,6,1,NULL};
    static cs_cursor_art art={&bits,1,1,0,0};
    memset(&owner,0,sizeof owner);memset(&bits,0,sizeof bits);
    bits.mask[0]=bits.image[0]=0x80u;
    CHECK(cs_surface_init(&s,pixels,sizeof pixels,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CHECK(cs_cursor_init(&cursor)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&cursor,&s,payload,sizeof payload,&token)==CS_CURSOR_OK);
    CHECK(cs_window_input_begin(&owner,&cursor,token,&art,&cfg,&product,held)==CS_WINDOW_INPUT_OK);
    CHECK(cs_foreground_begin(&foreground,&box,&capture,&owner,&hooks)==CS_FG_OK);
    CHECK(cs_inbox_baseline(&box,1,held)==CS_INBOX_OK);pump();
}
static void compare(const cs_pointer_sample *raw,unsigned n,uint32_t held)
{
    unsigned i;
    init_graph(held);
    for(i=0;i<n;i++)event(raw[i].dx,raw[i].dy,raw[i].buttons);
    expected=owner;memcpy(expected_pixels,pixels,sizeof pixels);
    init_graph(held);
    for(i=0;i<n;i++){
        int r=cs_inbox_push_motion(&box,1,raw[i].dx,raw[i].dy,raw[i].buttons);
        CHECK(r==CS_INBOX_OK || r==CS_INBOX_MERGED);
        merges+=(unsigned)(r==CS_INBOX_MERGED);
    }
    while(box.count)pump();
    CHECK(owner.pointer.x==expected.pointer.x && owner.pointer.y==expected.pointer.y);
    CHECK(owner.pointer.down==expected.pointer.down && owner.pointer.suppress==expected.pointer.suppress);
    CHECK(memcmp(&owner.window,&expected.window,sizeof owner.window)==0);
    CHECK(owner.counter.presses==expected.counter.presses && owner.counter.down==expected.counter.down);
    CHECK(memcmp(pixels,expected_pixels,sizeof pixels)==0);
    CHECK(box.losses==0u && owner.phase==CS_WINDOW_INPUT_READY);
}
int main(void)
{
    cs_pointer_sample raw[15];uint32_t seed=0x97u;
    const cs_pointer_sample edge[]={
        {-176,-92,0},{0,0,1},{-80,0,1},{-1,0,1},{1,0,1},{0,0,0},
        {120,0,0},{0,0,1},{600,0,1},{600,0,1},{0,500,1},{0,500,1},
        {-600,0,1},{-600,0,1},{0,0,0}};
    /* Press just outside the title then enter while held: must NOT start drag. */
    const cs_pointer_sample hit[]={
        {-177,-92,0},{0,0,1},{2,0,1},{2,0,1},{0,0,0},
        {0,0,1},{2,0,1},{2,0,1},{0,0,0}};
    const cs_pointer_sample extremes[]={
        {-176,-92,0},{0,0,1},{INT32_MAX-1,0,1},{1,0,1},
        {0,INT32_MIN+1,1},{0,-1,1},{0,0,0}};
    CHECK(strict_foreground_test_main()==0);
    compare(edge,sizeof edge/sizeof edge[0],0);
    compare(hit,sizeof hit/sizeof hit[0],0);
    compare(extremes,sizeof extremes/sizeof extremes[0],0);
    for(unsigned t=0;t<300;t++){
        raw[0]=(cs_pointer_sample){-176,-92,0};
        raw[1]=(cs_pointer_sample){0,0,1};
        for(unsigned i=2;i<15;i++){
            seed=seed*1664525u+1013904223u;
            raw[i].dx=(int32_t)((seed>>8)%1201u)-600;
            raw[i].dy=(int32_t)((seed>>20)%801u)-400;
            if(t&1u){if(raw[i].dx<0)raw[i].dx=-raw[i].dx;if(raw[i].dy<0)raw[i].dy=-raw[i].dy;}
            raw[i].buttons=(seed&15u)==0u?0u:1u;
        }
        compare(raw,15,t%2u);
    }
    CHECK(merges>0);
    printf("monotone real graph:303 traces,%u merges,%u checks; full final frames equal\n",merges,checks);
    return 0;
}
