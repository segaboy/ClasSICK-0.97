/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "probe.h"
#include "../../core/graphics/surface.h"
#include "../../core/memory/arena.h"
#include "../../core/input/input.h"
#include "../../core/time/clock.h"

int cs_core_link_probe(unsigned char *storage, size_t capacity)
{
    cs_arena arena;
    cs_arena_span rgba, mono, records, rejected;
    cs_surface color, bits;
    cs_input_queue queue;
    cs_input_event event;
    cs_input_record record;
    cs_clock clock;
    cs_time elapsed;
    int reached;
    if (storage==NULL || capacity!=CS_CORE_LINK_STORAGE) return 1;
    /* Volatile fixture writes keep this probe independent of compiler bulk helpers. */
    for (size_t i=0;i<capacity;++i) ((volatile unsigned char *)storage)[i]=0xA5;
    if (cs_arena_init(&arena,storage,capacity)!=CS_ARENA_OK
            || cs_arena_alloc(&arena,40,1,&rgba)!=CS_ARENA_OK
            || cs_arena_alloc(&arena,6,1,&mono)!=CS_ARENA_OK
            || cs_arena_alloc(&arena,2*sizeof(cs_input_record),_Alignof(cs_input_record),&records)!=CS_ARENA_OK) return 2;
    if (cs_surface_init(&color,rgba.data,rgba.size,4,2,20,CS_SURFACE_RGBA8)!=CS_SURFACE_OK
            || cs_surface_clear(&color,0x11223344u)!=CS_SURFACE_OK
            || cs_surface_fill(&color,(cs_rect){2,0,9,1},0xAABBCCDDu)!=CS_SURFACE_OK) return 3;
    /* Literal corner/clipping/padding predicates, independent of the fill loops. */
    if (rgba.data[0]!=0x11 || rgba.data[1]!=0x22 || rgba.data[2]!=0x33 || rgba.data[3]!=0x44
            || rgba.data[8]!=0xAA || rgba.data[11]!=0xDD || rgba.data[15]!=0xDD
            || rgba.data[16]!=0xA5 || rgba.data[19]!=0xA5
            || rgba.data[28]!=0x11 || rgba.data[39]!=0xA5) return 4;
    if (cs_surface_init(&bits,mono.data,mono.size,9,2,3,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK
            || cs_surface_clear(&bits,0)!=CS_SURFACE_OK
            || cs_surface_fill(&bits,(cs_rect){0,0,9,1},1)!=CS_SURFACE_OK
            || mono.data[0]!=0xFF || mono.data[1]!=0xA5 || mono.data[2]!=0xA5
            || mono.data[3]!=0 || mono.data[4]!=0x25 || mono.data[5]!=0xA5) return 5;
    if (cs_input_init(&queue,(cs_input_record *)records.data,2)!=CS_INPUT_OK) return 6;
    event.source=1; event.key=CS_KEY_SPACE; event.action=CS_KEY_PRESS; event.repeat=0;
    if (cs_input_push(&queue,&event)!=CS_INPUT_OK) return 7;
    event.source=2; event.key=CS_KEY_ESCAPE;
    if (cs_input_push(&queue,&event)!=CS_INPUT_OK
            || cs_input_push(&queue,&event)!=CS_INPUT_ERR_FULL
            || cs_input_pop(&queue,&record)!=CS_INPUT_OK || record.sequence!=1
            || record.event.source!=1 || record.event.key!=CS_KEY_SPACE
            || cs_input_pop(&queue,&record)!=CS_INPUT_OK || record.sequence!=2
            || record.event.source!=2 || record.event.key!=CS_KEY_ESCAPE
            || cs_input_pop(&queue,&record)!=CS_INPUT_ERR_EMPTY) return 8;
    if (cs_clock_init(&clock,(cs_time){7,900000000})!=CS_TIME_OK
            || cs_clock_advance(&clock,(cs_time){0,200000000})!=CS_TIME_OK
            || clock.last.seconds!=8 || clock.last.nanoseconds!=100000000
            || cs_time_elapsed((cs_time){7,900000000},clock.last,&elapsed)!=CS_TIME_OK
            || elapsed.seconds!=0 || elapsed.nanoseconds!=200000000
            || cs_time_reached(clock.last,(cs_time){8,100000000},&reached)!=CS_TIME_OK
            || reached!=1) return 9;
    rejected.data=storage; rejected.size=7;
    if (cs_arena_alloc(&arena,capacity,1,&rejected)!=CS_ARENA_ERR_EXHAUSTED
            || rejected.data!=storage || rejected.size!=7
            || cs_input_reset(&queue)!=CS_INPUT_OK || queue.count!=0
            || cs_arena_reset(&arena)!=CS_ARENA_OK || arena.used!=0) return 10;
    return 0;
}

/* Link-fixture entry only: no loader/firmware ABI, stack setup or hardware startup. */
int cs_link_entry(unsigned char *storage, size_t capacity)
{
    return cs_core_link_probe(storage,capacity);
}
