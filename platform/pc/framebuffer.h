/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_FRAMEBUFFER_H
#define CLASSICK_PC_FRAMEBUFFER_H
#include <stddef.h>
#include <stdint.h>
#include "../../core/graphics/surface.h"

/* SPEC-0009 native linear 32-bit framebuffer target. Pitch is pixels per scanline. */
typedef enum { CS_FB_OK=0, CS_FB_ARGUMENT=1, CS_FB_FORMAT=2, CS_FB_DIMENSION=3,
    CS_FB_OVERFLOW=4, CS_FB_STORAGE=5, CS_FB_SOURCE=6 } cs_fb_result;
enum { CS_FB_RGB_RESERVED=0, CS_FB_BGR_RESERVED=1,
    CS_FB_MAX_DIMENSION=8192, CS_FB_MAX_PITCH=16384 };
typedef struct {
    volatile unsigned char *base;
    size_t size;
    uint32_t width,height,pitch,format;
} cs_fb_target;

/* Base denotes size live writable bytes; validation cannot prove mapping or caching. */
cs_fb_result cs_fb_init(cs_fb_target *out,volatile unsigned char *base,size_t size,
    uint32_t width,uint32_t height,uint32_t pitch,uint32_t format);
/* Clipped RGBA8 source at signed (left,top); reserved byte zero; no framebuffer reads. */
cs_fb_result cs_fb_present(const cs_fb_target *target,const cs_surface *source,
    int32_t left,int32_t top);
#endif
