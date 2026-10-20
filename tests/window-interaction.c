/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original directed contract tests. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/ui/window.h"
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
static cs_window fresh(void)
{
    cs_window w;
    const cs_window_config c={80,70,224,144,18};
    CHECK(cs_window_begin(&w,&c)==CS_WINDOW_OK);
    return w;
}
static cs_window_effect apply(cs_window *w,int32_t x,int32_t y,uint32_t flags,uint32_t buttons,int result)
{
    const cs_pointer_event e={x,y,flags,buttons};
    cs_window_effect out;
    memset(&out,0xA5,sizeof(out));
    CHECK(cs_window_apply(w,&e,&out)==result);
    CHECK(out.x==w->x && out.y==w->y);
    return out;
}
static void refused(cs_window *w,cs_pointer_event e,int code)
{
    cs_window before=*w;
    cs_window_effect out,copy;
    cs_pointer_event event_copy=e;
    memset(&out,0xA5,sizeof(out));copy=out;
    CHECK(cs_window_apply(w,&e,&out)==code);
    CHECK(memcmp(w,&before,sizeof(before))==0);
    CHECK(memcmp(&out,&copy,sizeof(out))==0 && memcmp(&e,&event_copy,sizeof(e))==0);
}
int main(void)
{
    const uint32_t press=CS_POINTER_PRESS|CS_POINTER_DOWN;
    const uint32_t held=CS_POINTER_DOWN|CS_POINTER_MOVED;
    const uint32_t cancel=CS_POINTER_CANCELLED|CS_POINTER_SUPPRESSED;
    cs_window w=fresh();
    cs_window_effect o;
    cs_pointer_event e={100,75,press,1u};
    cs_window_config c={0,0,2,2,1},before_c;
    CHECK(sizeof(w)==44u && sizeof(c)==20u && sizeof(o)==20u);
    o=apply(&w,100,75,press|CS_POINTER_MOVED,1u,CS_WINDOW_CHANGED);
    CHECK(w.x==80 && w.y==70 && w.grab_x==20 && w.grab_y==5 && o.flags==CS_WINDOW_DRAG_BEGAN);
    refused(&w,e,CS_WINDOW_STATE); /* replayed press cannot toggle or reanchor */
    o=apply(&w,511,341,held,1u,CS_WINDOW_CHANGED);
    CHECK(w.x==288 && w.y==198 && w.grab_x==20 && w.grab_y==5 && o.flags==CS_WINDOW_MOVED);
    o=apply(&w,300,200,held,1u,CS_WINDOW_CHANGED);
    CHECK(w.x==280 && w.y==195); /* original grab survives clamp */
    o=apply(&w,100,120,CS_POINTER_MOVED|CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    CHECK(w.x==80 && w.y==115 && !w.dragging && !w.down && !w.grab_x && !w.grab_y);
    CHECK(o.old_x==280 && o.old_y==195 && o.flags==(CS_WINDOW_MOVED|CS_WINDOW_DRAG_ENDED));
    o=apply(&w,90,133,press,1u,CS_WINDOW_CHANGED); /* first content row */
    CHECK(w.pattern==1u && !w.dragging && o.flags==CS_WINDOW_TOGGLED);
    for (unsigned n=0u;n<20u;++n) {
        o=apply(&w,90,133,CS_POINTER_DOWN,1u,CS_WINDOW_OK);
        CHECK(w.pattern==1u && o.flags==0u);
    }
    o=apply(&w,90,133,CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    CHECK(o.flags==0u && w.pattern==1u);
    o=apply(&w,100,120,press,1u,CS_WINDOW_CHANGED);
    CHECK(w.dragging==1u && w.grab_y==5);
    o=apply(&w,511,341,cancel|CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    CHECK(w.x==80 && w.y==115 && w.pattern==1u && o.flags==CS_WINDOW_DRAG_ENDED);
    o=apply(&w,0,0,cancel,0u,CS_WINDOW_OK);
    CHECK(o.flags==0u);
    o=apply(&w,0,0,CS_POINTER_SUPPRESSED,1u,CS_WINDOW_OK);
    CHECK(o.flags==0u && !w.down && !w.dragging);
    o=apply(&w,0,0,0u,6u,CS_WINDOW_OK); CHECK(o.flags==0u);
    o=apply(&w,304,150,press,1u,CS_WINDOW_CHANGED); /* right edge excluded */
    CHECK(o.flags==0u && w.down && w.pattern==1u);
    (void)apply(&w,304,150,CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    o=apply(&w,100,259,press,1u,CS_WINDOW_CHANGED); /* bottom edge excluded */
    CHECK(o.flags==0u);
    (void)apply(&w,100,259,CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    o=apply(&w,80,115,press,1u,CS_WINDOW_CHANGED);
    CHECK(w.dragging && w.grab_x==0 && w.grab_y==0); /* top/left included */
    (void)apply(&w,0,0,held,1u,CS_WINDOW_CHANGED);
    CHECK(w.x==0 && w.y==0);
    (void)apply(&w,0,0,CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    o=apply(&w,223,17,press,1u,CS_WINDOW_CHANGED);
    CHECK(w.dragging && w.grab_x==223 && w.grab_y==17); /* last title pixel */
    (void)apply(&w,0,0,held,1u,CS_WINDOW_OK); CHECK(w.x==0 && w.y==0);
    (void)apply(&w,0,0,cancel|CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    (void)apply(&w,0,18,press,1u,CS_WINDOW_CHANGED); CHECK(w.pattern==0u);
    (void)apply(&w,0,18,CS_POINTER_RELEASE,0u,CS_WINDOW_CHANGED);
    (void)apply(&w,0,0,CS_POINTER_CLAMP_X|CS_POINTER_CLAMP_Y,0u,CS_WINDOW_OK);
    w=fresh();
    refused(&w,(cs_pointer_event){0,0,CS_POINTER_DOWN,1u},CS_WINDOW_STATE);
    refused(&w,(cs_pointer_event){0,0,CS_POINTER_RELEASE,0u},CS_WINDOW_STATE);
    refused(&w,(cs_pointer_event){0,0,cancel|CS_POINTER_RELEASE,0u},CS_WINDOW_STATE);
    refused(&w,(cs_pointer_event){INT32_MIN,0,0u,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,INT32_MAX,0u,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){512,0,0u,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,342,0u,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,256u,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,0u,8u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,CS_POINTER_PRESS,1u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,press|CS_POINTER_RELEASE,1u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,press|CS_POINTER_SUPPRESSED,1u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,CS_POINTER_SUPPRESSED,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,cancel|CS_POINTER_MOVED,0u},CS_WINDOW_ARGUMENT);
    refused(&w,(cs_pointer_event){0,0,cancel,1u},CS_WINDOW_ARGUMENT);
    w.valid=0u; refused(&w,(cs_pointer_event){-1,0,0u,0u},CS_WINDOW_STATE);
    w=fresh();w.grab_x=1;refused(&w,e,CS_WINDOW_STATE);
    w=fresh();w.dragging=1u;refused(&w,e,CS_WINDOW_STATE);
    w=fresh();w.pattern=2u;refused(&w,e,CS_WINDOW_STATE);
    w=fresh();w.down=2u;refused(&w,e,CS_WINDOW_STATE);
    w=fresh();w.width=INT32_MAX;refused(&w,e,CS_WINDOW_STATE);
    w=fresh();
    CHECK(cs_window_apply(NULL,&e,&o)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_apply(&w,NULL,&o)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_apply(&w,&e,NULL)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_apply(&w,(const cs_pointer_event *)(const void *)&w,&o)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_apply(&w,&e,(cs_window_effect *)(void *)&w)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_apply(&w,&e,(cs_window_effect *)(void *)&e)==CS_WINDOW_ARGUMENT);
    CHECK(w.x==80 && w.down==0u && e.flags==press);
    CHECK(cs_window_begin(NULL,&c)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_begin(&w,NULL)==CS_WINDOW_ARGUMENT);
    CHECK(cs_window_begin(&w,(const cs_window_config *)(const void *)&w)==CS_WINDOW_ARGUMENT);
    before_c=c;
    CHECK(cs_window_begin(&w,&c)==CS_WINDOW_OK);
    CHECK(w.x==0 && w.y==0 && !w.pattern && !w.down && memcmp(&c,&before_c,sizeof(c))==0);
    c=(cs_window_config){0,0,512,342,341};CHECK(cs_window_begin(&w,&c)==CS_WINDOW_OK);
    for (unsigned n=0u;n<6u;++n) {
        cs_window before=w;
        cs_window_config bad=c;
        if(n==0u) bad.width=INT32_MIN;
        if(n==1u) bad.height=INT32_MAX;
        if(n==2u) bad.title=342;
        if(n==3u) bad.title=0;
        if(n==4u) bad.x=1;
        if(n==5u) bad.y=-1;
        CHECK(cs_window_begin(&w,&bad)==CS_WINDOW_ARGUMENT);
        CHECK(memcmp(&w,&before,sizeof(w))==0);
    }
    printf("Window directed PASS:%u checks\n",checks);
    return 0;
}
