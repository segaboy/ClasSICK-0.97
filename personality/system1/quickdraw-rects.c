/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original fixed-port SPEC-0036 profile. */
#include "quickdraw-rects.h"

#define CS_QD_RECT_STRIDE 16u

static int32_t cs_qd_rect_s16(const uint8_t *p)
{
    uint32_t u = ((uint32_t)p[0] << 8) | (uint32_t)p[1];
    if (u >= UINT32_C(0x8000)) {
        return (int32_t)(u - UINT32_C(0x8000)) - INT32_C(0x8000);
    }
    return (int32_t)u;
}

static int32_t cs_qd_rect_clamp(int32_t v, int32_t hi)
{
    if (v < 0) {
        return 0;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

static void cs_qd_rect_bump(uint32_t *counter)
{
    if (*counter != UINT32_MAX) {
        *counter += 1u;
    }
}

int cs_qd_rect(cs_qd_rect_port *port, const uint8_t rect[8], uint16_t word)
{
    int32_t top, left, bottom, right;
    uint32_t x0, x1, y0, y1, y;
    uint32_t b0, b1, bx;
    uint8_t first_mask, last_mask;
    int paint;

    if (port == 0 || rect == 0) {
        return 0;
    }
    if (word == CS_QD_PAINT_RECT) {
        paint = 1;
    } else if (word == CS_QD_ERASE_RECT) {
        paint = 0;
    } else {
        return 0;
    }

    top = cs_qd_rect_s16(&rect[0]);
    left = cs_qd_rect_s16(&rect[2]);
    bottom = cs_qd_rect_s16(&rect[4]);
    right = cs_qd_rect_s16(&rect[6]);

    top = cs_qd_rect_clamp(top, (int32_t)CS_QD_RECT_HEIGHT);
    bottom = cs_qd_rect_clamp(bottom, (int32_t)CS_QD_RECT_HEIGHT);
    left = cs_qd_rect_clamp(left, (int32_t)CS_QD_RECT_WIDTH);
    right = cs_qd_rect_clamp(right, (int32_t)CS_QD_RECT_WIDTH);

    if (top < bottom && left < right) {
        y0 = (uint32_t)top;
        y1 = (uint32_t)bottom;
        x0 = (uint32_t)left;
        x1 = (uint32_t)right;

        b0 = x0 >> 3;
        b1 = (x1 - 1u) >> 3;
        first_mask = (uint8_t)(0xFFu >> (x0 & 7u));
        last_mask = (uint8_t)((0xFFu << (7u - ((x1 - 1u) & 7u))) & 0xFFu);
        if (b0 == b1) {
            first_mask = (uint8_t)(first_mask & last_mask);
        }

        for (y = y0; y < y1; ++y) {
            uint8_t *row = &port->pixels[y * CS_QD_RECT_STRIDE];
            for (bx = b0; bx <= b1; ++bx) {
                uint8_t mask;
                if (bx == b0) {
                    mask = first_mask;
                } else if (bx == b1) {
                    mask = last_mask;
                } else {
                    mask = 0xFFu;
                }
                if (paint) {
                    row[bx] = (uint8_t)(row[bx] | mask);
                } else {
                    row[bx] = (uint8_t)(row[bx] & (uint8_t)~mask);
                }
            }
        }
    }

    if (paint) {
        cs_qd_rect_bump(&port->paints);
    } else {
        cs_qd_rect_bump(&port->erases);
    }
    return 1;
}
