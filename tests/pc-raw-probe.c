/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/input-probe.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
static jmp_buf done;
static uint64_t actual[8];
static cs_pc_window_session session,before;
static unsigned checks;
#define C(x) do {++checks;if(!(x)){printf("raw probe FAIL:%d:%s\n",__LINE__,#x);return 1;}}while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{memcpy(actual,values,sizeof(actual));longjmp(done,1);}
int main(void)
{
    C(sizeof(cs_ps2_mouse_packet)==24);
    C(cs_pc_input_probe_observe(0,0,0)==0);
    C(cs_pc_input_probe_observe(0,0,&session)==0);
    session.delivery.kind=CS_PS2_DELIVERY_PACKET;
    C(cs_pc_input_probe_observe(0,0,&session)==0);
    session.delivery.packet.dx=255;session.delivery.packet.dy=-255;session.delivery.packet.buttons=2;
    session.delivery.packet.raw_header=0x2A;session.delivery.packet.raw_x=255;session.delivery.packet.raw_y=1;
    session.bus.decoder.header=0;session.bus.decoder.x=0;session.bus.last_byte=0xEE;
    session.bus.packets=3;session.bus.errors=4;session.owner.pointer.x=511;session.owner.pointer.y=341;
    before=session;
    if(setjmp(done)==0){(void)cs_pc_input_probe_observe(0,0,&session);C(0);}
    C(memcmp(&session,&before,sizeof(session))==0);
    C(actual[0]==UINT64_C(0x4353505241573031));
    C(actual[1]==UINT64_C(0x0000000300000004));
    C(actual[2]==UINT64_C(0x0000002A0000FF01));
    C(actual[3]==UINT64_C(0x000000FFFFFFFF01));
    C(actual[4]==UINT64_C(0x0000000200000000));
    C(actual[5]==UINT64_C(0x000001FF00000155));
    C(actual[6]==0 && actual[7]==0);
    printf("raw probe:%u assertions\n",checks);return 0;
}
