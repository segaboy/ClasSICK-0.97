/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input-probe.h"

static uint64_t pair(uint32_t high,uint32_t low)
{
    return ((uint64_t)high<<32)|(uint64_t)low;
}

int cs_pc_input_probe_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s)
{
    uint64_t v[CS_PC_INPUT_PROBE_WORDS];
    (void)context;
    (void)trace;
#if CLASSICK_PC_INPUT_PROBE_MOTION
    if(!s||s->delivery.kind!=CS_PS2_DELIVERY_PACKET||
        (!s->delivery.packet.dx&&!s->delivery.packet.dy)) return 0;
    v[0]=CS_PC_INPUT_PROBE_MOTION_MAGIC;
    v[1]=pair(s->bus.packets,s->bus.errors);
    v[2]=pair(s->bus.decoder.header,
        (((uint32_t)s->bus.decoder.x&255u)<<8)|((uint32_t)s->bus.last_byte&255u));
    /* Intentional two's-complement bit encoding of signed values. */
    v[3]=pair((uint32_t)s->delivery.packet.dx,(uint32_t)s->delivery.packet.dy);
    v[4]=pair(s->delivery.packet.buttons,
        (uint32_t)s->owner.pointer.down|((uint32_t)s->owner.pointer.suppress<<1));
    v[5]=pair((uint32_t)s->owner.pointer.x,(uint32_t)s->owner.pointer.y);
    v[6]=pair((uint32_t)s->owner.window.x,(uint32_t)s->owner.window.y);
    v[7]=pair(s->owner.product.inputs,(uint32_t)s->owner.window.dragging);
    cs_pc_input_probe_halt(v);
#else
    if(!s||s->delivery.kind!=CS_PS2_DELIVERY_LOST) return 0;
    v[0]=CS_PC_INPUT_PROBE_MAGIC;
    v[1]=pair(s->delivery.kind,s->bus.phase);
    v[2]=pair(s->bus.errors,s->bus.packets);
    v[3]=pair(s->bus.auxiliary,s->bus.last_byte);
    v[4]=pair(s->bus.decoder.header,s->bus.decoder.used);
    v[5]=pair(s->owner.pointer.down,s->owner.pointer.suppress);
    v[6]=pair(s->owner.product.inputs,s->bus.decoder.x);
    /* Intentional two's-complement bit encoding of signed positions. */
    v[7]=pair((uint32_t)s->owner.pointer.x,(uint32_t)s->owner.pointer.y);
    cs_pc_input_probe_halt(v);
#endif
}
