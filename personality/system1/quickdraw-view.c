/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0041 native drawing view. */
#include "quickdraw-view.h"

#define CS_QD_VIEW_MIN INT32_C(-32768)
#define CS_QD_VIEW_MAX INT32_C(32767)

static int32_t cs_qd_view_s16(const uint8_t *p)
{
    uint32_t u = ((uint32_t)p[0] << 8) | (uint32_t)p[1];
    if (u >= UINT32_C(0x8000)) {
        return (int32_t)(u - UINT32_C(0x8000)) - INT32_C(0x8000);
    }
    return (int32_t)u;
}

static int32_t cs_qd_view_clamp(int32_t v, int32_t hi)
{
    if (v < 0) {
        return 0;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

static int cs_qd_view_span(int32_t v, int32_t lo, int32_t hi)
{
    return v >= lo && v <= hi;
}

static int cs_qd_view_valid(const cs_qd_view *view)
{
    const int32_t h = (int32_t)CS_QD_RECT_HEIGHT;
    const int32_t w = (int32_t)CS_QD_RECT_WIDTH;

    if (view == 0 || view->port == 0) {
        return 0;
    }
    return cs_qd_view_span(view->x, CS_QD_VIEW_MIN, CS_QD_VIEW_MAX) &&
           cs_qd_view_span(view->y, CS_QD_VIEW_MIN, CS_QD_VIEW_MAX) &&
           cs_qd_view_span(view->top, 0, h) &&
           cs_qd_view_span(view->bottom, 0, h) &&
           cs_qd_view_span(view->left, 0, w) &&
           cs_qd_view_span(view->right, 0, w);
}

/* v is already bounded to [0,128]. */
static void cs_qd_view_put(uint8_t *p, int32_t v)
{
    p[0] = (uint8_t)(((uint32_t)v >> 8) & 0xFFu);
    p[1] = (uint8_t)((uint32_t)v & 0xFFu);
}

int cs_qd_view_init(cs_qd_view *view, cs_qd_rect_port *port)
{
    if (view == 0 || port == 0) {
        return 0;
    }
    view->port = port;
    view->x = 0;
    view->y = 0;
    view->top = 0;
    view->left = 0;
    view->bottom = (int32_t)CS_QD_RECT_HEIGHT;
    view->right = (int32_t)CS_QD_RECT_WIDTH;
    return 1;
}

int cs_qd_view_offset(cs_qd_view *view, int32_t x, int32_t y)
{
    if (!cs_qd_view_valid(view) ||
        !cs_qd_view_span(x, CS_QD_VIEW_MIN, CS_QD_VIEW_MAX) ||
        !cs_qd_view_span(y, CS_QD_VIEW_MIN, CS_QD_VIEW_MAX)) {
        return 0;
    }
    view->x = x;
    view->y = y;
    return 1;
}

int cs_qd_view_clip(cs_qd_view *view, const uint8_t rect[8])
{
    int32_t top, left, bottom, right;

    if (!cs_qd_view_valid(view) || rect == 0) {
        return 0;
    }
    top = cs_qd_view_clamp(cs_qd_view_s16(&rect[0]), (int32_t)CS_QD_RECT_HEIGHT);
    left = cs_qd_view_clamp(cs_qd_view_s16(&rect[2]), (int32_t)CS_QD_RECT_WIDTH);
    bottom = cs_qd_view_clamp(cs_qd_view_s16(&rect[4]), (int32_t)CS_QD_RECT_HEIGHT);
    right = cs_qd_view_clamp(cs_qd_view_s16(&rect[6]), (int32_t)CS_QD_RECT_WIDTH);
    view->top = top;
    view->left = left;
    view->bottom = bottom;
    view->right = right;
    return 1;
}

int cs_qd_view_rect(cs_qd_view *view, const uint8_t rect[8], uint16_t word)
{
    int32_t top, left, bottom, right;
    uint8_t packed[8];

    if (!cs_qd_view_valid(view) || rect == 0) {
        return 0;
    }
    if (word != CS_QD_PAINT_RECT && word != CS_QD_ERASE_RECT) {
        return 0;
    }

    /* Each sum lies in [-65536,65534]; no overflow or narrowing. */
    top = cs_qd_view_s16(&rect[0]) + view->y;
    left = cs_qd_view_s16(&rect[2]) + view->x;
    bottom = cs_qd_view_s16(&rect[4]) + view->y;
    right = cs_qd_view_s16(&rect[6]) + view->x;

    /* Clip fields are already within physical bounds. */
    if (top < view->top) {
        top = view->top;
    }
    if (left < view->left) {
        left = view->left;
    }
    if (bottom > view->bottom) {
        bottom = view->bottom;
    }
    if (right > view->right) {
        right = view->right;
    }
    if (top >= bottom || left >= right) {
        top = 0;
        left = 0;
        bottom = 0;
        right = 0;
    }

    cs_qd_view_put(&packed[0], top);
    cs_qd_view_put(&packed[2], left);
    cs_qd_view_put(&packed[4], bottom);
    cs_qd_view_put(&packed[6], right);
    return cs_qd_rect(view->port, packed, word);
}
