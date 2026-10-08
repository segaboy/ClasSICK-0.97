/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "native.h"
#include "../../apps/boot-scene/scene.h"

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
