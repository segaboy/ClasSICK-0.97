/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "keyboard.h"
#include "../../apps/boot-scene/scene.h"
#include "../pc/acpi.h"
#include "../pc/pmtimer.h"
typedef struct {
    cs_ps2 keyboard;
    cs_time elapsed;
    uint64_t reads;
    uint64_t ticks;
    cs_keyboard_observer observer;
    void *context;
    uint32_t seconds,frames,space,escape,consumed,late,max_delta,step,key,action,sequence,notice;
} diagnostics;
static void increment(uint32_t *v) { if(*v!=UINT32_MAX) ++*v; }
static void put(unsigned char *p,uint64_t v,unsigned n)
{
    for(unsigned i=0;i<n;++i) { p[i]=(unsigned char)v; v>>=8; }
}
static uint32_t record(unsigned char *trace,uint32_t result,const diagnostics *d)
{
    put(trace,CS_KBD_TRACE_MAGIC,4); put(trace+4,1,4); put(trace+8,result,4);
    put(trace+12,d->keyboard.result,4); put(trace+16,d->keyboard.phase,4);
    put(trace+20,d->seconds,4); put(trace+24,d->elapsed.seconds,4); put(trace+28,d->elapsed.nanoseconds,4);
    put(trace+32,d->frames,4); put(trace+36,d->space,4); put(trace+40,d->escape,4);
    put(trace+44,d->consumed,4); put(trace+48,d->keyboard.dropped,4); put(trace+52,d->keyboard.errors,4);
    put(trace+56,d->keyboard.auxiliary,4); put(trace+60,d->keyboard.drained,4);
    put(trace+64,d->keyboard.resends,4); put(trace+68,d->late,4); put(trace+72,d->max_delta,4);
    put(trace+76,d->step,4); put(trace+80,d->keyboard.held,4); put(trace+84,d->key,4);
    put(trace+88,d->action,4); put(trace+92,d->sequence,4); put(trace+96,d->reads,8);
    put(trace+104,d->keyboard.last_byte,4); put(trace+108,d->keyboard.configuration,4);
    if(d->observer!=NULL) d->observer(d->context,d->ticks,trace,result!=CS_KBD_RUNNING?1u:d->notice);
    return result;
}
static int draw(cs_boot_scene *scene,diagnostics *d,uint32_t step)
{
    cs_boot_report report;
    if(cs_boot_scene_draw(scene,step,&report)!=CS_BOOT_OK) return 0;
    d->step=step; increment(&d->frames); return 1;
}
uint32_t cs_native_keyboard_observed(const cs_uefi_handoff *h,uint32_t ready,
    const cs_native_devices *dev,const cs_ps2_io *keyboard,uint32_t seconds,unsigned char *trace,
    cs_keyboard_observer observer,void *context)
{
    diagnostics d;
    cs_acpi_pm_timer timer;
    cs_pmtimer counter;
    cs_uefi_framebuffer_layout layout;
    cs_fb_target target;
    cs_arena core;
    cs_boot_scene scene;
    cs_input_queue queue;
    cs_input_record event;
    cs_arena_span storage;
    size_t descriptors;
    uint32_t present=0,delta,still=0,offset=0,step=0,active=0;
    uint64_t start=0;
    (void)cs_ps2_begin(&d.keyboard);
    d.elapsed.seconds=0; d.elapsed.nanoseconds=0; d.reads=0; d.seconds=seconds;
    d.ticks=0; d.observer=observer; d.context=context;
    d.notice=2;
    d.frames=0; d.space=0; d.escape=0; d.consumed=0; d.late=0; d.max_delta=0;
    d.step=0; d.key=0; d.action=0; d.sequence=0;
    if(trace==NULL) return CS_KBD_ARGUMENT;
    if(h==NULL || dev==NULL || dev->memory==NULL || dev->port==NULL
            || dev->framebuffer==NULL || dev->arena==NULL || keyboard==NULL
            || keyboard->read==NULL || keyboard->write==NULL
            || seconds==0 || seconds>CS_LOOP_MAX_SECONDS)
        return record(trace,CS_KBD_ARGUMENT,&d);
    if(h->stage!=CS_UEFI_EXITED || h->firmware_status!=0) return record(trace,CS_KBD_NOT_EXITED,&d);
    if(ready!=1) return record(trace,CS_KBD_NOT_READY,&d);
    if(h->rsdp==0 || cs_uefi_check_map(dev->memory->map,dev->memory->map_size,
                dev->memory->map_stride,dev->memory->map_version,&descriptors)!=CS_UEFI_OK
            || cs_acpi_find_pm_timer(cs_native_read,(void *)(uintptr_t)dev->memory,h->rsdp,&timer)!=CS_ACPI_OK)
        return record(trace,CS_KBD_TIMER,&d);
    if(cs_acpi_8042(cs_native_read,(void *)(uintptr_t)dev->memory,timer.fadt,&present)!=CS_ACPI_OK || present!=1)
        return record(trace,CS_KBD_NO_8042,&d);
    if(cs_uefi_check_framebuffer(&h->framebuffer,&layout)!=CS_UEFI_OK
            || h->framebuffer.size>(uint64_t)SIZE_MAX
            || cs_fb_init(&target,dev->framebuffer,(size_t)h->framebuffer.size,h->framebuffer.width,
                h->framebuffer.height,h->framebuffer.pitch,h->framebuffer.format)!=CS_FB_OK)
        return record(trace,CS_KBD_TARGET,&d);
    if(h->arena_size!=CS_LOADER_ARENA_BYTES || cs_arena_init(&core,dev->arena,(size_t)h->arena_size)!=CS_ARENA_OK
            || cs_boot_scene_prepare(&core,&target,&scene)!=CS_BOOT_OK
            || cs_arena_alloc(&core,8u*sizeof(cs_input_record),_Alignof(cs_input_record),&storage)!=CS_ARENA_OK
            || cs_input_init(&queue,(cs_input_record *)(void *)storage.data,8)!=CS_INPUT_OK)
        return record(trace,CS_KBD_ARENA,&d);
    d.reads=1;
    if(cs_pmtimer_init(&counter,timer.bits,dev->port(dev->port_context,timer.port))!=CS_PMTIMER_OK)
        return record(trace,CS_KBD_VALUE,&d);
    for(;;) {
        uint32_t result,want;
        ++d.reads;
        if(cs_pmtimer_sample(&counter,dev->port(dev->port_context,timer.port),&delta)!=CS_PMTIMER_OK)
            return record(trace,CS_KBD_VALUE,&d);
        d.ticks=counter.ticks;
        if(observer!=NULL) {
            d.notice=0;
            (void)record(trace,CS_KBD_RUNNING,&d);
            d.notice=2;
        }
        if(active!=0 && cs_pmtimer_time(counter.ticks-start,&d.elapsed)!=CS_PMTIMER_OK)
            return record(trace,CS_KBD_VALUE,&d);
        if(delta>d.max_delta) d.max_delta=delta;
        if(delta>counter.mask/2u) increment(&d.late);
        still=delta==0?still+1u:0u;
        if(still>=CS_LOOP_STALL_READS) return record(trace,CS_KBD_STALLED,&d);
        result=cs_ps2_poll(&d.keyboard,keyboard,counter.ticks,&queue);
        if(result>CS_PS2_READY) return record(trace,CS_KBD_KEYBOARD,&d);
        if(active==0) {
            if(result!=CS_PS2_READY) continue;
            active=1; start=counter.ticks;
            if(!draw(&scene,&d,0)) return record(trace,CS_KBD_DRAW,&d);
            (void)record(trace,CS_KBD_RUNNING,&d);
            continue;
        }
        step=d.elapsed.seconds>=seconds?16u:d.elapsed.seconds*16u/seconds;
        for(unsigned i=0;i<8;++i) {
            cs_input_result popped=cs_input_pop(&queue,&event);
            if(popped==CS_INPUT_ERR_EMPTY) break;
            if(popped!=CS_INPUT_OK) { d.keyboard.result=CS_PS2_QUEUE; return record(trace,CS_KBD_KEYBOARD,&d); }
            increment(&d.consumed); d.key=event.event.key; d.action=event.event.action; d.sequence=event.sequence;
            if(event.event.action==CS_KEY_PRESS && event.event.repeat==0) {
                if(event.event.key==CS_KEY_SPACE) { increment(&d.space); offset=(offset+1u)%17u; }
                else if(event.event.key==CS_KEY_ESCAPE) {
                    increment(&d.escape);
                    if(!draw(&scene,&d,(step+offset+1u)%17u)) return record(trace,CS_KBD_DRAW,&d);
                    return record(trace,CS_KBD_ESCAPED,&d);
                }
            }
        }
        want=(step+offset)%17u;
        if(want!=d.step) {
            if(!draw(&scene,&d,want)) return record(trace,CS_KBD_DRAW,&d);
            (void)record(trace,CS_KBD_RUNNING,&d);
        }
        if(d.elapsed.seconds>=seconds) return record(trace,CS_KBD_OK,&d);
    }
}
uint32_t cs_native_keyboard_loop(const cs_uefi_handoff *h,uint32_t ready,
    const cs_native_devices *dev,const cs_ps2_io *keyboard,uint32_t seconds,unsigned char *trace)
{
    return cs_native_keyboard_observed(h,ready,dev,keyboard,seconds,trace,NULL,NULL);
}
