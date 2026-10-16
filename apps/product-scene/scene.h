/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PRODUCT_SCENE_H
#define CLASSICK_PRODUCT_SCENE_H
#include "../../core/graphics/surface.h"
enum { CS_PRODUCT_WIDTH=512, CS_PRODUCT_HEIGHT=342, CS_PRODUCT_MAX_DIMENSION=8192 };
typedef struct {
    uint32_t value,inputs,progress,phases,success;
    /* Optional1024-byte MSB mono image. NULL selects the original geometric demo. */
    const uint8_t *pixels;
} cs_product_state;
/* Full-frame coordinates with band_top translating band row0. Output may be
 * wider/taller than the frame; those outside pixels remain unchanged. */
int cs_product_scene(const cs_surface *band,int32_t width,int32_t height,
    int32_t band_top,const cs_product_state *state);
/* Full-frame descriptor. Only the translated80x20 input-counter region changes. */
int cs_product_scene_input(const cs_surface *screen,uint32_t inputs);
#endif
