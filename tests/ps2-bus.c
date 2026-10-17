/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/ps2-bus.h"
#include "ps2-bus-fixture.h"
#include <stdio.h>
int main(void)
{
    bus_fixture f;bf_init(&f);cs_ps2_io io=bf_io(&f);
    cs_ps2_bus bus;cs_input_queue queue;cs_input_record storage[8];
    if(cs_input_init(&queue,storage,8)!=CS_INPUT_OK || cs_ps2_bus_begin(&bus)!=CS_PS2_ACTIVE)return 1;
    unsigned polls;
    for(polls=0;polls<200 && bus.result==CS_PS2_ACTIVE;++polls){
        unsigned reads=f.key.status_reads,data=f.key.data_reads,writes=f.key.writes+f.mouse_writes;
        (void)cs_ps2_bus_poll(&bus,&io,(uint64_t)polls*1000u,&queue);
        if(f.key.status_reads-reads!=1 || f.key.data_reads-data>1
                || f.key.writes+f.mouse_writes-writes>1
                || (f.key.data_reads!=data && f.key.writes+f.mouse_writes!=writes))return 2;
    }
    if(bus.result!=CS_PS2_READY || bus.phase!=CS_PS2_BUS_READY || !f.ready
            || f.key.violations || f.mouse_writes!=14 || bus.auxiliary!=6
            || bus.keyboard.port_before!=0xCF || bus.keyboard.port_after!=0xCF
            || (bus.configuration&0x73u)!=0x10 || bus.left.presses)return 3;
    puts("PS2 complete keyboard/AUX startup and per-turn I/O bounds: PASS");return 0;
}
