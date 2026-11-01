/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Isolated numeric experiment; not a surface backend. */
#include "surface-numeric.h"
cs_surface_result cs_pc286_surface_numeric(int storage_present, int format, int32_t width,
    int32_t height, uint16_t stride, uint16_t storage_size)
{
    uint32_t row;
    uint16_t span;
    if (!storage_present) return CS_SURFACE_ERR_ARGUMENT;
    if (format != CS_SURFACE_MONO1_MSB && format != CS_SURFACE_RGBA8) return CS_SURFACE_ERR_FORMAT;
    if (width <= 0 || height <= 0) return CS_SURFACE_ERR_DIMENSION;
    if (format == CS_SURFACE_MONO1_MSB) {
        row = ((uint32_t)width >> 3) + (((uint32_t)width & 7u) != 0u);
    } else {
        if ((uint32_t)width > 16383u) return CS_SURFACE_ERR_OVERFLOW;
        row = (uint32_t)width << 2;
    }
    if (row > 65535u) return CS_SURFACE_ERR_OVERFLOW;
    if (stride < row) return CS_SURFACE_ERR_STRIDE;
    if ((uint32_t)height > (uint32_t)(32767u / stride)) return CS_SURFACE_ERR_OVERFLOW;
    span = (uint16_t)(stride * (uint16_t)height);
    if (storage_size < span) return CS_SURFACE_ERR_STORAGE;
    return CS_SURFACE_OK;
}
