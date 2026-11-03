/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Exercise the original source-informed latch model, not an emulator model. */
#define main existing_latch_fixture_main
#include "mac-motion-latch.c"
#undef main

static unsigned rr3_reads, bad_late, edge_before_late, edge_after_late;
static uint8_t checked_read(void *c,uint32_t ch,uint32_t reg)
{
    uint8_t value;
    if (ch==0u && reg==3u) {
        rr3_reads++;
        if (bad_late && rr3_reads==2u) return 2u;
        if (edge_before_late && rr3_reads==2u) set(c,0,1);
    }
    value=ior(c,ch,reg);
    if (ch==0u && reg==3u && edge_after_late && rr3_reads==2u) set(c,0,1);
    return value;
}
#if CLASSICK_MAC_FAST_COLLECT
static uint8_t scc_read(void *c,uint32_t ch,uint32_t reg){return checked_read(c,ch,reg);}
static void scc_write(void *c,uint32_t ch,uint32_t reg,uint8_t v){iow(c,ch,reg,v);}
static uint8_t motion_pb(void *c){return iopb(c);}
#include "../platform/mac128k/fast-collect.inc"
#endif

static int start(T *t)
{
    cs_mac_motion_io io={0};
    memset(t,0,sizeof *t);fx_init(&t->f,PBB);rr3_reads=0;bad_late=0;
    edge_before_late=0;edge_after_late=0;
    io.cookie=&t->f;io.read=checked_read;io.write=iow;io.pb=iopb;
#if CLASSICK_MAC_FAST_COLLECT
    io.collect=cs_mac_native_collect;
#endif
    CHECK(cs_inbox_begin(&t->in)==CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(&t->m,&io,&t->in,0u)==CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_arm(&t->m)==CS_MAC_MOTION_OK);
    return 0;
}

int main(void)
{
    T t;
    for(unsigned ch=0;ch<2u;ch++){
        RUN(start(&t));
        t.f.hook_at=1;t.f.hook_ch=(int)ch;t.f.hook_val=1;
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY);
        CHECK(t.in.phase==CS_INBOX_WAIT && !t.m.have_phase && !t.in.count);
        CHECK(rr3(&t.f)==(ch?1u:8u) && t.f.c[ch].resets==0 && rr3_reads==2u);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_BASELINED);
        CHECK(t.in.phase==CS_INBOX_PRIMING && t.f.c[ch].resets==1 && !rr3(&t.f));
        CHECK((ch?t.m.last_raw_b:t.m.last_raw_a)==1u && t.m.losses==0u);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY && t.m.losses==0u);
    }
    /* A latch observed before the first read is acknowledged normally. */
    RUN(start(&t));set(&t.f,0,1);
    CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_BASELINED);
    CHECK(t.f.c[0].resets==1 && !rr3(&t.f) && rr3_reads==2u);
    /* A subsequent coherent edge remains a staged sample in PRIMING. */
    RUN(start(&t));CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_BASELINED);
    set(&t.f,0,1);CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK);
    CHECK(t.in.phase==CS_INBOX_PRIMING && t.in.count==2u && t.m.losses==0u);
    /* Edge just before the late read defers; just after it remains an edge. */
    RUN(start(&t));edge_before_late=1;
    CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY && t.in.phase==CS_INBOX_WAIT);
    CHECK(rr3(&t.f)==8 && t.f.c[0].resets==0 && !t.m.have_phase);
    RUN(start(&t));edge_after_late=1;
    CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_BASELINED && rr3(&t.f)==8);
    CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_OK && t.in.count==2u && t.m.losses==0u);
    /* Unsupported bits arriving only on the late read are a hard fault. */
    RUN(start(&t));bad_late=1;
    CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_FAULT);
    CHECK(t.m.phase==CS_MAC_MOTION_FAILED && t.m.stage==CS_MAC_STAGE_PENDING &&
          t.m.cause==CS_MAC_CAUSE_PENDING && !t.in.count && t.f.wr9==2);
    /* An unstable WAIT pass never performs the added read. */
    RUN(start(&t));t.f.hook_at=2;t.f.hook_ch=0;t.f.hook_val=1;
    CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_EMPTY && rr3_reads==1u);
    CHECK(t.in.phase==CS_INBOX_WAIT && !t.in.count && rr3(&t.f)==8);
    /* Two same-direction model steps can be indistinguishable from a bounce:
     * the first latch freezes DCD while the second phase changes and returns.
     * A reset can expose the second DCD level only in the third snapshot.
     * Neither that changed snapshot nor a still-pending latch proves direction. */
    for(unsigned ch=0;ch<2u;ch++){
        RUN(start(&t));
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_BASELINED);
        CHECK(cs_inbox_claim(&t.in,t.m.epoch)==CS_INBOX_BASELINE_ITEM);
        CHECK(cs_inbox_complete(&t.in,t.m.epoch,0)==CS_INBOX_OK);
        set(&t.f,(int)ch,1);
        t.f.pb ^= ch ? PBY : PBX;
        set(&t.f,(int)ch,0);
        t.f.pb ^= ch ? PBY : PBX;
        CHECK(t.f.c[ch].closed && t.f.c[ch].latched==1 && t.f.c[ch].live==0);
        CHECK(cs_mac_motion_service(&t.m)==CS_MAC_MOTION_LOST);
        CHECK(t.in.phase==CS_INBOX_LOSS && !t.in.count && t.m.losses==1u);
        CHECK(t.m.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
    }
    printf("WAIT latch experiment: %u checks; synthetic IO, hardware timing unknown.\n",g_checks);
    return 0;
}
