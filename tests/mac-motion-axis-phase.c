/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Source-informed latch fixture; no physical timing claim. */
#define main existing_latch_main
#include "mac-motion-latch.c"
#undef main
#if CLASSICK_MAC_FAST_COLLECT
static uint8_t scc_read(void *c,uint32_t ch,uint32_t reg){return ior(c,ch,reg);}
static void scc_write(void *c,uint32_t ch,uint32_t reg,uint8_t v){iow(c,ch,reg,v);}
static uint8_t motion_pb(void *c){return iopb(c);}
#include "../platform/mac128k/fast-collect.inc"
#endif
static int ready(T *t,unsigned raw,unsigned phase,unsigned invert)
{
    cs_mac_motion_io io={0};
    memset(t,0,sizeof *t);fx_init(&t->f,(uint8_t)(PBB|(phase<<4)));
    t->f.c[0].live=(int)(raw&1u);t->f.c[1].live=(int)((raw>>1)&1u);
    io.cookie=&t->f;io.read=ior;io.write=iow;io.pb=iopb;
#if CLASSICK_MAC_FAST_COLLECT
    io.collect=cs_mac_native_collect;
#endif
    CHECK(cs_inbox_begin(&t->in)==CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(&t->m,&io,&t->in,invert)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_arm(&t->m)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_service(&t->m)==CS_MAC_MOTION_BASELINED);
    CHECK(cs_inbox_claim(&t->in,t->m.epoch)==CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&t->in,t->m.epoch,0)==CS_INBOX_OK);
    return 0;
}
#if CLASSICK_MAC_SHORT_COLLECT
#include "../platform/mac128k/window-session.h"
static uint32_t serial_enter(void *c){(void)c;return 0;}
static void serial_leave(void *c,uint32_t t){(void)c;(void)t;}
/* Whole original software graph, serialized fixture IO; not ISR/hardware proof. */
static int held_drag_model(unsigned axis)
{
    static cs_mac_window_session s;
    static uint8_t canvas[CS_MAC_WINDOW_BYTES];
    struct cs_mac_motion m;
    fx_t f;fx_init(&f,PBB);
    cs_foreground_hooks hooks={serial_enter,serial_leave,0};
    cs_mac_motion_io io={0};
    io.cookie=&f;io.read=ior;io.write=iow;io.pb=iopb;io.collect=cs_mac_native_collect;
    CHECK(cs_mac_window_begin(&s,canvas,sizeof canvas,&hooks)==CS_MAC_WINDOW_OK);
    CHECK(cs_mac_motion_prepare(&m,&io,&s.inbox,3u)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_arm(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_BASELINED);
    CHECK(cs_inbox_push(&s.inbox,1u,-100,-92,0u)==CS_INBOX_OK);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_DELIVERED);
    f.pb&=(uint8_t)~PBB;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_DELIVERED);
    CHECK(s.input.window.dragging==1u && s.input.counter.presses==1u);
    set(&f,(int)axis,1);set(&f,(int)axis,0);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_EMPTY);
    CHECK(s.input.window.dragging==1u && s.input.window.x==80 && s.input.window.y==70);
    CHECK(m.motion_drops==2u && m.losses==0u);
    set(&f,(int)axis,1);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_DELIVERED);
    CHECK(s.input.window.x==(axis?80:81) && s.input.window.y==(axis?69:70));
    f.pb|=PBB;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_DELIVERED);
    CHECK(s.input.window.dragging==0u && s.input.counter.presses==1u);
    set(&f,(int)axis,0);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_DELIVERED);
    CHECK(s.input.window.x==(axis?80:81) && s.input.window.y==(axis?69:70));
    f.pb&=(uint8_t)~PBB;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_DELIVERED);
    CHECK(s.input.window.dragging==1u && s.input.counter.presses==2u);
    CHECK(cs_inbox_uncertain(&s.inbox,1u)==CS_INBOX_LOST);
    CHECK(cs_foreground_step(&s.foreground)==CS_FG_RESET);
    CHECK(s.input.window.dragging==0u && s.inbox.phase==CS_INBOX_WAIT);
    return 0;
}
/* As ready(), but the baseline stays unclaimed: inbox remains PRIMING. */
static int primed(T *t)
{
    cs_mac_motion_io io={0};
    memset(t,0,sizeof *t);fx_init(&t->f,PBB);
    io.cookie=&t->f;io.read=ior;io.write=iow;io.pb=iopb;
    io.collect=cs_mac_native_collect;
    CHECK(cs_inbox_begin(&t->in)==CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(&t->m,&io,&t->in,0u)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_arm(&t->m)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_service(&t->m)==CS_MAC_MOTION_BASELINED);
    CHECK(t->in.phase==CS_INBOX_PRIMING);
    return 0;
}
static uint8_t late_value;
static uint8_t altered_late(void *c,uint32_t ch,uint32_t reg)
{
    return ch==CS_MAC_SCC_A && reg==3u?late_value:ior(c,ch,reg);
}
#endif
int main(void)
{
    T t; unsigned cases=0;
#if CLASSICK_MAC_SHORT_COLLECT
    RUN(held_drag_model(0));RUN(held_drag_model(1));
#endif
    /* A clean edge after the entry RR3 read remains latched, not lost. */
    for(unsigned axis=0;axis<2;axis++)for(unsigned raw=0;raw<4;raw++)
    for(unsigned phase=0;phase<4;phase++)for(unsigned invert=0;invert<4;invert++){
        RUN(ready(&t,raw,phase,invert));
        t.f.hook_at=t.f.pbs+1u;t.f.hook_ch=(int)axis;
        t.f.hook_val=(int)(((raw>>axis)^1u)&1u);
#if CLASSICK_MAC_SHORT_COLLECT
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.m.losses==0u && t.in.count==0u && t.m.have_phase==1u);
        CHECK(t.m.last_raw_a==(raw&1u) && t.m.last_raw_b==((raw>>1)&1u));
        CHECK(rr3(&t.f)==(axis?1u:8u));
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK);
        CHECK(t.m.losses==0u && t.in.count==1u && rr3(&t.f)==0u);
        CHECK(axis?t.in.records[t.in.head].dx==0:t.in.records[t.in.head].dy==0);
#else
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_PENDING);
#endif
    }
#if CLASSICK_MAC_SHORT_COLLECT
    for(unsigned axis=0;axis<2;axis++){
        /* A missing/mismatching late latch cannot justify retry. */
        for(unsigned value=0;value<2;value++){
            RUN(ready(&t,0,0,0));t.m.io.read=altered_late;
            late_value=(uint8_t)(value?(axis?8u:1u):0u);
            t.f.hook_at=t.f.pbs+1u;t.f.hook_ch=(int)axis;t.f.hook_val=1;
            /* Stable held button: omit motion, keep drag, count a drop. */
            CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
            CHECK(t.m.losses==0u && t.m.motion_drops==1u && t.in.count==0u);
            CHECK(t.in.phase==CS_INBOX_READY && t.m.have_phase==1u && t.m.quiesced==0u);
            CHECK((axis?t.m.last_raw_b:t.m.last_raw_a)==1u && rr3(&t.f)==(axis?1u:8u));
            /* Still latched at the rebaselined value: one pending/no-change drop. */
            CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
            CHECK(t.m.motion_drops==2u && rr3(&t.f)==0u && t.m.losses==0u);
            CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
            CHECK(t.m.motion_drops==2u && t.m.losses==0u);
        }
        RUN(ready(&t,0,0,0));t.m.io.read=altered_late;late_value=0x80;
        t.f.hook_at=t.f.pbs+1u;t.f.hook_ch=(int)axis;t.f.hook_val=1;
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_FAULT);
        /* A coincident button change is not deferred or synthesized. */
        RUN(ready(&t,0,0,0));
        t.f.hook_at=t.f.pbs+1u;t.f.hook_ch=(int)axis;t.f.hook_val=1;t.f.hook_xor=PBB;
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.in.count==0u && t.m.first_loss.reason==CS_MAC_LOSS_PENDING);
        CHECK(t.m.motion_drops==0u);
        /* An acknowledged entry-pending axis prevents retry; the mixed pass
         * is dropped whole (no partial publication), drag retained. */
        RUN(ready(&t,0,0,0));set(&t.f,(int)(axis^1u),1);
        t.f.hook_at=t.f.pbs+1u;t.f.hook_ch=(int)axis;t.f.hook_val=1;
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.m.losses==0u && t.m.motion_drops==1u && t.in.count==0u);
        CHECK(t.m.last_raw_a==1u && t.m.last_raw_b==1u);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY && t.m.motion_drops==2u);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY && t.m.motion_drops==2u);
        CHECK(t.m.losses==0u && rr3(&t.f)==0u);
        /* Held drag: delivered press, omitted ambiguous motion, coherent
         * motion with the held button, release delivered once. Model only. */
        RUN(ready(&t,0,0,0));
        t.f.pb&=(uint8_t)~PBB;
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK && t.in.count==1u);
        CHECK(t.in.records[t.in.head].buttons==CS_POINTER_LEFT);
        set(&t.f,(int)axis,1);set(&t.f,(int)axis,0);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.in.count==1u && t.m.motion_drops==1u);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.in.count==1u && t.m.motion_drops==2u && t.m.losses==0u);
        CHECK(t.in.phase==CS_INBOX_READY && t.in.losses==0u);
        CHECK(t.m.last_buttons==CS_POINTER_LEFT && t.m.quiesced==0u);
        set(&t.f,(int)axis,1);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK && t.in.count==2u);
        {
            const cs_pointer_sample *p=&t.in.records[(t.in.head+1u)%CS_INBOX_CAPACITY];
            CHECK(p->buttons==CS_POINTER_LEFT);
            CHECK(axis?(p->dx==0 && p->dy==1):(p->dx==-1 && p->dy==0));
        }
        t.f.pb|=PBB;
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK && t.in.count==3u);
        CHECK(t.in.records[(t.in.head+2u)%CS_INBOX_CAPACITY].buttons==0u);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY && t.in.count==3u);
        CHECK(t.m.losses==0u && t.m.motion_drops==2u);
        /* Ineligible passes keep the original cancel. Quiesced: */
        RUN(ready(&t,0,0,0));t.m.quiesced=1u;
        set(&t.f,(int)axis,1);set(&t.f,(int)axis,0);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_UNSTABLE && t.m.motion_drops==0u);
        /* PRIMING: */
        RUN(primed(&t));
        set(&t.f,(int)axis,1);set(&t.f,(int)axis,0);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_UNSTABLE && t.m.motion_drops==0u);
        /* Invalid count: never kept as a drop. */
        RUN(ready(&t,0,0,0));t.in.count=CS_INBOX_CAPACITY+1u;
        set(&t.f,(int)axis,1);set(&t.f,(int)axis,0);
        CHECK(cs_mac_motion_service(&t.m)!=CS_MAC_MOTION_EMPTY);
        CHECK(t.m.motion_drops==0u);
        /* Overflow stays a hard loss, distinct from drops. */
        for(unsigned invalid=0;invalid<3;invalid++){
            RUN(ready(&t,0,0,0));
            if(invalid==0u)t.in.count=CS_INBOX_CAPACITY+1u;
            else if(invalid==1u)t.in.head=CS_INBOX_CAPACITY;
            else t.in.claimed=CS_INBOX_CLAIM_BASELINE;
            t.f.hook_at=t.f.pbs+1u;t.f.hook_ch=(int)axis;t.f.hook_val=1;
            CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_FAULT);
            CHECK(t.m.motion_drops==0u);
        }
        RUN(ready(&t,0,0,0));t.m.motion_drops=UINT32_MAX;
        set(&t.f,(int)axis,1);set(&t.f,(int)axis,0);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.m.motion_drops==UINT32_MAX && t.m.losses==0u);
        RUN(ready(&t,0,0,0));
        for(unsigned i=0;i<CS_INBOX_CAPACITY;i++){
            set(&t.f,(int)axis,(int)((i+1u)&1u));
            CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK);
        }
        set(&t.f,(int)axis,(int)((CS_INBOX_CAPACITY+1u)&1u));
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_OVERFLOW);
        CHECK(t.m.motion_drops==0u && t.m.losses==1u);
    }
#endif
    for(unsigned raw=0;raw<4;raw++)for(unsigned phase=0;phase<4;phase++)
    for(unsigned invert=0;invert<4;invert++)for(unsigned pending=0;pending<4;pending++)
    for(unsigned change=0;change<8;change++)for(unsigned slot=2;slot<=3;slot++){
        int result;
        unsigned changed_phase=change>>1;
        int loss=(change&1u)!=0;
        int drop=!loss && (changed_phase&pending)!=0;
        RUN(ready(&t,raw,phase,invert));
        if(pending&1u)set(&t.f,0,(int)((raw^1u)&1u));
        if(pending&2u)set(&t.f,1,(int)(((raw^2u)>>1)&1u));
        t.f.hook_at=t.f.pbs+slot;t.f.hook_xor=(uint8_t)(change<<3);
        result=cs_mac_motion_service(&t.m);
#if !CLASSICK_MAC_SHORT_COLLECT
        loss=change!=0u;drop=0;
#else
        CHECK(t.m.motion_drops==(unsigned)drop);
#endif
        CHECK(result==(loss?CS_MAC_MOTION_LOST:drop?CS_MAC_MOTION_EMPTY:
            pending?CS_MAC_MOTION_OK:CS_MAC_MOTION_EMPTY));
        CHECK(t.m.losses==(unsigned)loss);
        if(!loss && !drop && pending){
            int dx=0,dy=0;
            if(pending&1u)dx=(((raw^1u^invert)&1u)==(phase&1u))?1:-1;
            if(pending&2u)dy=((((raw^2u^invert)>>1)&1u)!=((phase>>1)&1u))?1:-1;
            CHECK(t.in.count==1u);
            CHECK(t.in.records[t.in.head].dx==dx && t.in.records[t.in.head].dy==dy);
        }
        ++cases;
    }
    for(unsigned axis=0;axis<2;axis++){
        RUN(ready(&t,0,0,0));
        set(&t.f,(int)axis,1);set(&t.f,(int)axis,0);
#if CLASSICK_MAC_SHORT_COLLECT
        /* Hidden 0->1->0 pair: dropped, rebaselined to post-ack 0, latch still
         * closed; next pass is pending/no-change, dropped once, then clean. */
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.m.losses==0u && t.m.motion_drops==1u && t.in.count==0u);
        CHECK((axis?t.m.last_raw_b:t.m.last_raw_a)==0u && rr3(&t.f)==(axis?1u:8u));
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.m.motion_drops==2u && rr3(&t.f)==0u && t.m.losses==0u);
#else
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
#endif
        /* Three hidden same-direction edges collapse to one. Neither the
         * strict nor relaxed sampler can recover the unobserved count. */
        RUN(ready(&t,0,0,0));
        set(&t.f,(int)axis,1);t.f.pb^=axis?PBY:PBX;
        set(&t.f,(int)axis,0);t.f.pb^=axis?PBY:PBX;
        set(&t.f,(int)axis,1);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK);
        CHECK(t.in.count==1u && t.m.losses==0u);
        CHECK(axis?t.in.records[t.in.head].dy==1:t.in.records[t.in.head].dx==-1);
    }
    printf("Axis-phase: %u checks in %u matrix cases plus hidden-edge controls.\n",g_checks,cases);
    return 0;
}
