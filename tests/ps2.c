/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "ps2-fixture.h"
#include <stdio.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static cs_ps2 state;
static cs_input_record records[8];
static cs_input_queue queue;
static ps2_fixture fixture;
static cs_ps2_io io;
static uint64_t ticks;
static void setup(void)
{
    pf_init(&fixture); io=pf_io(&fixture); ticks=0;
    CHECK(cs_ps2_begin(&state)==0); CHECK(cs_input_init(&queue,records,8)==CS_INPUT_OK);
}
static uint32_t poll(void)
{
    unsigned sr=fixture.status_reads,dr=fixture.data_reads,w=fixture.writes;
    uint32_t r=cs_ps2_poll(&state,&io,++ticks,&queue);
    CHECK(fixture.status_reads-sr<=1 && fixture.data_reads-dr+fixture.writes-w<=1);
    CHECK(fixture.violations==0); return r;
}
static void ready(void)
{
    for(unsigned i=0;i<256 && state.result==0;++i) (void)poll();
    CHECK(state.result==1 && state.phase==30 && queue.count==0);
}
static void startup(void)
{
    static const uint8_t ports[]={0x64,0x64,0x64,0x64,0x60,0x64,0x64,0x64,0x64,0x60,0x64,0x64,0x64,0x60,0x60,0x60,0x60,0x60};
    static const uint8_t values[]={0xAD,0xA7,0x20,0x60,0x34,0xAA,0xAD,0xA7,0x60,0x34,0x20,0xAB,0xAE,0xFF,0xF5,0xF0,2,0xF4};
    setup(); pf_byte(&fixture,0x29,0); pf_byte(&fixture,0x76,0x20); ready();
    CHECK(state.drained==2 && state.configuration==0x34 && fixture.writes==sizeof values);
    CHECK(memcmp(fixture.ports,ports,sizeof ports)==0 && memcmp(fixture.values,values,sizeof values)==0);
    CHECK(state.events==0 && state.errors==0 && state.resends==0);
    /* Both retries are allowed separately for every keyboard command byte. */
    for(unsigned i=13;i<sizeof values;++i) {
        setup(); fixture.resend_value=values[i]; fixture.resend_left=2; ready();
        CHECK(state.resends==2 && fixture.writes==sizeof values+2);
        setup(); fixture.resend_value=values[i]; fixture.resend_left=3;
        for(unsigned j=0;j<128 && state.result==0;++j) (void)poll();
        CHECK(state.result==CS_PS2_DEVICE && state.resends==2);
    }
    setup(); fixture.blocked=1; CHECK(poll()==0 && fixture.writes==0 && state.phase==0);
    fixture.blocked=0; ready();
    setup(); for(unsigned i=0;i<64;++i) pf_byte(&fixture,0x29,0); ready(); CHECK(state.drained==64);
    setup(); for(unsigned i=0;i<65;++i) pf_byte(&fixture,0x29,0);
    for(unsigned i=0;i<100 && state.result==0;++i) (void)poll();
    CHECK(state.result==CS_PS2_CONTROLLER && state.drained==64 && fixture.data_reads==64);
}
static void errors(void)
{
    static const unsigned wait[]={4,8,15,17,20,21,23,25,27,29};
    setup(); CHECK(cs_ps2_begin(NULL)==CS_PS2_ARGUMENT);
    CHECK(cs_ps2_poll(NULL,&io,0,&queue)==CS_PS2_ARGUMENT);
    CHECK(cs_ps2_poll(&state,NULL,0,&queue)==CS_PS2_ARGUMENT);
    CHECK(cs_ps2_poll(&state,&io,0,NULL)==CS_PS2_ARGUMENT);
    for(unsigned phase=0;phase<30;++phase) {
        unsigned limit=phase==21?CS_PS2_BAT_TICKS:phase==8?CS_PS2_TEST_TICKS:CS_PS2_SHORT_TICKS;
        setup(); state.phase=phase; state.started=1; state.since=10; state.last_tick=10;
        CHECK(cs_ps2_poll(&state,&io,10u+limit,&queue)==CS_PS2_TIMEOUT && fixture.status_reads==0);
        CHECK(poll()==CS_PS2_TIMEOUT && fixture.status_reads==0);
        setup(); state.phase=phase; state.polls=CS_PS2_PHASE_POLLS; state.started=1;
        CHECK(poll()==CS_PS2_TIMEOUT && fixture.status_reads==0);
    }
    for(unsigned i=0;i<sizeof wait/sizeof wait[0];++i) {
        setup(); state.phase=wait[i]; pf_byte(&fixture,0x55,0x40); CHECK(poll()==CS_PS2_CONTROLLER);
        setup(); state.phase=wait[i]; pf_byte(&fixture,0x29,0x20); CHECK(poll()==0 && state.phase==wait[i] && state.auxiliary==1);
    }
    setup(); state.phase=20; pf_byte(&fixture,0x29,0); CHECK(poll()==CS_PS2_PROTOCOL);
    setup(); state.phase=20; pf_byte(&fixture,0xFC,0); CHECK(poll()==CS_PS2_DEVICE);
    setup(); state.phase=21; pf_byte(&fixture,0x55,0); CHECK(poll()==CS_PS2_DEVICE);
    setup(); fixture.self_test=0xFC; for(unsigned i=0;i<32 && state.result==0;++i) (void)poll(); CHECK(state.result==CS_PS2_CONTROLLER);
    setup(); fixture.interface_test=1; for(unsigned i=0;i<32 && state.result==0;++i) (void)poll(); CHECK(state.result==CS_PS2_CONTROLLER);
    setup(); state.phase=15; pf_byte(&fixture,0x74,0); CHECK(poll()==CS_PS2_CONTROLLER);
    setup(); state.phase=31; CHECK(poll()==CS_PS2_STATE && fixture.status_reads==0);
    setup(); state.started=1; state.last_tick=2; CHECK(poll()==CS_PS2_STATE && fixture.status_reads==0);
    setup(); state.phase=20; state.started=1; state.since=0; state.last_tick=0;
    pf_byte(&fixture,0xFA,0); CHECK(cs_ps2_poll(&state,&io,CS_PS2_SHORT_TICKS-1u,&queue)==0 && state.phase==21);
}
static void event(uint32_t key,uint32_t action,uint32_t repeat,uint32_t sequence)
{
    cs_input_record r; CHECK(cs_input_pop(&queue,&r)==CS_INPUT_OK);
    CHECK(r.event.source==3 && r.event.key==key && r.event.action==action && r.event.repeat==repeat && r.sequence==sequence);
}
static void byte(uint8_t v,uint8_t flags) { pf_byte(&fixture,v,flags); CHECK(poll()==1); }
static void stream(void)
{
    static const uint8_t pause[]={0xE1,0x14,0x77,0xE1,0xF0,0x14,0xF0,0x77};
    setup(); ready(); byte(0x29,0); byte(0x29,0); byte(0xF0,0); byte(0x29,0);
    byte(0x76,0); byte(0xF0,0); byte(0x76,0);
    event(1,1,0,1); event(1,1,1,2); event(1,2,0,3); event(2,1,0,4); event(2,2,0,5);
    byte(0xF0,0); byte(0x29,0); CHECK(queue.count==0 && state.held==0);
    byte(0xE0,0); byte(0x29,0); byte(0xE0,0); byte(0xF0,0); byte(0x76,0);
    for(unsigned i=0;i<sizeof pause;++i) byte(pause[i],0);
    CHECK(queue.count==0 && state.pause==0 && state.extended==0);
    byte(0x29,0); byte(0xF0,0); byte(0x76,0x20); byte(0x29,0);
    event(1,1,0,6); event(1,2,0,7); CHECK(state.auxiliary==1);
    byte(0x29,0); byte(0x76,0x80); CHECK(state.errors==1 && state.held==0);
    byte(0x29,0); event(1,1,0,8); event(1,1,0,9);
    for(unsigned v=0;v<256;++v) {
        setup(); ready(); byte((uint8_t)v,0);
        CHECK(queue.count==((v==0x29 || v==0x76)?1u:0u));
    }
}
static void overflow(void)
{
    setup(); ready(); for(unsigned i=0;i<8;++i) byte(0x29,0);
    byte(0xF0,0); byte(0x29,0); CHECK(state.dropped==1 && state.held==0 && queue.count==8);
    CHECK(cs_input_reset(&queue)==CS_INPUT_OK); byte(0x29,0); event(1,1,0,1);
    state.events=UINT32_MAX; byte(0x29,0); CHECK(state.events==UINT32_MAX);
    state.errors=UINT32_MAX; byte(0xFF,0); CHECK(state.errors==UINT32_MAX);
    queue.next_sequence=0; pf_byte(&fixture,0x76,0); CHECK(poll()==CS_PS2_QUEUE);
}
int main(int argc,char **argv)
{
    const char *s=argc>1?argv[1]:"";
    if(strcmp(s,"startup")==0) startup(); else if(strcmp(s,"errors")==0) errors();
    else if(strcmp(s,"stream")==0) stream(); else if(strcmp(s,"overflow")==0) overflow();
    else return 2;
    if(failures) return 1;
    printf("PS/2 %s: PASS\n",s); return 0;
}
