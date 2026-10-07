/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_GRAPHICS_SURFACE_H
#define CLASSICK_CORE_GRAPHICS_SURFACE_H

#include <stddef.h>
#include <stdint.h>

/* Native descriptors, never guest records. See SPEC-0001 v1 for preconditions. */
typedef enum {
    CS_SURFACE_MONO1_MSB = 1,
    CS_SURFACE_RGBA8 = 2
} cs_surface_format;

typedef enum {
    CS_SURFACE_OK = 0,
    CS_SURFACE_ERR_ARGUMENT = 1,
    CS_SURFACE_ERR_FORMAT = 2,
    CS_SURFACE_ERR_DIMENSION = 3,
    CS_SURFACE_ERR_STRIDE = 4,
    CS_SURFACE_ERR_OVERFLOW = 5,
    CS_SURFACE_ERR_STORAGE = 6,
    CS_SURFACE_ERR_RECT = 7,
    CS_SURFACE_ERR_COLOR = 8
} cs_surface_result;

typedef struct {
    uint8_t *storage;
    size_t storage_size;
    int32_t width;
    int32_t height;
    size_t stride;
    cs_surface_format format;
} cs_surface;

typedef struct {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} cs_rect;

/* Full stride*height storage required. No pixel initialization or ownership transfer. */
cs_surface_result cs_surface_init(cs_surface *out, uint8_t *storage,
    size_t storage_size, int32_t width, int32_t height, size_t stride,
    cs_surface_format format);

/* Half-open rectangle, clipped to the surface. Mono value 0/1; RGBA 0xRRGGBBAA. */
cs_surface_result cs_surface_fill(const cs_surface *surface, cs_rect rect,
    uint32_t value);

/* Clear logical pixels only; padding bits/bytes and trailing storage survive. */
cs_surface_result cs_surface_clear(const cs_surface *surface, uint32_t value);

#endif
