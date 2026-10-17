/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_PS2_BUS_H
#define CLASSICK_PC_PS2_BUS_H
#include "ps2.h"
#include "ps2-mouse.h"
#include "../../core/input/mouse.h"
/* SPEC-0040 strict dual-input profile. Results reuse CS_PS2_*. After
   cs_ps2_bus_begin the bus is the sole poller of its embedded keyboard. */
enum { CS_PS2_BUS_READY=24 };
typedef struct {
    cs_ps2 keyboard;
    cs_ps2_mouse decoder;
    cs_ps2_mouse_packet report;
    cs_mouse_button left;
    uint64_t since,last_tick;
    uint32_t phase,result,polls,retries,drained,packets,errors,auxiliary,resends,configuration,last_byte,started,synced;
} cs_ps2_bus;
uint32_t cs_ps2_bus_begin(cs_ps2_bus *bus);
uint32_t cs_ps2_bus_poll(cs_ps2_bus *bus,const cs_ps2_io *io,uint64_t now,cs_input_queue *queue);
#endif
