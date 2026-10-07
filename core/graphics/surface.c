/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "surface.h"

static cs_surface_result validate(uint8_t *storage, size_t storage_size,
    int32_t width, int32_t height, size_t stride, cs_surface_format format)
{
    uintmax_t row_bytes;
    size_t span;

    if (storage == NULL) {
        return CS_SURFACE_ERR_ARGUMENT;
    }
    if (format != CS_SURFACE_MONO1_MSB && format != CS_SURFACE_RGBA8) {
        return CS_SURFACE_ERR_FORMAT;
    }
    if (width <= 0 || height <= 0) {
        return CS_SURFACE_ERR_DIMENSION;
    }
    row_bytes = (uintmax_t)(uint32_t)width;
    if (format == CS_SURFACE_MONO1_MSB) {
        row_bytes = row_bytes / 8u + (row_bytes % 8u != 0u ? 1u : 0u);
    } else {
        if (row_bytes > SIZE_MAX / 4u) {
            return CS_SURFACE_ERR_OVERFLOW;
        }
        row_bytes *= 4u;
    }
    if (row_bytes > SIZE_MAX) {
        return CS_SURFACE_ERR_OVERFLOW;
    }
    if (stride < (size_t)row_bytes) {
        return CS_SURFACE_ERR_STRIDE;
    }
    /* stride is nonzero here. Check before narrowing height or multiplying. */
    if ((uintmax_t)(uint32_t)height > SIZE_MAX / stride) {
        return CS_SURFACE_ERR_OVERFLOW;
    }
    span = stride * (size_t)height;
    if ((uintmax_t)span > (uintmax_t)PTRDIFF_MAX) {
        return CS_SURFACE_ERR_OVERFLOW;
    }
    if (storage_size < span) {
        return CS_SURFACE_ERR_STORAGE;
    }
    return CS_SURFACE_OK;
}

cs_surface_result cs_surface_init(cs_surface *out, uint8_t *storage,
    size_t storage_size, int32_t width, int32_t height, size_t stride,
    cs_surface_format format)
{
    cs_surface_result result;
    if (out == NULL) {
        return CS_SURFACE_ERR_ARGUMENT;
    }
    result = validate(storage, storage_size, width, height, stride, format);
    if (result != CS_SURFACE_OK) {
        return result;
    }
    out->storage = storage;
    out->storage_size = storage_size;
    out->width = width;
    out->height = height;
    out->stride = stride;
    out->format = format;
    return CS_SURFACE_OK;
}

cs_surface_result cs_surface_fill(const cs_surface *surface, cs_rect rect,
    uint32_t value)
{
    cs_surface_result result;
    int32_t x;
    int32_t y;
    if (surface == NULL) {
        return CS_SURFACE_ERR_ARGUMENT;
    }
    result = validate(surface->storage, surface->storage_size, surface->width,
        surface->height, surface->stride, surface->format);
    if (result != CS_SURFACE_OK) {
        return result;
    }
    if (rect.left > rect.right || rect.top > rect.bottom) {
        return CS_SURFACE_ERR_RECT;
    }
    if (surface->format == CS_SURFACE_MONO1_MSB && value > 1u) {
        return CS_SURFACE_ERR_COLOR;
    }
    if (rect.left < 0) { rect.left = 0; }
    if (rect.top < 0) { rect.top = 0; }
    if (rect.right > surface->width) { rect.right = surface->width; }
    if (rect.bottom > surface->height) { rect.bottom = surface->height; }
    if (rect.left >= rect.right || rect.top >= rect.bottom) {
        return CS_SURFACE_OK;
    }
    for (y = rect.top; y < rect.bottom; ++y) {
        uint8_t *row = surface->storage + (size_t)y * surface->stride;
        for (x = rect.left; x < rect.right; ++x) {
            if (surface->format == CS_SURFACE_MONO1_MSB) {
                size_t byte_index = (size_t)x / 8u;
                uint8_t mask = (uint8_t)(0x80u >> ((uint32_t)x % 8u));
                if (value != 0u) {
                    row[byte_index] = (uint8_t)(row[byte_index] | mask);
                } else {
                    row[byte_index] = (uint8_t)(row[byte_index] & (uint8_t)~mask);
                }
            } else {
                size_t offset = (size_t)x * 4u;
                row[offset] = (uint8_t)(value >> 24u);
                row[offset + 1u] = (uint8_t)(value >> 16u);
                row[offset + 2u] = (uint8_t)(value >> 8u);
                row[offset + 3u] = (uint8_t)value;
            }
        }
    }
    return CS_SURFACE_OK;
}

cs_surface_result cs_surface_clear(const cs_surface *surface, uint32_t value)
{
    cs_rect rect;
    if (surface == NULL) {
        return CS_SURFACE_ERR_ARGUMENT;
    }
    rect.left = 0;
    rect.top = 0;
    rect.right = surface->width;
    rect.bottom = surface->height;
    return cs_surface_fill(surface, rect, value);
}
