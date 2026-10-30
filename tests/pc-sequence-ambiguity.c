/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent counterexample:controller-status and native sync losses can
 * produce identical CSPSEQ01 records. This is original model evidence only. */
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "../platform/pc/input-probe.h"
#include "ps2-bus-fixture.h"
static jmp_buf stopped;
static uint64_t actual[8], normal[8];
static cs_pc_window_session session;
static cs_pc_input_sequence recorder;
static bus_fixture hw;
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t words[8])
{
    memcpy(actual, words, sizeof actual);
    longjmp(stopped, 1);
}
static int trial(uint8_t status)
{
    cs_ps2_io io;
    uint64_t tick=0;
    unsigned turns=0;
    memset(&session,0,sizeof session);
    bf_init(&hw);io=bf_io(&hw);
    if(cs_input_init(&session.queue,session.records,CS_PWS_QUEUE_CAPACITY)!=CS_INPUT_OK ||
       cs_ps2_bus_begin(&session.bus)!=CS_PS2_ACTIVE)return 1;
    while(session.bus.result!=CS_PS2_READY){
        if(++turns>5000u || cs_ps2_bus_poll_delivery(&session.bus,&io,tick+=1000u,
           &session.queue,&session.delivery)>CS_PS2_READY)return 2;
    }
    if(cs_pc_input_sequence_begin(&recorder)!=0 ||
       cs_pc_input_probe_observe(&recorder,NULL,&session)!=0)return 3;
    pf_byte(&hw.key,0x01,status);
    if(cs_ps2_bus_poll_delivery(&session.bus,&io,tick+1000u,&session.queue,
       &session.delivery)!=CS_PS2_READY || session.delivery.kind!=CS_PS2_DELIVERY_LOST ||
       session.bus.errors!=1u || session.bus.decoder.used!=0u)return 4;
    if(setjmp(stopped)==0){
        (void)cs_pc_input_probe_observe(&recorder,NULL,&session);
        return 5;
    }
    return hw.key.violations?6:0;
}
int main(void)
{
    int code=trial(0x20u);
    if(code!=0){printf("sync-loss trial:%d\n",code);return 1;}
    memcpy(normal,actual,sizeof normal);
    code=trial(0xA0u);
    if(code!=0){printf("status-loss trial:%d\n",code);return 1;}
    if(memcmp(normal,actual,sizeof normal)!=0 ||
       actual[0]!=UINT64_C(0x4353505345513031) ||
       actual[1]!=UINT64_C(0x0100000000000001) || actual[2]!=1u){
        puts("different recorder output for ambiguous losses");return 1;
    }
    puts("PASS:distinct native loss causes produce identical observed words");
    return 0;
}
