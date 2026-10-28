/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/input-probe.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#if (CLASSICK_PC_INPUT_PROBE_REPORT+0)!=1
#error build with RAW_PACKET_PROBE=1 MOTION=1 SKIP_FIRST_PACKET=1 REPORT=1
#endif
static jmp_buf done;
static uint64_t actual[8];
static cs_pc_window_session session,before;
static unsigned checks;
#define C(x) do {++checks;if(!(x)){printf("report probe FAIL:%d:%s\n",__LINE__,#x);return 1;}}while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{memcpy(actual,values,sizeof(actual));longjmp(done,1);}
/* Returns 1 when the observer captured (halted), else 0. */
static int run(const cs_pc_window_session *s)
{
    memset(actual,0,sizeof(actual));
    if(setjmp(done)==0){(void)cs_pc_input_probe_observe(0,0,s);return 0;}
    return 1;
}
int main(void)
{
    C(cs_pc_input_probe_observe(0,0,0)==0);
    C(!run(&session));
    session.delivery.kind=CS_PS2_DELIVERY_LOST;session.bus.packets=5;
    C(!run(&session));
    session.delivery.kind=CS_PS2_DELIVERY_NONE;
    C(!run(&session));
    session.delivery.kind=CS_PS2_DELIVERY_PACKET;
    session.bus.packets=0;C(!run(&session));
    session.bus.packets=1;C(!run(&session));
    session.delivery.packet.dx=3;session.delivery.packet.dy=-3;
    C(!run(&session));
    session.delivery.packet.dx=0;session.delivery.packet.dy=0;
    session.delivery.packet.raw_header=8;session.bus.errors=5;
    session.owner.pointer.x=1;session.owner.pointer.y=2;
    session.bus.packets=2;
    memcpy(&before,&session,sizeof before);
    C(run(&session));
    C(memcmp(&session,&before,sizeof(session))==0);
    C(actual[0]==UINT64_C(0x4353505250543031));
    C(actual[1]==UINT64_C(0x0000000200000005));
    C(actual[2]==UINT64_C(0x0000000800000000));
    C(actual[3]==0 && actual[4]==0);
    C(actual[5]==UINT64_C(0x0000000100000002));
    C(actual[6]==0 && actual[7]==0);
    session.delivery.packet.dx=-2;session.delivery.packet.dy=3;
    session.delivery.packet.buttons=5;
    session.delivery.packet.raw_header=0x1D;session.delivery.packet.raw_x=0xFE;
    session.delivery.packet.raw_y=3;
    session.bus.decoder.header=0x77;session.bus.decoder.x=0x66;session.bus.last_byte=0x55;
    session.bus.packets=7;session.bus.errors=1;
    session.owner.pointer.down=1;session.owner.pointer.suppress=1;
    session.owner.pointer.x=-1;session.owner.pointer.y=300;
    session.owner.window.x=-5;session.owner.window.y=6;
    session.owner.product.inputs=9;session.owner.window.dragging=1;
    memcpy(&before,&session,sizeof before);
    C(run(&session));
    C(memcmp(&session,&before,sizeof(session))==0);
    C(actual[0]==UINT64_C(0x4353505250543031));
    C(actual[1]==UINT64_C(0x0000000700000001));
    C(actual[2]==UINT64_C(0x0000001D0000FE03));
    C(actual[3]==UINT64_C(0xFFFFFFFE00000003));
    C(actual[4]==UINT64_C(0x0000000500000003));
    C(actual[5]==UINT64_C(0xFFFFFFFF0000012C));
    C(actual[6]==UINT64_C(0xFFFFFFFB00000006));
    C(actual[7]==UINT64_C(0x0000000900000001));
    session.bus.decoder.header=0;session.bus.decoder.x=0;session.bus.last_byte=0;
    C(run(&session));
    C(actual[2]==UINT64_C(0x0000001D0000FE03));
    printf("report probe:%u assertions\n",checks);return 0;
}
