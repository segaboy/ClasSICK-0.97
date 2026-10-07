/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOWS_PIXELS_H
#define CLASSICK_WINDOWS_PIXELS_H
#include "surface.h"

#define CS_WIN_MAX_DIMENSION 4096
typedef enum {
    CS_WIN_OK = 0, CS_WIN_ERR_ARGUMENT = 1, CS_WIN_ERR_SURFACE = 2,
    CS_WIN_ERR_DIMENSION = 3, CS_WIN_ERR_STRIDE = 4, CS_WIN_ERR_OVERFLOW = 5,
    CS_WIN_ERR_STORAGE = 6, CS_WIN_ERR_HOST = 7
} cs_win_result;

/* SPEC-0002: caller-owned, live, non-overlapping byte buffers. B,G,R,0 output. */
cs_win_result cs_win_pixels(const cs_surface *source, uint8_t *destination,
    size_t capacity, size_t stride);
cs_win_result cs_win_viewport(int32_t width, int32_t height,
    int32_t client_width, int32_t client_height, cs_rect *out);
#endif
