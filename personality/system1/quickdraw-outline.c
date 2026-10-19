/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0048 native outline. */
#include <stdint.h>
#include "quickdraw-outline.h"

/* Original engineering helper; not the historical FrameRect. Storage per
 * cs_qd_rect paint: black=1, stride16, byte(r*16+(c>>3)), 0x80>>(c&7). */

static int ol_range(int32_t v, int32_t lo, int32_t hi)
{
    return v >= lo && v <= hi;
}

static int ol_view_valid(const cs_qd_view *view)
{
    if (view == 0 || view->port == 0)
        return 0;
    if (!ol_range(view->x, -32768, 32767) || !ol_range(view->y, -32768, 32767))
        return 0;
    if (!ol_range(view->top, 0, 64) || !ol_range(view->bottom, 0, 64))
        return 0;
    if (!ol_range(view->left, 0, 128) || !ol_range(view->right, 0, 128))
        return 0;
    return 1;
}

static int32_t ol_be16(const uint8_t *p)
{
    uint32_t v = ((uint32_t)p[0] << 8) | (uint32_t)p[1];
    int32_t s = (int32_t)v;
    if ((v & 0x8000u) != 0u)
        s -= 65536;
    return s;
}

static int32_t ol_max(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

static int32_t ol_min(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

int cs_qd_view_outline(cs_qd_view *view, const uint8_t rect[8],
                       int32_t horizontal_width, int32_t vertical_height)
{
    cs_qd_rect_port *port;
    int32_t t, l, b, r;
    int32_t in_t, in_b, in_l, in_r;
    int32_t r0, r1, c0, c1, row, col;

    /* OL4 preflight: view, rect, pen ranges; no rect read or port write. */
    if (!ol_view_valid(view))
        return 0;
    if (rect == 0)
        return 0;
    if (!ol_range(horizontal_width, 0, 32767) ||
        !ol_range(vertical_height, 0, 32767))
        return 0;

    port = view->port;

    /* OL5 decode and translate; results in [-65536,65534]. */
    t = ol_be16(&rect[0]) + view->y;
    l = ol_be16(&rect[2]) + view->x;
    b = ol_be16(&rect[4]) + view->y;
    r = ol_be16(&rect[6]) + view->x;

    /* OL6/OL7: empty rect or zero pen draws nothing but still counts. */
    if (t < b && l < r && horizontal_width != 0 && vertical_height != 0) {
        /* OL8 inner hole bounds from the unclipped rect; a pixel inside
         * the rect is covered unless strictly inside the hole. */
        in_t = ol_min(t + vertical_height, b);
        in_b = ol_max(b - vertical_height, t);
        in_l = ol_min(l + horizontal_width, r);
        in_r = ol_max(r - horizontal_width, l);

        /* OL9 intersect with clip already clamped inside 128x64. */
        r0 = ol_max(t, view->top);
        r1 = ol_min(b, view->bottom);
        c0 = ol_max(l, view->left);
        c1 = ol_min(r, view->right);

        for (row = r0; row < r1; ++row) {
            int row_edge = row < in_t || row >= in_b;
            uint32_t base = (uint32_t)row << 4;
            for (col = c0; col < c1; ++col) {
                if (row_edge || col < in_l || col >= in_r) {
                    uint32_t uc = (uint32_t)col;
                    uint32_t idx = base + (uc >> 3);
                    uint8_t mask = (uint8_t)(0x80u >> (uc & 7u));
                    port->pixels[idx] = (uint8_t)(port->pixels[idx] | mask);
                }
            }
        }
    }

    /* OL10 single saturating paint count; erases and view untouched. */
    if (port->paints != UINT32_MAX)
        port->paints = port->paints + 1u;
    return 1;
}
