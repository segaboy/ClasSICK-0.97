/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0041 native drawing view. */
#ifndef CLASSICK_SYSTEM1_QUICKDRAW_VIEW_H
#define CLASSICK_SYSTEM1_QUICKDRAW_VIEW_H
#include <stdint.h>
#include "quickdraw-rects.h"
typedef struct {
    cs_qd_rect_port *port;
    int32_t x,y,top,left,bottom,right;
} cs_qd_view;
/* Native view borrowing a caller-owned SPEC-0036 port from init until
 * reinit/end; never owns, frees, serializes or exposes it to guests.
 * physical=logical+x/y native translation; clip is physical, clamped to
 * 128x64, inverted/empty accepted. Objects live/aligned/disjoint/synchronized.
 * Null/corrupt/unknown word returns0 unchanged; otherwise draw delegates
 * exactly once to cs_qd_rect and propagates its result. Not a guest GrafPort.
 * No libc, allocation, mutable global or callback. */
int cs_qd_view_init(cs_qd_view *view,cs_qd_rect_port *port);
int cs_qd_view_offset(cs_qd_view *view,int32_t x,int32_t y);
int cs_qd_view_clip(cs_qd_view *view,const uint8_t rect[8]);
int cs_qd_view_rect(cs_qd_view *view,const uint8_t rect[8],uint16_t word);
#endif
