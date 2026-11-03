/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Compare the actual collectors on our source-informed DCD latch model. */
#define main old_fixture_main
#include "mac-motion-latch.c"
#undef main
static unsigned short_reads[2];
static uint8_t scc_read(void *c,uint32_t ch,uint32_t reg)
{
    if(reg==0u)short_reads[ch]++;
    return ior(c,ch,reg);
}
static void scc_write(void *c,uint32_t ch,uint32_t reg,uint8_t v){iow(c,ch,reg,v);}
static uint8_t motion_pb(void *c){return iopb(c);}
#define cs_mac_native_collect reference_collect
#include "../platform/mac128k/fast-collect.inc"
#undef cs_mac_native_collect
#undef CLASSICK_MAC_SHORT_COLLECT
#define CLASSICK_MAC_SHORT_COLLECT 1
#define cs_mac_native_collect proposed_collect
#include "../platform/mac128k/fast-collect.inc"
#undef cs_mac_native_collect
int main(void)
{
    unsigned pending;
    int hook;
    for(pending=0;pending<4;pending++)for(hook=-1;hook<2;hook++){
        fx_t seed,ref,opt;
        uint8_t a[10]={0},b[10]={0};
        fx_init(&seed,PBB);both(&seed);
        if(pending&1u)set(&seed,0,1);
        if(pending&2u)set(&seed,1,1);
        seed.hook_at=2;seed.hook_ch=hook;seed.hook_xor=PBX;
        if(hook>=0)seed.hook_val=seed.c[hook].live?0:1;
        ref=seed;opt=seed;short_reads[0]=short_reads[1]=0;
        reference_collect(&ref,a);
        CHECK(short_reads[0]==3 && short_reads[1]==3);
        short_reads[0]=short_reads[1]=0;proposed_collect(&opt,b);
        CHECK(short_reads[0]==((pending&1u)?2u:3u));
        CHECK(short_reads[1]==((pending&2u)?2u:3u));
        CHECK(memcmp(a,b,10)==0);
        CHECK(rr3(&ref)==rr3(&opt));
        CHECK(ref.c[0].resets==opt.c[0].resets && ref.c[1].resets==opt.c[1].resets);
    }
    printf("Short collector: %u model checks across 12 scenarios.\n",g_checks);
    return 0;
}
