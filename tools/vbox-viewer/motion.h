/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original diagnostic, SPEC-0098. */
#ifndef CS_VIEWER_MOTION_H
#define CS_VIEWER_MOTION_H
#include <stdint.h>
#include <stddef.h>
typedef struct { int x,y,anchored,held,suppressed; } cs_view_motion;
typedef struct { int dx,dy,buttons; } cs_view_event;
static inline int cs_view_cancel(cs_view_motion *s,cs_view_event *e)
{
    int result=s->held?2:0;
    e->dx=0;e->dy=0;e->buttons=0;
    s->anchored=0;s->held=0;s->suppressed=1;
    return result;
}
/* press distinguishes an actual down message from entering already held. */
static inline int cs_view_step(cs_view_motion *s,int x,int y,int inside,
    int held,int enabled,int press,cs_view_event *e)
{
    int dx,dy;
    if(x < -32768 || x > 32767 || y < -32768 || y > 32767 ||
       (inside!=0 && inside!=1) || (held!=0 && held!=1) ||
       (enabled!=0 && enabled!=1) || (press!=0 && press!=1) || (press&&!held)) return -1;
    if(!enabled || !inside) {
        int r=cs_view_cancel(s,e);
        if(!held) s->suppressed=0;
        return r;
    }
    if(s->suppressed) {
        if(held) return 0;
        s->suppressed=0;s->anchored=0;
    }
    if(!s->anchored) {
        s->x=x;s->y=y;s->anchored=1;
        if(held&&!press) {s->suppressed=1;s->anchored=0;return 0;}
        if(!held) return 0;
        s->held=1;e->dx=0;e->dy=0;e->buttons=1;return 1;
    }
    dx=x-s->x;dy=y-s->y;
    if(dx < -64 || dx > 64 || dy < -64 || dy > 64) {
        int r=cs_view_cancel(s,e);
        if(!held) {s->suppressed=0;s->anchored=1;s->x=x;s->y=y;}
        return r;
    }
    s->x=x;s->y=y;
    if(!dx&&!dy&&held==s->held) return 0;
    s->held=held;e->dx=dx;e->dy=dy;e->buttons=held;return 1;
}
static inline uint32_t cs_view_le32(const unsigned char *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
#define CS_VIEW_FRAME_SIZE (16u+1024u*768u*4u)
static inline uint32_t cs_view_frame(const unsigned char *p,size_t n,uint32_t last)
{
    uint32_t seq;
    if(!p || n!=CS_VIEW_FRAME_SIZE || cs_view_le32(p)!=UINT32_C(0x43534246) ||
       cs_view_le32(p+8)!=1024 || cs_view_le32(p+12)!=768) return 0;
    seq=cs_view_le32(p+4);
    return seq>last?seq:0;
}
#endif
