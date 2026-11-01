/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_SURFACE_NUMERIC_H
#define CLASSICK_PC286_SURFACE_NUMERIC_H
#include "../../core/graphics/surface.h"

/* Isolated numeric model: a present flag is not proof of native storage. */
cs_surface_result cs_pc286_surface_numeric(int storage_present, int format,
    int32_t width, int32_t height, uint16_t stride, uint16_t storage_size);
#endif
