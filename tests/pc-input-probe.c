/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/input-probe.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static cs_pc_window_session graph;
static unsigned char saved[sizeof graph];
static uint64_t actual[8];
static jmp_buf jump;
static unsigned calls,checks;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"probe line%d\n",__LINE__);exit(1);}}while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{
    calls++;memcpy(actual,values,sizeof actual);longjmp(jump,1);
}
int main(void)
{
    /* Independent oracle frozen before the worker artifact was inspected. */
    static const uint64_t expected[8]={UINT64_C(0x435350524f424531),UINT64_C(0x0000000200000018),
        UINT64_C(0xffffffff01234567),UINT64_C(0x89abcdef000000c8),UINT64_C(0x000000e900000002),
        UINT64_C(0x0000000000000001),UINT64_C(0x0000123400000056),UINT64_C(0xffffffff00000155)};
    memset(&graph,0xA5,sizeof graph);
    for(unsigned kind=0;kind<2;kind++){
        graph.delivery.kind=kind;memcpy(saved,&graph,sizeof graph);
        CHECK(cs_pc_input_probe_observe(NULL,NULL,&graph)==0);
        CHECK(!calls && !memcmp(saved,&graph,sizeof graph));
    }
    graph.delivery.kind=2;graph.bus.phase=24;graph.bus.errors=UINT32_MAX;
    graph.bus.packets=UINT32_C(0x01234567);graph.bus.auxiliary=UINT32_C(0x89abcdef);
    graph.bus.last_byte=0xC8;graph.bus.decoder.header=0xE9;graph.bus.decoder.used=2;
    graph.owner.pointer.down=0;graph.owner.pointer.suppress=1;
    graph.owner.product.inputs=0x1234;graph.bus.decoder.x=0x56;
    graph.owner.pointer.x=-1;graph.owner.pointer.y=341;
    memcpy(saved,&graph,sizeof graph);
    if(setjmp(jump)==0){(void)cs_pc_input_probe_observe(NULL,NULL,&graph);CHECK(0);}
    CHECK(calls==1 && !memcmp(saved,&graph,sizeof graph));
    for(unsigned i=0;i<8;i++)CHECK(actual[i]==expected[i]);
    printf("Input probe: %u checks; frozen register-word oracle; native backend unexecuted.\n",checks);
    return 0;
}
