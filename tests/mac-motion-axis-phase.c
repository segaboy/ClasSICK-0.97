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
int main(void)
{
    T t; unsigned cases=0;
    for(unsigned raw=0;raw<4;raw++)for(unsigned phase=0;phase<4;phase++)
    for(unsigned invert=0;invert<4;invert++)for(unsigned pending=0;pending<4;pending++)
    for(unsigned change=0;change<8;change++)for(unsigned slot=2;slot<=3;slot++){
        int result;
        unsigned changed_phase=change>>1;
        int loss=(change&1u)!=0 || (changed_phase&pending)!=0;
        RUN(ready(&t,raw,phase,invert));
        if(pending&1u)set(&t.f,0,(int)((raw^1u)&1u));
        if(pending&2u)set(&t.f,1,(int)(((raw^2u)>>1)&1u));
        t.f.hook_at=t.f.pbs+slot;t.f.hook_xor=(uint8_t)(change<<3);
        result=cs_mac_motion_service(&t.m);
#if !CLASSICK_MAC_SHORT_COLLECT
        loss=change!=0u;
#endif
        CHECK(result==(loss?CS_MAC_MOTION_LOST:pending?CS_MAC_MOTION_OK:CS_MAC_MOTION_EMPTY));
        CHECK(t.m.losses==(unsigned)loss);
        if(!loss && pending){
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
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
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
