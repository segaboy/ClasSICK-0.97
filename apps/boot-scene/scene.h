/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_BOOT_SCENE_H
#define CLASSICK_BOOT_SCENE_H
#include <stddef.h>
#include <stdint.h>
#include "../../core/graphics/surface.h"
#include "../../core/memory/arena.h"
#include "../../platform/pc/framebuffer.h"

/* SPEC-0009 original geometric boot test scene; not a historical Macintosh image. */
typedef enum { CS_SCENE_OK=0, CS_SCENE_ARGUMENT=1, CS_SCENE_DIMENSION=2,
    CS_SCENE_FORMAT=3, CS_SCENE_SURFACE=4 } cs_scene_result;
typedef enum { CS_BOOT_OK=0, CS_BOOT_ARGUMENT=1, CS_BOOT_TARGET=2,
    CS_BOOT_ARENA=3, CS_BOOT_SCENE=4, CS_BOOT_PRESENT=5 } cs_boot_result;
enum { CS_SCENE_MAX_DIMENSION=8192, CS_SCENE_SEGMENTS=16,
    CS_BOOT_STAGING_LIMIT=65536 };
typedef struct {
    cs_fb_target target;
    unsigned char *staging;
    size_t staging_bytes;
    uint32_t band_rows;
} cs_boot_scene;
typedef struct { uint32_t result,bands,band_rows,rows,step; } cs_boot_report;

/* Band row r receives scene row band_top+r; pixels outside the scene are unchanged. */
cs_scene_result cs_scene_render(const cs_surface *band,int32_t width,int32_t height,
    int32_t band_top,uint32_t step);
/* Allocates at most 64 KiB of RGBA8 staging from a caller-initialized arena. */
cs_boot_result cs_boot_scene_prepare(cs_arena *arena,const cs_fb_target *target,
    cs_boot_scene *out);
/* Presents the complete frame top to bottom; report is always written when non-null. */
cs_boot_result cs_boot_scene_draw(const cs_boot_scene *scene,uint32_t step,
    cs_boot_report *report);
#endif
