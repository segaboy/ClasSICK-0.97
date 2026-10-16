/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "scene.h"

#include "../native-drawing/workload.h"
#include "../service-probe/probe.h"
typedef struct {
    cs_trap_gateway gateway;
    cs_qd_rect_port port;
    cs_native_draw_result result;
} boot_workspace;
_Static_assert(sizeof(boot_workspace)+_Alignof(boot_workspace)-1u<=CS_BOOT_WORKSPACE_LIMIT,"Probe workspace budget");

cs_scene_result cs_scene_render(const cs_surface *band,int32_t width,int32_t height,
    int32_t band_top,uint32_t step)
{
    cs_surface checked;
    if(!band)return CS_SCENE_ARGUMENT;
    if(width<1 || height<1 || width>CS_SCENE_MAX_DIMENSION || height>CS_SCENE_MAX_DIMENSION ||
        band_top<0 || band_top>=height)return CS_SCENE_DIMENSION;
    if(cs_surface_init(&checked,band->storage,band->storage_size,band->width,band->height,
        band->stride,band->format)!=CS_SURFACE_OK)return CS_SCENE_SURFACE;
    if(checked.format!=CS_SURFACE_RGBA8)return CS_SCENE_FORMAT;
    cs_qd_rect_port port;cs_native_draw_result result;
    if(!cs_native_draw(&port,&result))return CS_SCENE_WORKLOAD;
    cs_product_state state;state.value=result.value;state.inputs=0;state.progress=step%17u;
    state.phases=result.phase;state.success=result.success;state.pixels=port.pixels;
    return cs_product_scene(&checked,width,height,band_top,&state)?CS_SCENE_OK:CS_SCENE_SURFACE;
}

cs_boot_result cs_boot_scene_prepare(cs_arena *arena,const cs_fb_target *target,
    cs_boot_scene *out)
{
    cs_fb_target checked;
    cs_arena_span span;
    size_t row,rows,staging,offset,alignment;
    if(arena==NULL || target==NULL || out==NULL) return CS_BOOT_ARGUMENT;
    if(cs_fb_init(&checked,target->base,target->size,target->width,target->height,
            target->pitch,target->format)!=CS_FB_OK) return CS_BOOT_TARGET;
    row=(size_t)checked.width*4u;
    rows=CS_BOOT_STAGING_LIMIT/row;
    if(rows>(size_t)checked.height) rows=(size_t)checked.height;
    staging=rows*row;alignment=_Alignof(boot_workspace);
    offset=staging+(alignment-staging%alignment)%alignment;
    if(cs_arena_alloc(arena,offset+sizeof(boot_workspace),
            CS_ARENA_MAX_ALIGNMENT<16u?CS_ARENA_MAX_ALIGNMENT:16u,&span)!=CS_ARENA_OK) return CS_BOOT_ARENA;
    boot_workspace *w=(boot_workspace *)(void *)(span.data+offset);
    unsigned char *bytes=(unsigned char *)(void *)w;
    for(size_t i=0;i<sizeof *w;++i)bytes[i]=0;
    cs_service_probe_result service;
    if(!cs_service_probe(&w->gateway,&service) ||
        !cs_native_draw(&w->port,&w->result))return CS_BOOT_PROBE;
    /* Field copies keep every optimization level free of memcpy helpers. */
    out->target.base=checked.base; out->target.size=checked.size;
    out->target.width=checked.width; out->target.height=checked.height;
    out->target.pitch=checked.pitch; out->target.format=checked.format;
    out->staging=span.data;
    out->staging_bytes=staging;
    out->band_rows=(uint32_t)rows;out->workspace_bytes=offset-staging+sizeof *w;
    out->state.value=w->result.value;out->state.inputs=0;out->state.progress=0;
    out->state.phases=w->result.phase;out->state.success=w->result.success;
    out->state.pixels=w->port.pixels;
    return CS_BOOT_OK;
}

static cs_boot_result report_result(cs_boot_report *report,cs_boot_result result)
{
    if(report!=NULL) report->result=(uint32_t)result;
    return result;
}

cs_boot_result cs_boot_scene_draw(const cs_boot_scene *scene,uint32_t step,
    cs_boot_report *report)
{
    return cs_boot_scene_draw_input(scene,step,0,report);
}
cs_boot_result cs_boot_scene_draw_input(const cs_boot_scene *scene,uint32_t step,
    uint32_t inputs,cs_boot_report *report)
{
    cs_fb_target target;
    uint32_t top;
    if(report!=NULL) {
        report->result=CS_BOOT_ARGUMENT; report->bands=0; report->band_rows=0;
        report->rows=0; report->step=step;
    }
    if(scene==NULL) return report_result(report,CS_BOOT_ARGUMENT);
    if(cs_fb_init(&target,scene->target.base,scene->target.size,scene->target.width,
            scene->target.height,scene->target.pitch,scene->target.format)!=CS_FB_OK)
        return report_result(report,CS_BOOT_TARGET);
    if(scene->staging==NULL || scene->band_rows==0 || scene->band_rows>target.height
            || (size_t)scene->band_rows>CS_BOOT_STAGING_LIMIT/((size_t)target.width*4u)
            || scene->staging_bytes<(size_t)scene->band_rows*(size_t)target.width*4u)
        return report_result(report,CS_BOOT_ARENA);
    if(report!=NULL) report->band_rows=scene->band_rows;
    for(top=0;top<target.height;top+=scene->band_rows) {
        cs_surface band;
        uint32_t rows=target.height-top;
        if(rows>scene->band_rows) rows=scene->band_rows;
        if(cs_surface_init(&band,scene->staging,scene->staging_bytes,(int32_t)target.width,
                (int32_t)rows,(size_t)target.width*4u,CS_SURFACE_RGBA8)!=CS_SURFACE_OK)
            return report_result(report,CS_BOOT_ARENA);
        cs_product_state state;
        state.value=scene->state.value;state.inputs=inputs;state.progress=step%17u;
        state.phases=scene->state.phases;state.success=scene->state.success;state.pixels=scene->state.pixels;
        if(!cs_product_scene(&band,(int32_t)target.width,(int32_t)target.height,
                (int32_t)top,&state))return report_result(report,CS_BOOT_SCENE);
        if(cs_fb_present(&target,&band,0,(int32_t)top)!=CS_FB_OK)
            return report_result(report,CS_BOOT_PRESENT);
        if(report!=NULL) { ++report->bands; report->rows+=rows; }
    }
    return report_result(report,CS_BOOT_OK);
}
