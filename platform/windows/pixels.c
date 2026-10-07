/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "pixels.h"

cs_win_result cs_win_pixels(const cs_surface *source, uint8_t *destination,
    size_t capacity, size_t stride)
{
    cs_surface checked;
    size_t span;
    if (source == NULL || destination == NULL) return CS_WIN_ERR_ARGUMENT;
    if (cs_surface_init(&checked, source->storage, source->storage_size,
            source->width, source->height, source->stride, source->format)
            != CS_SURFACE_OK) return CS_WIN_ERR_SURFACE;
    if (checked.width > CS_WIN_MAX_DIMENSION || checked.height > CS_WIN_MAX_DIMENSION)
        return CS_WIN_ERR_DIMENSION;
    if (stride < (size_t)checked.width * 4u) return CS_WIN_ERR_STRIDE;
    if (stride > SIZE_MAX / (size_t)checked.height) return CS_WIN_ERR_OVERFLOW;
    span = stride * (size_t)checked.height;
    if (span > (size_t)PTRDIFF_MAX) return CS_WIN_ERR_OVERFLOW;
    if (capacity < span) return CS_WIN_ERR_STORAGE;
    for (int32_t y = 0; y < checked.height; ++y) {
        const uint8_t *row = checked.storage + (size_t)y * checked.stride;
        uint8_t *out = destination + (size_t)y * stride;
        for (int32_t x = 0; x < checked.width; ++x) {
            size_t offset = (size_t)x * 4u;
            if (checked.format == CS_SURFACE_RGBA8) {
                out[offset] = row[offset + 2u];
                out[offset + 1u] = row[offset + 1u];
                out[offset + 2u] = row[offset];
            } else {
                uint8_t mask = (uint8_t)(0x80u >> ((unsigned)x % 8u));
                uint8_t value = (row[(size_t)x / 8u] & mask) != 0u ? 0u : 255u;
                out[offset] = value;
                out[offset + 1u] = value;
                out[offset + 2u] = value;
            }
            out[offset + 3u] = 0u;
        }
    }
    return CS_WIN_OK;
}

cs_win_result cs_win_viewport(int32_t width, int32_t height,
    int32_t client_width, int32_t client_height, cs_rect *out)
{
    cs_rect rect = {0, 0, 0, 0};
    int32_t scale = 1;
    int32_t drawn_width, drawn_height;
    if (out == NULL) return CS_WIN_ERR_ARGUMENT;
    if (width <= 0 || height <= 0 || width > CS_WIN_MAX_DIMENSION
            || height > CS_WIN_MAX_DIMENSION || client_width < 0 || client_height < 0)
        return CS_WIN_ERR_DIMENSION;
    if (client_width != 0 && client_height != 0) {
        int32_t horizontal = client_width / width;
        int32_t vertical = client_height / height;
        if (horizontal >= 1 && vertical >= 1)
            scale = horizontal < vertical ? horizontal : vertical;
        drawn_width = width * scale;
        drawn_height = height * scale;
        rect.left = (client_width - drawn_width) / 2;
        rect.top = (client_height - drawn_height) / 2;
        rect.right = rect.left + drawn_width;
        rect.bottom = rect.top + drawn_height;
    }
    *out = rect;
    return CS_WIN_OK;
}
