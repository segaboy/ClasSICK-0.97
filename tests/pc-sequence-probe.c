/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Unit oracle for the CSPSEQ01 READY AUX-byte sequence recorder. Literal
 * expectations; the session is never written by the observer. */
#include "../platform/pc/input-probe.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#if (CLASSICK_PC_INPUT_PROBE_SEQUENCE+0)!=1
#error build with CLASSICK_PC_INPUT_PROBE_SEQUENCE=1
#endif
static jmp_buf done;
static uint64_t actual[8];
static unsigned halts, checks;
static cs_pc_window_session session, before;
static cs_pc_input_sequence rec, rec_before;
#define C(x) do {++checks;if(!(x)){printf("sequence probe FAIL:%d:%s\n",__LINE__,#x);return 1;}}while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{memcpy(actual,values,sizeof(actual));++halts;longjmp(done,1);}
/* 1 if the observer halted; asserts the session is untouched either way. */
static int run(void *ctx)
{
    memcpy(&before,&session,sizeof before);
    if(setjmp(done)==0){
        int r=cs_pc_input_probe_observe(ctx,0,&session);
        if(r!=0||memcmp(&session,&before,sizeof session)!=0) return -1;
        return 0;
    }
    return memcmp(&session,&before,sizeof session)==0?1:-1;
}
static void ready(void){session.bus.phase=CS_PS2_BUS_READY;session.bus.result=CS_PS2_READY;}
/* One READY AUX byte consumed by the previous poll. */
static int aux(uint8_t b,unsigned err)
{
    session.bus.last_byte=b; ++session.bus.auxiliary; if(err) ++session.bus.errors;
    return run(&rec);
}
int main(void)
{
    unsigned i;
    C(cs_pc_input_sequence_begin(0)==1);
    C(cs_pc_input_probe_observe(0,0,&session)==0);
    /* Unstarted recorder: never records or stops, and stays unchanged. */
    memset(&rec,0,sizeof rec); memcpy(&rec_before,&rec,sizeof rec);
    session.bus.auxiliary=5; C(run(&rec)==0); session.bus.auxiliary=6; C(run(&rec)==0);
    C(memcmp(&rec,&rec_before,sizeof rec)==0);
    C(cs_pc_input_sequence_begin(&rec)==0 && rec.magic==CS_PC_INPUT_SEQUENCE_STARTED);
    C(cs_pc_input_probe_observe(&rec,0,0)==0 && rec.primed==0);
    /* Startup: phase 23 primes; the ACK that enters READY is not recorded. */
    session.bus.phase=23; session.bus.result=CS_PS2_ACTIVE; session.bus.auxiliary=10;
    C(run(&rec)==0 && rec.primed==1 && rec.was_ready==0 && rec.count==0);
    ready(); session.bus.auxiliary=11; session.bus.last_byte=0xFA;
    C(run(&rec)==0 && rec.count==0 && rec.was_ready==1);
    /* Idle READY turns and keyboard bytes (no AUX step) are not recorded. */
    C(run(&rec)==0 && rec.count==0);   /* idle READY turn */
    session.bus.last_byte=0x1E; C(run(&rec)==0 && rec.count==0);
    /* Bytes, error flags and a gap. */
    C(aux(0x08,0)==0 && rec.count==1 && rec.bytes[0]==0x08 && rec.flags==0);
    C(aux(0x48,1)==0 && rec.count==2 && rec.flags==UINT64_C(2));
    session.bus.auxiliary+=2;                       /* two bytes never observed */
    C(aux(0xEE,0)==0 && rec.count==3 && rec.bytes[2]==0xEE && rec.gaps==2u);
    /* A faulted bus stops recording; the first READY turn after it re-primes. */
    session.bus.result=CS_PS2_DEVICE; session.bus.last_byte=0x11; ++session.bus.auxiliary;
    C(run(&rec)==0 && rec.count==3 && rec.was_ready==0);
    ready(); session.bus.last_byte=0x22; ++session.bus.auxiliary;
    C(run(&rec)==0 && rec.count==3 && rec.was_ready==1);
    /* Fill to 48 with a counting pattern; byte 47 carries an error flag. */
    for(i=3;i<47u;++i) C(aux((uint8_t)(0x30u+i),0)==0);
    C(rec.count==47 && halts==0);
    C(aux(0x99,1)==1 && halts==1 && rec.count==48);
    C(actual[0]==UINT64_C(0x4353505345513031));
    C(actual[1]==(UINT64_C(2)|(UINT64_C(1)<<47)|(UINT64_C(2)<<48)|(UINT64_C(48)<<56)));
    C(actual[2]==UINT64_C(0x3736353433EE4808));
    C(actual[3]==UINT64_C(0x3F3E3D3C3B3A3938));
    C(actual[7]==UINT64_C(0x995E5D5C5B5A5958));
    /* Full recorder: further observations neither record nor halt. */
    C(aux(0x01,0)==0 && halts==1 && rec.count==48);
    /* Gap saturation and the saturating AUX counter. */
    C(cs_pc_input_sequence_begin(&rec)==0); session.bus.auxiliary=0;
    C(run(&rec)==0);
    session.bus.auxiliary+=300; C(aux(0x08,0)==0 && rec.gaps==255u && rec.count==1);
    session.bus.auxiliary=UINT32_MAX-1u; C(run(&rec)==0 && rec.count==2);   /* gap again, saturated */
    C(aux(0x09,0)==0 && rec.count==3);              /* reaches UINT32_MAX */
    session.bus.last_byte=0x0A; C(run(&rec)==0 && rec.count==3);  /* saturated: no step */
    printf("sequence probe:%u assertions\n",checks);
    return 0;
}
