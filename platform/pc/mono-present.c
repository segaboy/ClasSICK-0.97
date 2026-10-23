/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mono-present.h"

typedef struct { uintptr_t begin,end; } mono_span;

static int span(const volatile void *p,size_t n,mono_span *out)
{
    uintptr_t b=(uintptr_t)p;
    if(p==NULL || n==0 || n>UINTPTR_MAX-b) return 0;
    out->begin=b; out->end=b+n;
    return 1;
}

static int overlap(mono_span a,mono_span b)
{
    return a.begin<b.end && b.begin<a.end;
}

int cs_pc_mono_present(const cs_fb_target *target,const cs_surface *canvas,
    uint8_t *row,size_t row_capacity,uint32_t initial,cs_pc_mono_report *out)
{
    mono_span reads[3],writes[3];
    cs_surface source,line;
    cs_fb_target checked;
    size_t bytes;
    uint32_t width,height,ox,oy;
    if(!span(target,sizeof(*target),&reads[0])
            || !span(canvas,sizeof(*canvas),&reads[1])
            || !span(out,sizeof(*out),&writes[0]) || row==NULL || initial>1u)
        return CS_MONO_ARGUMENT;
    if(canvas->format!=CS_SURFACE_MONO1_MSB || canvas->width!=512
            || canvas->height!=342 || canvas->stride!=64u
            || !span(canvas->storage,canvas->storage_size,&reads[2])
            || cs_surface_init(&source,canvas->storage,canvas->storage_size,
                canvas->width,canvas->height,canvas->stride,canvas->format)!=CS_SURFACE_OK)
        return CS_MONO_SOURCE;
    if(cs_fb_init(&checked,target->base,target->size,target->width,target->height,
                target->pitch,target->format)!=CS_FB_OK
            || checked.width<512u || checked.height<342u
            || !span(checked.base,checked.size,&writes[1]))
        return CS_MONO_TARGET;
    bytes=(size_t)checked.width*4u;
    if(row_capacity<bytes || !span(row,row_capacity,&writes[2]))
        return CS_MONO_CAPACITY;
    for(unsigned i=0;i<3u;++i) {
        for(unsigned j=0;j<3u;++j)
            if(overlap(writes[i],reads[j])) return CS_MONO_ALIAS;
        for(unsigned j=0;j<i;++j)
            if(overlap(writes[i],writes[j])) return CS_MONO_ALIAS;
    }
    width=initial!=0u?checked.width:512u;
    height=initial!=0u?checked.height:342u;
    ox=(checked.width-512u)/2u; oy=(checked.height-342u)/2u;
    /* These dimensions/capacity have already been checked; local descriptor only. */
    if(cs_surface_init(&line,row,row_capacity,(int32_t)width,1,
                (size_t)width*4u,CS_SURFACE_RGBA8)!=CS_SURFACE_OK)
        return CS_MONO_CAPACITY;
    out->phase=CS_MONO_VALIDATED; out->rows=0; out->required=height; out->cause=0;
    for(uint32_t y=0;y<height;++y) {
        uint32_t sy=initial!=0u?y:oy+y;
        cs_fb_result result;
        for(uint32_t x=0;x<width;++x) {
            uint32_t sx=initial!=0u?x:ox+x;
            uint8_t value=255u;
            size_t at=(size_t)x*4u;
            if(sx>=ox && sx-ox<512u && sy>=oy && sy-oy<342u) {
                size_t p=(size_t)(sy-oy)*64u+(size_t)(sx-ox)/8u;
                uint32_t shift=7u-((sx-ox)%8u);
                if(((uint32_t)source.storage[p]>>shift & 1u)!=0u) value=0u;
            }
            row[at]=value; row[at+1u]=value; row[at+2u]=value; row[at+3u]=255u;
        }
        result=cs_fb_present(&checked,&line,initial!=0u?0:(int32_t)ox,(int32_t)sy);
        if(result!=CS_FB_OK) {
            out->phase=CS_MONO_FAILED; out->cause=(int32_t)result;
            return CS_MONO_PRESENT;
        }
        ++out->rows;
    }
    out->phase=CS_MONO_COMPLETE;
    return CS_MONO_OK;
}

/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Appended to mono-present.c; reuses mono_span, span() and overlap(). */

static int32_t mono_clamp(int32_t v,int32_t hi)
{
    return v<0?0:v>hi?hi:v;
}

int cs_pc_mono_present_rects(const cs_fb_target *target,const cs_surface *canvas,
    uint8_t *row,size_t row_capacity,const cs_rect *rects,size_t count,
    cs_pc_mono_report *out)
{
    mono_span reads[4],writes[3];
    cs_surface source,line;
    cs_fb_target checked;
    unsigned nreads=3u;
    uint32_t ox,oy,required=0u;
    if(!span(target,sizeof(*target),&reads[0])
            || !span(canvas,sizeof(*canvas),&reads[1])
            || !span(out,sizeof(*out),&writes[0]) || row==NULL || count>3u
            || (count!=0u && !span(rects,count*sizeof(*rects),&reads[3])))
        return CS_MONO_ARGUMENT;
    if(count!=0u) nreads=4u;
    for(size_t i=0;i<count;++i)
        if(rects[i].left>rects[i].right || rects[i].top>rects[i].bottom)
            return CS_MONO_ARGUMENT;
    if(canvas->format!=CS_SURFACE_MONO1_MSB || canvas->width!=512
            || canvas->height!=342 || canvas->stride!=64u
            || !span(canvas->storage,canvas->storage_size,&reads[2])
            || cs_surface_init(&source,canvas->storage,canvas->storage_size,
                canvas->width,canvas->height,canvas->stride,canvas->format)!=CS_SURFACE_OK)
        return CS_MONO_SOURCE;
    if(cs_fb_init(&checked,target->base,target->size,target->width,target->height,
                target->pitch,target->format)!=CS_FB_OK
            || checked.width<512u || checked.height<342u
            || !span(checked.base,checked.size,&writes[1]))
        return CS_MONO_TARGET;
    if(row_capacity<(size_t)checked.width*4u || !span(row,row_capacity,&writes[2]))
        return CS_MONO_CAPACITY;
    for(unsigned i=0;i<3u;++i) {
        for(unsigned j=0;j<nreads;++j)
            if(overlap(writes[i],reads[j])) return CS_MONO_ALIAS;
        for(unsigned j=0;j<i;++j)
            if(overlap(writes[i],writes[j])) return CS_MONO_ALIAS;
    }
    ox=(checked.width-512u)/2u; oy=(checked.height-342u)/2u;
    for(size_t i=0;i<count;++i) {
        int32_t l=mono_clamp(rects[i].left,512),r=mono_clamp(rects[i].right,512);
        int32_t t=mono_clamp(rects[i].top,342),b=mono_clamp(rects[i].bottom,342);
        if(l<r && t<b) required+=(uint32_t)(b-t);
    }
    out->phase=CS_MONO_VALIDATED; out->rows=0; out->required=required; out->cause=0;
    for(size_t i=0;i<count;++i) {
        int32_t l=mono_clamp(rects[i].left,512),r=mono_clamp(rects[i].right,512);
        int32_t t=mono_clamp(rects[i].top,342),b=mono_clamp(rects[i].bottom,342);
        uint32_t w;
        if(l>=r || t>=b) continue;
        w=(uint32_t)(r-l);
        /* w<=512<=target width; capacity already checked. Defensive only. */
        cs_surface_result local=cs_surface_init(&line,row,row_capacity,(int32_t)w,1,
                    (size_t)w*4u,CS_SURFACE_RGBA8);
        if(local!=CS_SURFACE_OK) {
            out->phase=CS_MONO_FAILED; out->cause=(int32_t)local;
            return CS_MONO_CAPACITY;
        }
        for(int32_t y=t;y<b;++y) {
            cs_fb_result result;
            for(uint32_t x=0;x<w;++x) {
                uint32_t sx=(uint32_t)l+x;
                size_t p=(size_t)y*64u+(size_t)sx/8u;
                uint8_t value=((uint32_t)source.storage[p]>>(7u-sx%8u)&1u)!=0u?0u:255u;
                size_t at=(size_t)x*4u;
                row[at]=value; row[at+1u]=value; row[at+2u]=value; row[at+3u]=255u;
            }
            result=cs_fb_present(&checked,&line,(int32_t)(ox+(uint32_t)l),
                (int32_t)(oy+(uint32_t)y));
            if(result!=CS_FB_OK) {
                out->phase=CS_MONO_FAILED; out->cause=(int32_t)result;
                return CS_MONO_PRESENT;
            }
            ++out->rows;
        }
    }
    out->phase=CS_MONO_COMPLETE;
    return CS_MONO_OK;
}
