/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOW_SCENE_H
#define CLASSICK_WINDOW_SCENE_H
#include "../../core/graphics/cursor.h"
#include "../../core/ui/window.h"
#include "../product-scene/scene.h"

/* SPEC-0054 stateless compositor over one bound cursor/full-frame surface. */
typedef struct {
    cs_cursor_token token;
    cs_rect dirty;
    uint32_t stage;
    int32_t cause;
} cs_window_scene_effect;

enum { CS_WINDOW_SCENE_OK = 0, CS_WINDOW_SCENE_ERR_ARGUMENT = 1,
       CS_WINDOW_SCENE_ERR_STATE = 2, CS_WINDOW_SCENE_ERR_CURSOR = 3,
       CS_WINDOW_SCENE_ERR_PAINT = 4, CS_WINDOW_SCENE_ERR_EXHAUSTED = 5 };
enum { CS_WINDOW_SCENE_HIDDEN = 1, CS_WINDOW_SCENE_PAINTING = 2,
       CS_WINDOW_SCENE_PAINTED = 3, CS_WINDOW_SCENE_ADVANCED = 4,
       CS_WINDOW_SCENE_VISIBLE = 5 };

/* old_window NULL: full repaint from HIDDEN. Otherwise VISIBLE and old_window
 * is the last-rendered snapshot. Pre-effect refusals leave everything intact;
 * later failures publish the reached stage in *out and the owner must repair. */
int cs_window_scene_compose(cs_cursor_state *cursor, cs_cursor_token token,
    const cs_cursor_art *art, const cs_product_state *product,
    const cs_window *old_window, const cs_window *window, uint32_t old_inputs,
    int32_t x, int32_t y, cs_window_scene_effect *out);
#endif
