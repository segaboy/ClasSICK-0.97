/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOWS_WINDOW_PREVIEW_H
#define CLASSICK_WINDOWS_WINDOW_PREVIEW_H
#include "../../apps/window-scene/input.h"
#include "presenter.h"
enum { CS_WIN_WINDOW_WIDTH=640,CS_WIN_WINDOW_HEIGHT=480,
       CS_WIN_WINDOW_CANVAS_X=64,CS_WIN_WINDOW_CANVAS_Y=69 };
typedef struct {
    cs_window_input input;
    cs_cursor_state cursor;
    cs_surface surface;
    cs_cursor_bits bits;
    cs_cursor_art art;
    uint8_t payload[CS_CURSOR_MONO_PAYLOAD];
    uint8_t *frame,*scratch;
    HWND window;
    unsigned paints;
    int failed,releasing,live;
} cs_win_window;
/* Fresh/quiescent object;initial buttons0/LEFT. Existing live session may not reopen. */
int cs_win_window_open(cs_win_window*,int visible,uint32_t initial_buttons);
/* Quiescent cleanup;idempotent after open/refusal;WM_DESTROY queues WM_QUIT. */
void cs_win_window_close(cs_win_window*);
#endif
