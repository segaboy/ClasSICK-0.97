/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input-timeline.h"

int cs_pc_input_timeline_begin(cs_pc_input_timeline *r)
{
    uint32_t i;
    if(!r) return 1;
    r->primed=0u; r->was_ready=0u; r->count=0u; r->gaps=0u;
    r->unreliable=0u; r->baseline=0u; r->auxiliary=0u; r->errors=0u; r->used=0u;
    for(i=0u;i<CS_PC_INPUT_TIMELINE_ENTRIES;++i) r->entries[i]=0u;
    r->magic=CS_PC_INPUT_TIMELINE_STARTED;
    return 0;
}

/* Values above 255 encode as 0xff and mark the record unreliable. */
static uint64_t cap8(uint32_t v,uint32_t *bad)
{
    if(v>255u){ *bad=1u; return 0xffu; }
    return (uint64_t)v;
}

/* Out-of-range coordinates encode as 0xffff, never truncated. */
static uint64_t coord(int64_t v,int64_t max)
{
    return (v<0 || v>max) ? UINT64_C(0xffff) : (uint64_t)v;
}

__attribute__((noreturn)) static void timeline_stop(const cs_pc_input_timeline *r,
    const cs_pc_window_session *s,uint32_t reason)
{
    uint64_t v[8];
    uint32_t i;
    v[0]=CS_PC_INPUT_TIMELINE_MAGIC;
    v[1]=(uint64_t)(r->count&0xffu)|((uint64_t)(r->gaps&0xffu)<<8)|
        ((uint64_t)(r->unreliable!=0u)<<16)|((uint64_t)(r->baseline!=0u)<<17)|
        ((uint64_t)(reason&0xffu)<<24)|
        (coord((int64_t)s->owner.pointer.x,511)<<32)|(coord((int64_t)s->owner.pointer.y,341)<<48);
    for(i=0u;i<CS_PC_INPUT_TIMELINE_ENTRIES;++i)
        v[2u+i]=i<r->count ? r->entries[i] : UINT64_C(0);
    cs_pc_input_probe_halt(v);
}

int cs_pc_input_timeline_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s)
{
    cs_pc_input_timeline *r=(cs_pc_input_timeline *)context;
    uint32_t ready, prior, aux, err, used;
    (void)trace;
    if(!r || r->magic!=CS_PC_INPUT_TIMELINE_STARTED || !s || s->magic!=CS_PWS_MAGIC)
        return 0;
    ready=(s->bus.phase==CS_PS2_BUS_READY && s->bus.result==CS_PS2_READY) ? 1u : 0u;
    prior=(r->primed && r->was_ready) ? 1u : 0u;
    aux=s->bus.auxiliary; err=s->bus.errors; used=s->bus.decoder.used;
    if(err==UINT32_MAX) r->unreliable=1u; /* saturation hides further errors */
    if(ready && !prior) r->baseline=1u; /* bytes before this point are not claimed */
    else if(ready && prior){
        if(aux<r->auxiliary || aux==UINT32_MAX) r->unreliable=1u;
        else if(aux>r->auxiliary){
            uint32_t step=aux-r->auxiliary, bad=0u;
            uint64_t e;
            if(step-1u>=255u-r->gaps) r->gaps=255u; else r->gaps+=step-1u;
            e=cap8(s->bus.last_byte,&bad)|(cap8(r->used,&bad)<<8)|
                (cap8(used,&bad)<<16)|(cap8(s->delivery.kind,&bad)<<24)|
                ((uint64_t)(err!=r->errors)<<32)|(cap8(s->phase,&bad)<<40)|
                (cap8((uint32_t)s->owner.phase,&bad)<<48)|
                (cap8((uint32_t)s->owner.stage,&bad)<<56);
            if(bad) r->unreliable=1u;
            if(r->count<CS_PC_INPUT_TIMELINE_ENTRIES) r->entries[r->count++]=e;
        }
    }
    r->primed=1u; r->was_ready=ready;
    r->auxiliary=aux; r->errors=err; r->used=used;
    if(r->count>=CS_PC_INPUT_TIMELINE_ENTRIES) timeline_stop(r,s,1u);
    /* Optional native wiring observes a failed poll once before returning. */
    if(s->phase==CS_PWS_FAULTED) timeline_stop(r,s,3u);
    if(ready && prior && s->drained>0u) timeline_stop(r,s,2u);
    return 0;
}
