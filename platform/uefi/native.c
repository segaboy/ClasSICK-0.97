/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "native.h"
#include "../../apps/boot-scene/scene.h"
#include "../pc/acpi.h"
#include "../pc/pmtimer.h"

static void put32(unsigned char *p,uint32_t value)
{
    for(unsigned i=0;i<4;++i) { p[i]=(unsigned char)value; value>>=8; }
}
static uint32_t record(unsigned char *trace,uint32_t result,const cs_uefi_handoff *handoff,
    const cs_boot_report *report)
{
    put32(trace,CS_NATIVE_TRACE_MAGIC); put32(trace+4,CS_NATIVE_TRACE_VERSION);
    put32(trace+8,result); put32(trace+12,handoff!=NULL?handoff->stage:0u);
    put32(trace+16,report!=NULL?report->bands:0u);
    put32(trace+20,report!=NULL?report->band_rows:0u);
    put32(trace+24,report!=NULL?report->rows:0u);
    put32(trace+28,report!=NULL?report->step:0u);
    return result;
}
uint32_t cs_native_present(const cs_uefi_handoff *handoff,uint32_t descriptors_ready,
    volatile unsigned char *framebuffer,void *arena,unsigned char *trace)
{
    cs_uefi_framebuffer_layout layout;
    cs_fb_target target;
    cs_arena core;
    cs_boot_scene scene;
    cs_boot_report report;
    if(trace==NULL) return CS_NATIVE_ARGUMENT;
    if(handoff==NULL || framebuffer==NULL || arena==NULL)
        return record(trace,CS_NATIVE_ARGUMENT,handoff,NULL);
    if(handoff->stage!=CS_UEFI_EXITED || handoff->firmware_status!=0)
        return record(trace,CS_NATIVE_NOT_EXITED,handoff,NULL);
    if(descriptors_ready!=1u) return record(trace,CS_NATIVE_NOT_READY,handoff,NULL);
    if(cs_uefi_check_framebuffer(&handoff->framebuffer,&layout)!=CS_UEFI_OK
            || handoff->framebuffer.size>(uint64_t)SIZE_MAX
            || cs_fb_init(&target,framebuffer,(size_t)handoff->framebuffer.size,
                handoff->framebuffer.width,handoff->framebuffer.height,
                handoff->framebuffer.pitch,handoff->framebuffer.format)!=CS_FB_OK)
        return record(trace,CS_NATIVE_TARGET,handoff,NULL);
    if(handoff->arena_size!=CS_LOADER_ARENA_BYTES
            || cs_arena_init(&core,arena,(size_t)handoff->arena_size)!=CS_ARENA_OK
            || cs_boot_scene_prepare(&core,&target,&scene)!=CS_BOOT_OK)
        return record(trace,CS_NATIVE_ARENA,handoff,NULL);
    if(cs_boot_scene_draw(&scene,0,&report)!=CS_BOOT_OK)
        return record(trace,CS_NATIVE_DRAW,handoff,&report);
    return record(trace,CS_NATIVE_PRESENTED,handoff,&report);
}

static uint64_t get64(const unsigned char *p)
{
    uint64_t value=0;
    for(unsigned i=8;i>0;--i) value=(value<<8)|p[i-1];
    return value;
}
int cs_native_read(void *context,uint64_t physical,unsigned char *out,size_t length)
{
    const cs_native_memory *m=context;
    const unsigned char *d;
    uint64_t end,base,stop;
    size_t count,i;
    if(m==NULL || out==NULL || m->map==NULL || length==0 || m->map_stride<40
            || physical>UINT64_MAX-length || m->window_physical>UINT64_MAX-m->window_size) return 0;
    end=physical+length;
    if(physical<m->window_physical || end>m->window_physical+m->window_size) return 0;
    count=m->map_size/m->map_stride;
    for(i=0;i<count;++i) {
        uint32_t kind;
        d=m->map+i*m->map_stride;
        kind=(uint32_t)d[0]|((uint32_t)d[1]<<8)|((uint32_t)d[2]<<16)|((uint32_t)d[3]<<24);
        base=get64(d+8); stop=base+(get64(d+24)<<12);
        if((kind==0 || kind==6 || kind==9 || kind==10) && physical>=base && end<=stop) break;
    }
    if(i==count) return 0;
    {
        const volatile unsigned char *in=(const volatile unsigned char *)
            (m->window_base+(uintptr_t)(physical-m->window_physical));
        for(size_t j=0;j<length;++j) out[j]=in[j];
    }
    return 1;
}
static void put64(unsigned char *p,uint64_t value)
{
    for(unsigned i=0;i<8;++i) { p[i]=(unsigned char)value; value>>=8; }
}
static uint32_t timer_record(unsigned char *trace,uint32_t result,uint32_t acpi,
    const cs_acpi_pm_timer *timer,uint32_t reads,uint64_t ticks,cs_time elapsed)
{
    put32(trace,CS_TIMER_TRACE_MAGIC); put32(trace+4,CS_TIMER_TRACE_VERSION);
    put32(trace+8,result); put32(trace+12,acpi);
    put32(trace+16,timer!=NULL?timer->source:0u); put32(trace+20,timer!=NULL?timer->port:0u);
    put32(trace+24,timer!=NULL?timer->bits:0u); put32(trace+28,reads);
    put64(trace+32,ticks); put32(trace+40,elapsed.seconds); put32(trace+44,elapsed.nanoseconds);
    return result;
}
uint32_t cs_native_timer_probe(const cs_uefi_handoff *handoff,uint32_t descriptors_ready,
    const cs_native_memory *memory,cs_native_port32 port,void *port_context,unsigned char *trace)
{
    cs_acpi_pm_timer timer;
    cs_acpi_result acpi;
    cs_pmtimer counter;
    cs_time zero,elapsed;
    size_t descriptors;
    uint32_t reads=0,delta;
    /* Field stores, not aggregate initializers: no O0 memset/memcpy helpers. */
    zero.seconds=0; zero.nanoseconds=0;
    elapsed.seconds=0; elapsed.nanoseconds=0;
    if(trace==NULL) return CS_TIMER_ARGUMENT;
    if(handoff==NULL || memory==NULL || port==NULL)
        return timer_record(trace,CS_TIMER_ARGUMENT,0,NULL,0,0,zero);
    if(handoff->stage!=CS_UEFI_EXITED || handoff->firmware_status!=0)
        return timer_record(trace,CS_TIMER_NOT_EXITED,0,NULL,0,0,zero);
    if(descriptors_ready!=1u) return timer_record(trace,CS_TIMER_NOT_READY,0,NULL,0,0,zero);
    if(handoff->rsdp==0) return timer_record(trace,CS_TIMER_NO_RSDP,0,NULL,0,0,zero);
    if(cs_uefi_check_map(memory->map,memory->map_size,memory->map_stride,memory->map_version,
            &descriptors)!=CS_UEFI_OK)
        return timer_record(trace,CS_TIMER_ACPI,CS_ACPI_ACCESS,NULL,0,0,zero);
    acpi=cs_acpi_find_pm_timer(cs_native_read,(void *)(uintptr_t)memory,handoff->rsdp,&timer);
    if(acpi==CS_ACPI_UNSUPPORTED) return timer_record(trace,CS_TIMER_UNSUPPORTED,acpi,NULL,0,0,zero);
    if(acpi!=CS_ACPI_OK) return timer_record(trace,CS_TIMER_ACPI,acpi,NULL,0,0,zero);
    if(cs_pmtimer_init(&counter,timer.bits,port(port_context,timer.port))!=CS_PMTIMER_OK)
        return timer_record(trace,CS_TIMER_VALUE,acpi,&timer,1,0,zero);
    reads=1;
    while(counter.ticks<CS_TIMER_TARGET_TICKS) {
        if(reads>=CS_TIMER_POLLS)
            return timer_record(trace,CS_TIMER_STALLED,acpi,&timer,reads,counter.ticks,zero);
        ++reads;
        if(cs_pmtimer_sample(&counter,port(port_context,timer.port),&delta)!=CS_PMTIMER_OK)
            return timer_record(trace,CS_TIMER_VALUE,acpi,&timer,reads,counter.ticks,zero);
    }
    (void)cs_pmtimer_time(counter.ticks,&elapsed);
    return timer_record(trace,CS_TIMER_OK,acpi,&timer,reads,counter.ticks,elapsed);
}

static uint32_t loop_record(unsigned char *trace,uint32_t result,uint32_t seconds,cs_time elapsed,
    uint32_t frames,uint32_t late,uint64_t reads,uint32_t max_delta,uint32_t step)
{
    put32(trace,CS_LOOP_TRACE_MAGIC); put32(trace+4,CS_LOOP_TRACE_VERSION);
    put32(trace+8,result); put32(trace+12,seconds);
    put32(trace+16,elapsed.seconds); put32(trace+20,elapsed.nanoseconds);
    put32(trace+24,frames); put32(trace+28,late);
    put64(trace+32,reads); put32(trace+40,max_delta); put32(trace+44,step);
    return result;
}
uint32_t cs_native_progress_loop(const cs_uefi_handoff *handoff,uint32_t descriptors_ready,
    const cs_native_devices *devices,uint32_t seconds,unsigned char *trace)
{
    cs_acpi_pm_timer timer;
    cs_pmtimer counter;
    cs_uefi_framebuffer_layout layout;
    cs_fb_target target;
    cs_arena core;
    cs_boot_scene scene;
    cs_boot_report report;
    cs_time zero,elapsed;
    size_t descriptors;
    uint64_t reads=0;
    uint32_t frames=0,late=0,max_delta=0,step=0,delta,still=0,half;
    zero.seconds=0; zero.nanoseconds=0; elapsed.seconds=0; elapsed.nanoseconds=0;
    if(trace==NULL) return CS_LOOP_ARGUMENT;
    if(handoff==NULL || devices==NULL || devices->memory==NULL || devices->port==NULL
            || devices->framebuffer==NULL || devices->arena==NULL
            || seconds==0 || seconds>CS_LOOP_MAX_SECONDS)
        return loop_record(trace,CS_LOOP_ARGUMENT,seconds,zero,0,0,0,0,0);
    if(handoff->stage!=CS_UEFI_EXITED || handoff->firmware_status!=0)
        return loop_record(trace,CS_LOOP_NOT_EXITED,seconds,zero,0,0,0,0,0);
    if(descriptors_ready!=1u) return loop_record(trace,CS_LOOP_NOT_READY,seconds,zero,0,0,0,0,0);
    if(handoff->rsdp==0 || cs_uefi_check_map(devices->memory->map,devices->memory->map_size,
                devices->memory->map_stride,devices->memory->map_version,&descriptors)!=CS_UEFI_OK
            || cs_acpi_find_pm_timer(cs_native_read,(void *)(uintptr_t)devices->memory,
                handoff->rsdp,&timer)!=CS_ACPI_OK)
        return loop_record(trace,CS_LOOP_TIMER,seconds,zero,0,0,0,0,0);
    if(cs_uefi_check_framebuffer(&handoff->framebuffer,&layout)!=CS_UEFI_OK
            || handoff->framebuffer.size>(uint64_t)SIZE_MAX
            || cs_fb_init(&target,devices->framebuffer,(size_t)handoff->framebuffer.size,
                handoff->framebuffer.width,handoff->framebuffer.height,
                handoff->framebuffer.pitch,handoff->framebuffer.format)!=CS_FB_OK)
        return loop_record(trace,CS_LOOP_TARGET,seconds,zero,0,0,0,0,0);
    if(handoff->arena_size!=CS_LOADER_ARENA_BYTES
            || cs_arena_init(&core,devices->arena,(size_t)handoff->arena_size)!=CS_ARENA_OK
            || cs_boot_scene_prepare(&core,&target,&scene)!=CS_BOOT_OK)
        return loop_record(trace,CS_LOOP_ARENA,seconds,zero,0,0,0,0,0);
    if(cs_boot_scene_draw(&scene,0,&report)!=CS_BOOT_OK)
        return loop_record(trace,CS_LOOP_DRAW,seconds,zero,0,0,0,0,0);
    frames=1;
    reads=1;
    if(cs_pmtimer_init(&counter,timer.bits,devices->port(devices->port_context,timer.port))!=CS_PMTIMER_OK)
        return loop_record(trace,CS_LOOP_VALUE,seconds,zero,frames,0,reads,0,0);
    half=counter.mask/2u;
    for(;;) {
        uint32_t want;
        ++reads;
        if(cs_pmtimer_sample(&counter,devices->port(devices->port_context,timer.port),&delta)
                !=CS_PMTIMER_OK || cs_pmtimer_time(counter.ticks,&elapsed)!=CS_PMTIMER_OK)
            return loop_record(trace,CS_LOOP_VALUE,seconds,elapsed,frames,late,reads,max_delta,step);
        if(delta>max_delta) max_delta=delta;
        /* A delta above half the period may hide a wrap: count it as a late sample. */
        if(delta>half) ++late;
        still=delta==0?still+1u:0u;
        if(still>=CS_LOOP_STALL_READS)
            return loop_record(trace,CS_LOOP_STALLED,seconds,elapsed,frames,late,reads,max_delta,step);
        if(elapsed.seconds>=seconds) break;
        /* Sixteen segments fill linearly over the requested duration. */
        want=(uint32_t)(((uint64_t)elapsed.seconds*16u)/seconds);
        if(want!=step) {
            step=want;
            if(cs_boot_scene_draw(&scene,step,&report)!=CS_BOOT_OK)
                return loop_record(trace,CS_LOOP_DRAW,seconds,elapsed,frames,late,reads,max_delta,step);
            ++frames;
        }
    }
    step=16;
    if(cs_boot_scene_draw(&scene,step,&report)!=CS_BOOT_OK)
        return loop_record(trace,CS_LOOP_DRAW,seconds,elapsed,frames,late,reads,max_delta,step);
    ++frames;
    return loop_record(trace,CS_LOOP_OK,seconds,elapsed,frames,late,reads,max_delta,step);
}
