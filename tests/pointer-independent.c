/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Contract-derived oracle, authored before implementation review. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../core/input/pointer.h"
#include "../core/input/mouse.h"

static unsigned checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"pointer:%d: %s\n",__LINE__,#c); return 1; } } while (0)
static void expected(cs_pointer *p, const cs_pointer_sample *s, cs_pointer_event *e, int *rc)
{
    int64_t x=(int64_t)p->x+s->dx, y=(int64_t)p->y+s->dy;
    uint32_t old=p->suppress, left=s->buttons&1u, f=0;
    if(x<0){x=0;f|=CS_POINTER_CLAMP_X;}
    if(x>511){x=511;f|=CS_POINTER_CLAMP_X;}
    if(y<0){y=0;f|=CS_POINTER_CLAMP_Y;}
    if(y>341){y=341;f|=CS_POINTER_CLAMP_Y;}
    if(x!=p->x || y!=p->y)f|=CS_POINTER_MOVED;
    p->x=(int32_t)x;p->y=(int32_t)y;
    if(p->suppress){if(!left)p->suppress=0;}
    else if(left!=p->down){p->down=left;f|=left?CS_POINTER_PRESS:CS_POINTER_RELEASE;}
    *rc=((f&(CS_POINTER_MOVED|CS_POINTER_PRESS|CS_POINTER_RELEASE)) || old!=p->suppress)?CS_POINTER_CHANGED:CS_POINTER_OK;
    e->x=p->x;e->y=p->y;e->buttons=s->buttons;
    e->flags=f|(p->down?CS_POINTER_DOWN:0u)|(p->suppress?CS_POINTER_SUPPRESSED:0u);
}
static int same(const cs_pointer *a,const cs_pointer *b)
{
    return a->x==b->x && a->y==b->y && a->down==b->down && a->suppress==b->suppress && a->valid==b->valid;
}
int main(void)
{
    cs_pointer p,r,before;cs_pointer_event out,want,saved;cs_pointer_sample sample;
    cs_mouse_button counter;uint32_t rng=0x93427u,count=UINT32_MAX-1u;
    int rc;unsigned i;
    memset(&p,0xA5,sizeof p);before=p;
    CHECK(cs_pointer_begin(NULL,0)==CS_POINTER_ARGUMENT);
    CHECK(cs_pointer_begin(&p,8)==CS_POINTER_ARGUMENT && memcmp(&p,&before,sizeof p)==0);
    CHECK(cs_pointer_begin(&p,1)==CS_POINTER_OK);
    r.x=256;r.y=171;r.down=0;r.suppress=1;r.valid=CS_POINTER_VALID;
    CHECK(same(&p,&r));CHECK(cs_mouse_button_begin(&counter,0)==CS_MOUSE_OK);
    counter.presses=count;
    for(i=0;i<3000u;++i){
        uint32_t f;
        rng=rng*1664525u+1013904223u;
        sample.dx=(int32_t)(rng&1023u)-512;sample.dy=(int32_t)((rng>>10)&1023u)-512;
        if(i%31u==0)sample.dx=INT32_MIN;
        if(i%37u==0)sample.dy=INT32_MAX;
        if(i%41u==0)sample.dx=INT32_MAX;
        if(i%43u==0)sample.dy=INT32_MIN;
        sample.buttons=(rng>>20)&7u;
        expected(&r,&sample,&want,&rc);memset(&out,0x5A,sizeof out);
        CHECK(cs_pointer_sample_apply(&p,&sample,&out)==rc);
        CHECK(same(&p,&r));CHECK(out.x==want.x && out.y==want.y && out.flags==want.flags && out.buttons==want.buttons);
        f=out.flags;
        if(f&CS_POINTER_PRESS){CHECK(cs_mouse_button_sample(&counter,1)==CS_MOUSE_CHANGED);if(count!=UINT32_MAX)++count;}
        if(f&CS_POINTER_RELEASE)CHECK(cs_mouse_button_sample(&counter,0)==CS_MOUSE_CHANGED);
        CHECK(counter.down==p.down && counter.presses==count);
        if(i%97u==0){
            f=CS_POINTER_CANCELLED|CS_POINTER_SUPPRESSED|(r.down?CS_POINTER_RELEASE:0u);
            CHECK(cs_pointer_cancel(&p,&out)==CS_POINTER_CHANGED);
            if(r.down)CHECK(cs_mouse_button_sample(&counter,0)==CS_MOUSE_CHANGED);
            r.down=0;r.suppress=1;
            CHECK(out.flags==f && out.buttons==0 && out.x==r.x && out.y==r.y && same(&p,&r));
            CHECK(cs_pointer_cancel(&p,&out)==CS_POINTER_CHANGED && out.flags==(CS_POINTER_CANCELLED|CS_POINTER_SUPPRESSED));
            CHECK(counter.down==0 && counter.presses==count);
        }
    }
    CHECK(counter.presses==UINT32_MAX);
    before=p;memset(&out,0xC7,sizeof out);saved=out;sample.dx=1;sample.dy=2;sample.buttons=8;
    CHECK(cs_pointer_sample_apply(&p,&sample,&out)==CS_POINTER_ARGUMENT);
    CHECK(memcmp(&p,&before,sizeof p)==0 && memcmp(&out,&saved,sizeof out)==0);
    CHECK(cs_pointer_sample_apply(&p,&sample,NULL)==CS_POINTER_ARGUMENT);
    sample.buttons=0;
    CHECK(cs_pointer_sample_apply(&p,(const cs_pointer_sample *)(const void *)&p,&out)==CS_POINTER_ARGUMENT);
    CHECK(cs_pointer_cancel(&p,(cs_pointer_event *)(void *)&p)==CS_POINTER_ARGUMENT);
    CHECK(memcmp(&p,&before,sizeof p)==0 && memcmp(&out,&saved,sizeof out)==0);
    for(i=0;i<6u;++i){
        p=before;
        if(i==0)p.valid=0;
        if(i==1)p.x=-1;
        if(i==2)p.y=342;
        if(i==3)p.down=2;
        if(i==4)p.suppress=2;
        if(i==5){p.down=1;p.suppress=1;}
        r=p;
        CHECK(cs_pointer_sample_apply(&p,&sample,&out)==CS_POINTER_STATE);
        CHECK(memcmp(&p,&r,sizeof p)==0 && memcmp(&out,&saved,sizeof out)==0);
        CHECK(cs_pointer_cancel(&p,&out)==CS_POINTER_STATE);
        sample.buttons=8;CHECK(cs_pointer_sample_apply(&p,&sample,&out)==CS_POINTER_ARGUMENT);sample.buttons=0;
    }
    CHECK(cs_pointer_begin(&p,6)==CS_POINTER_OK && p.down==0 && p.suppress==0);
    sample.dx=INT32_MAX;sample.dy=INT32_MAX;sample.buttons=0;
    CHECK(cs_pointer_sample_apply(&p,&sample,&out)==CS_POINTER_CHANGED && p.x==511 && p.y==341);
    sample.dx=1;sample.dy=1;
    CHECK(cs_pointer_sample_apply(&p,&sample,&out)==CS_POINTER_OK && out.flags==(CS_POINTER_CLAMP_X|CS_POINTER_CLAMP_Y));
    CHECK(cs_pointer_begin(&p,1)==CS_POINTER_OK);sample.dx=0;sample.dy=0;sample.buttons=2;
    CHECK(cs_pointer_sample_apply(&p,&sample,&out)==CS_POINTER_CHANGED && p.suppress==0 && out.buttons==2 && out.flags==0);
    printf("pointer independent PASS: %u assertions,3000 reference transitions;counter saturation/cancel/refusals\n",checks);
    return 0;
}
