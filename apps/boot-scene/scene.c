/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "scene.h"

static const uint32_t bar_colors[8]={0xFFFFFFFFu,0xFFFF00FFu,0x00FFFFFFu,0x00FF00FFu,
    0xFF00FFFFu,0xFF0000FFu,0x0000FFFFu,0x000000FFu};

/* Scene coordinates are bounded by 8192, so every sum below fits int32_t. */
static int paint(const cs_surface *band,int32_t band_top,int32_t left,int32_t top,
    int32_t right,int32_t bottom,uint32_t color)
{
    cs_rect rect;
    if(right<left || bottom<top) return 1;
    rect.left=left; rect.right=right;
    rect.top=top-band_top; rect.bottom=bottom-band_top;
    return cs_surface_fill(band,rect,color)==CS_SURFACE_OK;
}

cs_scene_result cs_scene_render(const cs_surface *band,int32_t width,int32_t height,
    int32_t band_top,uint32_t step)
{
    cs_surface checked;
    int32_t u,m,pw,ph,px,py,ix,iy,iw,ih,tx,ty,tw,tb,lit;
    int ok=1;
    if(band==NULL) return CS_SCENE_ARGUMENT;
    if(width<1 || height<1 || width>CS_SCENE_MAX_DIMENSION || height>CS_SCENE_MAX_DIMENSION
            || band_top<0 || band_top>=height) return CS_SCENE_DIMENSION;
    if(cs_surface_init(&checked,band->storage,band->storage_size,band->width,band->height,
            band->stride,band->format)!=CS_SURFACE_OK) return CS_SCENE_SURFACE;
    if(checked.format!=CS_SURFACE_RGBA8) return CS_SCENE_FORMAT;
    u=(width<height?width:height)/32;
    if(u<1) u=1;
    m=2*u;
    ok&=paint(&checked,band_top,0,0,width,height,0x1E3A4CFFu);
    ok&=paint(&checked,band_top,0,0,m,m,0xFF0000FFu);
    ok&=paint(&checked,band_top,width-m,0,width,m,0x00FF00FFu);
    ok&=paint(&checked,band_top,0,height-m,m,height,0x0000FFFFu);
    ok&=paint(&checked,band_top,width-m,height-m,width,height,0xFFFFFFFFu);
    pw=width/2; ph=height/3; px=(width-pw)/2; py=(height-ph)/2;
    ok&=paint(&checked,band_top,px,py,px+pw,py+ph,0xF2F2F2FFu);
    ix=px+u; iy=py+u; iw=pw-2*u; ih=ph-2*u;
    if(iw>0 && ih>0) {
        ok&=paint(&checked,band_top,ix,iy,ix+iw,iy+ih,0x2B2B2BFFu);
        for(int32_t i=0;i<8;++i)
            ok&=paint(&checked,band_top,ix+iw*i/8,iy,ix+iw*(i+1)/8,iy+ih/2,bar_colors[i]);
        tx=ix+u; tw=ix+iw-u-tx; ty=iy+ih/2+u; tb=iy+ih-u;
        ok&=paint(&checked,band_top,tx,ty,tx+tw,tb,0x505050FFu);
        lit=(int32_t)(step%17u);
        if(tw>0) {
            for(int32_t i=0;i<CS_SCENE_SEGMENTS;++i)
                ok&=paint(&checked,band_top,tx+tw*i/16,ty,tx+tw*(i+1)/16-u/2,tb,
                    i<lit?0x7FD35FFFu:0x3A3A3AFFu);
        }
    }
    return ok?CS_SCENE_OK:CS_SCENE_SURFACE;
}

cs_boot_result cs_boot_scene_prepare(cs_arena *arena,const cs_fb_target *target,
    cs_boot_scene *out)
{
    cs_fb_target checked;
    cs_arena_span span;
    size_t row,rows;
    if(arena==NULL || target==NULL || out==NULL) return CS_BOOT_ARGUMENT;
    if(cs_fb_init(&checked,target->base,target->size,target->width,target->height,
            target->pitch,target->format)!=CS_FB_OK) return CS_BOOT_TARGET;
    row=(size_t)checked.width*4u;
    rows=CS_BOOT_STAGING_LIMIT/row;
    if(rows>(size_t)checked.height) rows=(size_t)checked.height;
    if(cs_arena_alloc(arena,rows*row,
            CS_ARENA_MAX_ALIGNMENT<16u?CS_ARENA_MAX_ALIGNMENT:16u,&span)!=CS_ARENA_OK) return CS_BOOT_ARENA;
    /* Field copies keep every optimization level free of memcpy helpers. */
    out->target.base=checked.base; out->target.size=checked.size;
    out->target.width=checked.width; out->target.height=checked.height;
    out->target.pitch=checked.pitch; out->target.format=checked.format;
    out->staging=span.data;
    out->staging_bytes=span.size;
    out->band_rows=(uint32_t)rows;
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
        if(cs_scene_render(&band,(int32_t)target.width,(int32_t)target.height,
                (int32_t)top,step)!=CS_SCENE_OK) return report_result(report,CS_BOOT_SCENE);
        if(cs_fb_present(&target,&band,0,(int32_t)top)!=CS_FB_OK)
            return report_result(report,CS_BOOT_PRESENT);
        if(report!=NULL) { ++report->bands; report->rows+=rows; }
    }
    return report_result(report,CS_BOOT_OK);
}
