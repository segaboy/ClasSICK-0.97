/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input-probe.h"

#if (CLASSICK_PC_RAW_PACKET_PROBE+0)==1 && (CLASSICK_PC_INPUT_PROBE_MOTION+0)!=1
#error CLASSICK_PC_RAW_PACKET_PROBE=1 requires CLASSICK_PC_INPUT_PROBE_MOTION=1
#endif

#if !CLASSICK_PC_INPUT_PROBE_SEQUENCE
static uint64_t pair(uint32_t high,uint32_t low)
{
    return ((uint64_t)high<<32)|(uint64_t)low;
}
#endif

#if CLASSICK_PC_INPUT_PROBE_SEQUENCE
int cs_pc_input_sequence_begin(cs_pc_input_sequence *r)
{
    if(!r) return 1;
    r->magic=CS_PC_INPUT_SEQUENCE_STARTED;
    r->primed=0; r->was_ready=0; r->count=0; r->gaps=0;
    r->auxiliary=0; r->errors=0; r->flags=0;
    return 0;
}

/* One observation per loop turn, before the next poll. A READY poll consumes at
 * most one controller byte, so an AUX-counter step of one between two READY
 * observations identifies last_byte as the next report-stream byte. The byte
 * that completes startup (seen while not yet READY) is never recorded. */
static int sequence_observe(cs_pc_input_sequence *r,const cs_pc_window_session *s)
{
    uint64_t v[CS_PC_INPUT_PROBE_WORDS];
    uint32_t ready,step,i;
    if(!r||!s||r->magic!=CS_PC_INPUT_SEQUENCE_STARTED||r->count>=CS_PC_INPUT_SEQUENCE_BYTES) return 0;
    ready=(s->bus.phase==CS_PS2_BUS_READY&&s->bus.result==CS_PS2_READY)?1u:0u;
    if(!r->primed){
        r->primed=1; r->auxiliary=s->bus.auxiliary; r->errors=s->bus.errors; r->was_ready=ready;
        return 0;
    }
    step=s->bus.auxiliary-r->auxiliary;
    if(r->was_ready&&ready&&step!=0u){
        if(step>1u) r->gaps=(step-1u>=255u||r->gaps>=255u-(step-1u))?255u:r->gaps+(step-1u);
        if(s->bus.errors!=r->errors) r->flags|=UINT64_C(1)<<r->count;
        r->bytes[r->count]=(uint8_t)(s->bus.last_byte&255u);
        ++r->count;
    }
    r->auxiliary=s->bus.auxiliary; r->errors=s->bus.errors; r->was_ready=ready;
    if(r->count<CS_PC_INPUT_SEQUENCE_BYTES) return 0;
    v[0]=CS_PC_INPUT_PROBE_SEQUENCE_MAGIC;
    v[1]=r->flags|((uint64_t)r->gaps<<48)|((uint64_t)r->count<<56);
    v[2]=0; v[3]=0; v[4]=0; v[5]=0; v[6]=0; v[7]=0;
    for(i=0;i<CS_PC_INPUT_SEQUENCE_BYTES;++i)
        v[2u+(i>>3)]|=(uint64_t)r->bytes[i]<<((i&7u)*8u);
    cs_pc_input_probe_halt(v);
}
#endif

int cs_pc_input_probe_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s)
{
#if CLASSICK_PC_INPUT_PROBE_SEQUENCE
    (void)trace;
    return sequence_observe((cs_pc_input_sequence *)context,s);
#else
    uint64_t v[CS_PC_INPUT_PROBE_WORDS];
    (void)context;
    (void)trace;
#if CLASSICK_PC_INPUT_PROBE_MOTION
    if(!s||s->delivery.kind!=CS_PS2_DELIVERY_PACKET) return 0;
#if !CLASSICK_PC_INPUT_PROBE_REPORT
    if(!s->delivery.packet.dx&&!s->delivery.packet.dy) return 0;
#endif
#if CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET
    if(s->bus.packets<=1) return 0;
#endif
#if CLASSICK_PC_RAW_PACKET_PROBE
#if CLASSICK_PC_INPUT_PROBE_REPORT
    v[0]=CS_PC_INPUT_PROBE_REPORT_MAGIC;
#else
    v[0]=UINT64_C(0x4353505241573031);
#endif
    v[1]=pair(s->bus.packets,s->bus.errors);
    v[2]=pair(s->delivery.packet.raw_header,
        ((s->delivery.packet.raw_x&255u)<<8)|(s->delivery.packet.raw_y&255u));
#else
    v[0]=CS_PC_INPUT_PROBE_MOTION_MAGIC;
    v[1]=pair(s->bus.packets,s->bus.errors);
    v[2]=pair(s->bus.decoder.header,
        (((uint32_t)s->bus.decoder.x&255u)<<8)|((uint32_t)s->bus.last_byte&255u));
#endif
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
#endif
}
