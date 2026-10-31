/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOW_INPUT_H
#define CLASSICK_WINDOW_INPUT_H
#include "scene.h"
#include "../../core/input/mouse.h"

enum { CS_WINDOW_CURSOR_SIZE = 15, CS_WINDOW_CURSOR_HOT = 7 };
/* Original 15x15 black plus with a white halo; hotspot 7,7.
 * Truthful caller-owned bits object;NULL leaves everything unchanged.
 * Header-local keeps fault-object/native import ABIs unchanged. */
static inline void cs_window_cursor_bits_init(cs_cursor_bits *bits)
{
    unsigned i, r;
    if (bits == NULL)
        return;
    for (i = 0u; i < 32u; i++)
        bits->mask[i] = bits->image[i] = 0u;
    /* Two MSB-first bytes per row; the sixteenth column remains clear. */
    for (r = 0u; r < 15u; r++) {
        bits->mask[2u * r] = (r >= 6u && r <= 8u) ? 0xFFu : 0x03u;
        bits->mask[2u * r + 1u] = (r >= 6u && r <= 8u) ? 0xFEu : 0x80u;
        bits->image[2u * r] = (r == 7u) ? 0xFFu : 0x01u;
        bits->image[2u * r + 1u] = (r == 7u) ? 0xFEu : 0u;
    }
}

typedef struct {
    cs_pointer pointer;
    cs_mouse_button counter;
    cs_window window,rendered_window;
    cs_product_state product;
    cs_cursor_state *cursor;
    const cs_cursor_art *art;
    cs_cursor_token token;
    cs_pointer_event last;
    cs_window_scene_effect render;
    uint32_t rendered_inputs,valid,phase,stage;
    int32_t cause;
} cs_window_input;
#define CS_WINDOW_INPUT_VALID 0x57494F31u
enum { CS_WINDOW_INPUT_OK=0,CS_WINDOW_INPUT_CHANGED=1,CS_WINDOW_INPUT_ARGUMENT=2,
       CS_WINDOW_INPUT_STATE=3,CS_WINDOW_INPUT_FAULT=4 };
enum { CS_WINDOW_INPUT_READY=1,CS_WINDOW_INPUT_FAULTED=2 };
enum { CS_WINDOW_INPUT_POINTER=1,CS_WINDOW_INPUT_COUNTER=2,
       CS_WINDOW_INPUT_WINDOW=3,CS_WINDOW_INPUT_RENDERED=4 };
/* Fresh/quiescent owner only;borrowed cursor is already bound HIDDEN. */
int cs_window_input_begin(cs_window_input*,cs_cursor_state*,cs_cursor_token,
    const cs_cursor_art*,const cs_window_config*,const cs_product_state*,uint32_t);
int cs_window_input_sample(cs_window_input*,const cs_pointer_sample*);
int cs_window_input_cancel(cs_window_input*);
/* Explicit fault recovery;current or newly rebound cursor token. */
int cs_window_input_repair(cs_window_input*,cs_cursor_token);
#endif
