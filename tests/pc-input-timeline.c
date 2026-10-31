/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* CSPTIM01 observer: halt stubbed with longjmp; literal expected words. */
#include "../platform/pc/input-timeline.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
static jmp_buf done;
static uint64_t got[8];
static unsigned checks;
static int ret;
static cs_pc_window_session s, snap;
static cs_pc_input_timeline r, copy;
#define C(x) do {++checks;if(!(x)){printf("timeline FAIL:%d:%s\n",__LINE__,#x);return 1;}}while(0)
#define OK() C(obs(&r,&s)==0 && ret==0)
#define HALT() C(obs(&r,&s)==1)
#define READY() (s.bus.phase=CS_PS2_BUS_READY,s.bus.result=CS_PS2_READY)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{memcpy(got,values,sizeof(got));longjmp(done,1);}
/* 0 returned, 1 halted, 2 session mutated. */
static int obs(void *ctx,const cs_pc_window_session *p)
{
    memset(got,0xA5,sizeof(got)); ret=-1;
    if(p) memcpy(&snap,p,sizeof(snap));
    if(setjmp(done)==0){
        ret=cs_pc_input_timeline_observe(ctx,0,p);
        return (p && memcmp(&snap,p,sizeof(snap))) ? 2 : 0;
    }
    return (p && memcmp(&snap,p,sizeof(snap))) ? 2 : 1;
}
static void reset(void){memset(&s,0,sizeof(s));s.magic=CS_PWS_MAGIC;s.phase=CS_PWS_RUNNING;}
int main(void)
{
    unsigned i;
    /* NULL, unstarted recorder, NULL and uninitialized session. */
    reset(); READY();
    C(cs_pc_input_timeline_begin(0)==1);
    C(obs(0,&s)==0 && ret==0);
    memset(&r,0,sizeof(r));
    for(i=0;i<8u;++i){++s.bus.auxiliary;s.drained=1;OK();}
    C(r.primed==0 && r.count==0);
    C(cs_pc_input_timeline_begin(&r)==0);
    C(obs(&r,0)==0 && ret==0);
    s.magic=0; memcpy(&copy,&r,sizeof(r));
    for(i=0;i<8u;++i){++s.bus.auxiliary;OK();}
    C(memcmp(&copy,&r,sizeof(r))==0);
    /* Never READY: no baseline, no stop, no zero-byte claim. */
    reset(); C(cs_pc_input_timeline_begin(&r)==0); s.drained=1;
    for(i=0;i<4u;++i){++s.bus.auxiliary;OK();}
    C(r.baseline==0 && r.count==0);
    /* Phase alone does not establish READY; a marker on exit is not one. */
    reset(); READY(); s.bus.result=CS_PS2_ACTIVE; s.drained=1;
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    C(r.baseline==0 && r.count==0);
    READY(); s.drained=0; OK(); C(r.baseline==1);
    s.bus.phase=0; s.drained=1; OK(); C(r.count==0);
    /* Startup boundary and READY interruption: no bytes claimed, marker only post-READY. */
    reset(); s.owner.pointer.x=10; s.owner.pointer.y=20; s.bus.auxiliary=5;
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    READY(); s.bus.auxiliary=9; s.drained=1; OK();
    C(r.baseline==1 && r.count==0);
    s.drained=0; s.bus.phase=0; s.bus.auxiliary=12; OK();
    READY(); s.bus.auxiliary=15; OK();
    s.drained=1; HALT();
    C(got[0]==UINT64_C(0x43535054494D3031));
    C(got[1]==UINT64_C(0x0014000A02020000));
    for(i=2;i<8u;++i) C(got[i]==0);
    /* Full: order, each field, gap, error delta, caps; full beats marker. */
    reset(); READY(); s.owner.pointer.x=511; s.owner.pointer.y=341; s.bus.auxiliary=100;
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    s.bus.auxiliary=101; s.bus.last_byte=0xC8u; s.bus.decoder.used=1;
    s.owner.phase=2; s.owner.stage=3; OK();
    s.bus.auxiliary=104; s.bus.last_byte=0x0A; s.bus.decoder.used=2;
    s.delivery.kind=CS_PS2_DELIVERY_PACKET; ++s.bus.errors;
    s.owner.phase=4; s.owner.stage=5; OK();
    s.bus.auxiliary=105; s.bus.last_byte=0xFF; s.bus.decoder.used=0;
    s.delivery.kind=CS_PS2_DELIVERY_LOST; s.phase=CS_PWS_RUNNING;
    s.owner.phase=6; s.owner.stage=7; OK();
    OK(); /* no increment: no entry */
    s.bus.auxiliary=106; s.bus.last_byte=0x11; s.bus.decoder.used=300;
    s.delivery.kind=0; s.phase=CS_PWS_STOPPED; s.owner.phase=0; s.owner.stage=0; OK();
    s.bus.auxiliary=107; s.bus.last_byte=0x22; s.bus.decoder.used=1;
    s.delivery.kind=0x100; s.phase=1; s.owner.phase=1; s.owner.stage=1; OK();
    C(r.count==5);
    s.bus.auxiliary=108; s.bus.last_byte=0x33; s.delivery.kind=0;
    s.owner.phase=0; s.owner.stage=0; s.drained=1; HALT();
    C(got[0]==UINT64_C(0x43535054494D3031));
    C(got[1]==UINT64_C(0x015501FF01030206));
    C(got[2]==UINT64_C(0x03020100000100C8));
    C(got[3]==UINT64_C(0x050401010102010A));
    C(got[4]==UINT64_C(0x07060100020002FF));
    C(got[5]==UINT64_C(0x0000030000FF0011));
    C(got[6]==UINT64_C(0x01010100FF01FF22));
    C(got[7]==UINT64_C(0x0000010000010133));
    /* Partial: entry stored before marker stop; invalid pointer. */
    reset(); READY(); s.owner.pointer.x=-1; s.owner.pointer.y=342;
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    s.bus.auxiliary=1; s.bus.last_byte=0x55; s.bus.decoder.used=1; s.drained=1; HALT();
    C(got[1]==UINT64_C(0xFFFFFFFF02020001));
    C(got[2]==UINT64_C(0x0000010000010055));
    for(i=3;i<8u;++i) C(got[i]==0);
    /* Regression and saturation latch unreliable without entries. */
    reset(); READY(); s.bus.auxiliary=10;
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    s.bus.auxiliary=5; OK(); C(r.count==0 && r.unreliable==1);
    s.bus.auxiliary=6; s.bus.last_byte=0x77; OK();
    s.bus.auxiliary=UINT32_MAX; OK(); C(r.count==1);
    s.drained=1; HALT();
    C(got[1]==UINT64_C(0x0000000002030001));
    C(got[2]==UINT64_C(0x0000010000000077));
    for(i=3;i<8u;++i) C(got[i]==0);
    /* Gap saturates at 255; zero byte is a real recorded byte. */
    reset(); READY();
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    s.bus.auxiliary=300; OK(); C(r.gaps==255);
    s.bus.auxiliary=301; OK(); C(r.gaps==255 && r.count==2);
    s.drained=1; HALT();
    C(got[1]==UINT64_C(0x000000000202FF02));
    C(got[2]==UINT64_C(0x0000010000000000) && got[3]==UINT64_C(0x0000010000000000));
    /* Out-of-domain bytes and saturated errors are explicit uncertainty. */
    reset(); READY();
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    s.bus.auxiliary=1; s.bus.last_byte=256u; s.bus.errors=UINT32_MAX; s.drained=1; HALT();
    C((got[1]&UINT64_C(0x10000))!=0u && (got[2]&UINT64_C(0xff))==UINT64_C(0xff));
    /* Fault is observable without a keyboard marker; no input is invented. */
    reset(); READY();
    C(cs_pc_input_timeline_begin(&r)==0); OK();
    s.phase=CS_PWS_FAULTED; HALT();
    C(got[1]==UINT64_C(0x0000000003020000));
    for(i=2;i<8u;++i) C(got[i]==0);
    printf("input timeline:%u assertions\n",checks);
    return 0;
}
