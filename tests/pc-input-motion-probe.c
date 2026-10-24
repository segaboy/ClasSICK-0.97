/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/input-probe.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if CLASSICK_PC_INPUT_PROBE_MOTION!=1
#error build this test with -DCLASSICK_PC_INPUT_PROBE_MOTION=1
#endif
static cs_pc_window_session graph;
static unsigned char saved[sizeof graph],trace[16],savedtrace[16],ctx[16],savedctx[16];
static uint64_t actual[8];
static jmp_buf jump;
static unsigned calls,checks,armed;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"motion probe line%d\n",__LINE__);exit(1);}}while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{
    if(!armed){fprintf(stderr,"unexpected motion-probe capture\n");exit(1);}
    calls++;memcpy(actual,values,sizeof actual);longjmp(jump,1);
}
static void snap(void){memcpy(saved,&graph,sizeof graph);memcpy(savedtrace,trace,sizeof trace);memcpy(savedctx,ctx,sizeof ctx);}
static int same(void){return !memcmp(saved,&graph,sizeof graph)&&!memcmp(savedtrace,trace,sizeof trace)&&!memcmp(savedctx,ctx,sizeof ctx);}
static void ignored(unsigned kind,int dx,int dy)
{
    graph.delivery.kind=kind;graph.delivery.packet.dx=dx;graph.delivery.packet.dy=dy;snap();
    CHECK(cs_pc_input_probe_observe(ctx,trace,&graph)==0);
    CHECK(!calls&&same());
}
static void capture(const uint64_t *e)
{
    snap();
    armed=1;
    if(setjmp(jump)==0){(void)cs_pc_input_probe_observe(ctx,trace,&graph);CHECK(0);}
    armed=0;
    CHECK(calls==1&&same());
    for(unsigned i=0;i<8;i++)CHECK(actual[i]==e[i]);
    calls=0;
}
static void at(uint32_t n,unsigned kind,int dx,int dy,const uint64_t *e)
{
    graph.bus.packets=n;graph.delivery.kind=kind;graph.delivery.packet.dx=dx;graph.delivery.packet.dy=dy;
    if(e)capture(e);else ignored(kind,dx,dy);
}
int main(void)
{
    /* Literal oracles; author supplied implementation and tests. */
    static const uint64_t mixed[8]={UINT64_C(0x4353504D4F543031),UINT64_C(0xFFFFFFFF01020304),
        UINT64_C(0x000000FD0000ABCD),UINT64_C(0xFFFFFFFD00000005),UINT64_C(0x0000000600000003),
        UINT64_C(0xFFFFFFF9FFFFFFF7),UINT64_C(0xFFFFFF9C0000012C),UINT64_C(0x1234567800000001)};
    static const uint64_t xonly[8]={UINT64_C(0x4353504D4F543031),UINT64_C(0xFFFFFFFF01020304),
        UINT64_C(0x000000FD0000ABCD),UINT64_C(0x0000000400000000),UINT64_C(0x0000000600000003),
        UINT64_C(0xFFFFFFF9FFFFFFF7),UINT64_C(0xFFFFFF9C0000012C),UINT64_C(0x1234567800000001)};
    static const uint64_t yonly[8]={UINT64_C(0x4353504D4F543031),UINT64_C(0xFFFFFFFF01020304),
        UINT64_C(0x000000FD0000ABCD),UINT64_C(0x00000000FFFFFFFF),UINT64_C(0x0000000600000003),
        UINT64_C(0xFFFFFFF9FFFFFFF7),UINT64_C(0xFFFFFF9C0000012C),UINT64_C(0x1234567800000001)};
    memset(&graph,0xA5,sizeof graph);memset(trace,0x5A,sizeof trace);memset(ctx,0x3C,sizeof ctx);
    snap();
    CHECK(cs_pc_input_probe_observe(ctx,trace,NULL)==0);
    CHECK(!calls&&same());
    ignored(0,7,7);ignored(2,7,7);ignored(1,0,0);
    graph.bus.packets=UINT32_MAX;graph.bus.errors=UINT32_C(0x01020304);
    graph.bus.decoder.header=0xFD;graph.bus.decoder.x=0x1AB;graph.bus.last_byte=0x1CD;
    graph.delivery.packet.buttons=6;graph.owner.pointer.down=1;graph.owner.pointer.suppress=1;
    graph.owner.pointer.x=-7;graph.owner.pointer.y=-9;
    graph.owner.window.x=-100;graph.owner.window.y=300;
    graph.owner.product.inputs=UINT32_C(0x12345678);graph.owner.window.dragging=1;
    graph.delivery.kind=1;
    graph.delivery.packet.dx=-3;graph.delivery.packet.dy=5;capture(mixed);
    graph.delivery.packet.dx=4;graph.delivery.packet.dy=0;capture(xonly);
    graph.delivery.packet.dx=0;graph.delivery.packet.dy=-1;capture(yonly);
    /* Realistic reset parser state: header/X cleared, only rawY retained. */
    graph.bus.errors=0;graph.bus.decoder.header=0;graph.bus.decoder.x=0;graph.bus.last_byte=1;
    graph.delivery.packet.buttons=2;graph.owner.pointer.down=1;graph.owner.pointer.suppress=0;
    graph.owner.pointer.x=10;graph.owner.pointer.y=20;graph.owner.window.x=30;graph.owner.window.y=40;
    graph.owner.product.inputs=3;graph.owner.window.dragging=1;
    {
        static const uint64_t two[8]={UINT64_C(0x4353504D4F543031),UINT64_C(0x0000000200000000),
            UINT64_C(0x0000000000000001),UINT64_C(0x000000FFFFFFFF01),UINT64_C(0x0000000200000001),
            UINT64_C(0x0000000A00000014),UINT64_C(0x0000001E00000028),UINT64_C(0x0000000300000001)};
        uint64_t w[8];
        const int later=CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET;
        memcpy(w,two,sizeof w);
        w[1]=0;at(0,1,255,-255,later?NULL:w);
        w[1]=UINT64_C(0x0000000100000000);at(1,1,255,-255,later?NULL:w);
        at(1,0,255,-255,NULL);at(1,2,255,-255,NULL);at(1,1,0,0,NULL);
        at(2,0,255,-255,NULL);at(2,2,255,-255,NULL);at(2,1,0,0,NULL);
        at(2,1,255,-255,two);
        memcpy(w,two,sizeof w);
        w[3]=UINT64_C(0x0000000400000000);at(2,1,4,0,w);
        w[3]=UINT64_C(0x00000000FFFFFFFF);at(2,1,0,-1,w);
        w[1]=UINT64_C(0xFFFFFFFF00000000);w[3]=two[3];at(UINT32_MAX,1,255,-255,w);
    }
    printf("Input motion probe: %u checks; literal oracle; native backend unexecuted.\n",checks);
    return 0;
}
