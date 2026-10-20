/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "scene.h"

typedef struct { uintptr_t p; size_t n; } sp;

static sp mk(const void *p, size_t n) { sp s; s.p = (uintptr_t)p; s.n = n; return s; }
static int bad(sp a) { return a.p == 0u || a.p + a.n < a.p; }
static int hit(sp a, sp b) { return a.n && b.n && a.p < b.p + b.n && b.p < a.p + a.n; }
static int32_t mn(int32_t a, int32_t b) { return a < b ? a : b; }
static int32_t mx(int32_t a, int32_t b) { return a > b ? a : b; }

static int win_ok(const cs_window *w)
{
    if (w->valid != CS_WINDOW_VALID || w->width < 2 || w->width > 512 ||
        w->height < 2 || w->height > 342 || w->title < 1 || w->title >= w->height ||
        w->x < 0 || w->x > 512-w->width || w->y < 0 || w->y > 342-w->height ||
        w->down > 1u || w->dragging > 1u || w->pattern > 1u)
        return 0;
    if (!w->dragging)
        return w->grab_x == 0 && w->grab_y == 0;
    return w->down && w->grab_x >= 0 && w->grab_x < w->width &&
           w->grab_y >= 0 && w->grab_y < w->title;
}

static int art_ok(const cs_cursor_art *a)
{
    unsigned i;
    if (!a->bits || a->width < 1u || a->width > 16u || a->height < 1u || a->height > 16u ||
        a->hot_x >= a->width || a->hot_y >= a->height)
        return 0;
    for (i = 0; i < 32u; i++) {
        unsigned row = i / 2u, col = (i % 2u) * 8u;
        unsigned valid = 0;
        unsigned b;
        if (row < a->height)
            for (b = 0; b < 8u; b++)
                if (col + b < a->width)
                    valid |= 0x80u >> b;
        if ((a->bits->mask[i] & ~valid) || (a->bits->image[i] & ~valid) ||
            (a->bits->image[i] & (unsigned)~a->bits->mask[i]))
            return 0;
    }
    return 1;
}

static void eff(cs_window_scene_effect *o, cs_cursor_token t, cs_rect d, uint32_t st, int32_t c)
{
    o->token.binding=t.binding; o->token.revision=t.revision;
    o->dirty.left=d.left; o->dirty.top=d.top; o->dirty.right=d.right; o->dirty.bottom=d.bottom;
    o->stage=st; o->cause=c;
}

struct ctx { const cs_surface *s; int32_t ox, oy, bt, bb; };

/* Fill a logical rect clipped to the logical canvas and the physical band. */
static int rect(const struct ctx *c, int32_t l, int32_t t, int32_t r, int32_t b, uint32_t v)
{
    cs_rect q;
    q.left = mx(l, 0) + c->ox; q.right = mn(r, CS_PRODUCT_WIDTH) + c->ox;
    q.top = mx(mx(t, 0) + c->oy, c->bt); q.bottom = mn(mn(b, CS_PRODUCT_HEIGHT) + c->oy, c->bb);
    if (q.left >= q.right || q.top >= q.bottom)
        return 0;
    return cs_surface_fill(c->s, q, v) != CS_SURFACE_OK;
}

static int draw_window(const struct ctx *c, const cs_window *w, uint32_t blk, uint32_t wht)
{
    int32_t ct = w->y + w->title, ce = w->y + w->height - 1, lx, by, bx;
    if (rect(c, w->x, w->y, w->x + w->width, w->y + w->height, blk) ||
        rect(c, w->x + 1, w->y + 1, w->x + w->width - 1, w->y + w->title - 1, wht) ||
        rect(c, w->x + 1, ct, w->x + w->width - 1, ce, wht))
        return 1;
    if (w->pattern) {
        for (lx = 4; lx < w->width - 1; lx += 8)
            if (rect(c, w->x + lx, ct, w->x + mn(lx + 4, w->width - 1), ce, blk))
                return 1;
        return 0;
    }
    for (by = 0; ct + by * 8 < ce; by++)
        for (bx = 0; bx * 8 < w->width - 1; bx++)
            if (((bx ^ by) & 1) &&
                rect(c, w->x + mx(bx * 8, 1), ct + by * 8, w->x + mn(bx * 8 + 8, w->width - 1),
                     mn(ct + by * 8 + 8, ce), blk))
                return 1;
    return 0;
}

int cs_window_scene_compose(cs_cursor_state *cursor, cs_cursor_token token,
    const cs_cursor_art *art, const cs_product_state *product,
    const cs_window *old_window, const cs_window *window, uint32_t old_inputs,
    int32_t x, int32_t y, cs_window_scene_effect *out)
{
    cs_cursor_info info;
    const cs_surface *s;
    cs_surface chk, band;
    sp w[4], r[7];
    cs_cursor_token nt;
    cs_rect dirty;
    struct ctx c;
    int32_t W, H, lo = 0, hi = 0, rows;
    int full, render = 0, counter, i, j, nr = 0, rc;
    uint32_t blk, wht;

    if (!cursor || !art || !product || !window || !out)
        return CS_WINDOW_SCENE_ERR_ARGUMENT;
    w[0] = mk(cursor, sizeof *cursor); w[1] = mk(out, sizeof *out);
    if (bad(w[0]) || bad(w[1]) || hit(w[0], w[1]))
        return CS_WINDOW_SCENE_ERR_ARGUMENT;
    if (cs_cursor_query(cursor, &info) != CS_CURSOR_OK)
        return CS_WINDOW_SCENE_ERR_CURSOR;
    full = old_window == NULL;
    if (info.fault != 0u || info.token.binding != token.binding ||
        info.token.revision != token.revision ||
        info.phase != (full ? CS_CURSOR_HIDDEN : CS_CURSOR_VISIBLE))
        return CS_WINDOW_SCENE_ERR_STATE;
    if (!cursor->surface || !cursor->payload)
        return CS_WINDOW_SCENE_ERR_CURSOR;
    r[0] = mk(cursor->surface, sizeof *s);
    if (bad(r[0]))
        return CS_WINDOW_SCENE_ERR_ARGUMENT;
    s = cursor->surface;
    if (s->width < 1 || s->width > CS_PRODUCT_MAX_DIMENSION || s->height < 1 ||
        s->height > CS_PRODUCT_MAX_DIMENSION || !s->storage ||
        cs_surface_init(&chk, s->storage, s->storage_size, s->width, s->height, s->stride,
                        s->format) != CS_SURFACE_OK ||
        s->width != cursor->frame_w || s->height != cursor->frame_h ||
        (uint8_t)s->format != cursor->format)
        return CS_WINDOW_SCENE_ERR_CURSOR;
    r[1] = mk(art, sizeof *art);
    if (bad(r[1]))
        return CS_WINDOW_SCENE_ERR_ARGUMENT;
    r[2] = mk(art->bits, sizeof *art->bits); r[3] = mk(product, sizeof *product);
    r[4] = mk(window, sizeof *window);
    if (bad(r[3]) || bad(r[4])) return CS_WINDOW_SCENE_ERR_ARGUMENT;
    nr = 5;
    if (product->pixels)
        r[nr++] = mk(product->pixels, 1024u);
    if (!full)
        r[nr++] = mk(old_window, sizeof *old_window);
    w[2] = mk(s->storage, s->storage_size); w[3] = mk(cursor->payload, cursor->payload_capacity);
    for (i = 0; i < 4; i++)
        if (bad(w[i]))
            return CS_WINDOW_SCENE_ERR_ARGUMENT;
    for (i = 0; i < nr; i++)
        if (bad(r[i]))
            return CS_WINDOW_SCENE_ERR_ARGUMENT;
    for (i = 0; i < 4; i++) {
        for (j = i + 1; j < 4; j++)
            if (hit(w[i], w[j]))
                return CS_WINDOW_SCENE_ERR_ARGUMENT;
        for (j = 0; j < nr; j++)
            if (hit(w[i], r[j]))
                return CS_WINDOW_SCENE_ERR_ARGUMENT;
    }
    if (product->phases > 6u || product->success > 1u || x < 0 || x > 511 || y < 0 || y > 341 ||
        !art_ok(art) || !win_ok(window) ||
        (!full && (!win_ok(old_window) || old_window->width != window->width ||
                   old_window->height != window->height || old_window->title != window->title)))
        return CS_WINDOW_SCENE_ERR_ARGUMENT;

    counter = !full && old_inputs != product->inputs;
    if (!full)
        render = old_window->x != window->x || old_window->y != window->y ||
                 old_window->pattern != window->pattern;
    if (!full && !render && !counter) {
        if (cs_cursor_move(cursor, token, art, x, y) != CS_CURSOR_OK)
            return CS_WINDOW_SCENE_ERR_CURSOR;
        dirty.left = dirty.top = dirty.right = dirty.bottom = 0;
        eff(out, token, dirty, CS_WINDOW_SCENE_VISIBLE, 0);
        return CS_WINDOW_SCENE_OK;
    }
    if (token.revision == UINT32_MAX)
        return CS_WINDOW_SCENE_ERR_EXHAUSTED;

    W = s->width; H = s->height;
    c.s = s;
    c.ox = W > CS_PRODUCT_WIDTH ? (W - CS_PRODUCT_WIDTH) / 2 : 0;
    c.oy = H > CS_PRODUCT_HEIGHT ? (H - CS_PRODUCT_HEIGHT) / 2 : 0;
    if (full) {
        c.bt = 0; c.bb = H;
    } else {
        int have = 0;
        if (render) {
            lo = mn(old_window->y, window->y);
            hi = mx(old_window->y + old_window->height, window->y + window->height);
            have = 1;
        }
        if (counter) {
            lo = have ? mn(lo, 286) : 286;
            hi = have ? mx(hi, 306) : 306;
        }
        c.bt = mx(lo + c.oy, 0); c.bb = mn(hi + c.oy, H);
        if (c.bt >= c.bb)
            c.bt = c.bb = 0;
    }
    dirty.left = 0; dirty.right = c.bt < c.bb ? W : 0;
    dirty.top = c.bt; dirty.bottom = c.bb;
    rows = c.bb - c.bt;

    if (info.phase == CS_CURSOR_VISIBLE && cs_cursor_hide(cursor, token) != CS_CURSOR_OK)
        return CS_WINDOW_SCENE_ERR_CURSOR;
    eff(out, token, dirty, CS_WINDOW_SCENE_PAINTING, 0);
    if (rows > 0) {
        size_t off = (size_t)c.bt * s->stride;
        if (cs_surface_init(&band,s->storage+off,s->storage_size-off,s->width,rows,s->stride,s->format)!=CS_SURFACE_OK) {
            out->cause=CS_WINDOW_SCENE_ERR_PAINT; return CS_WINDOW_SCENE_ERR_PAINT;
        }
        blk = s->format == CS_SURFACE_RGBA8 ? 0x000000FFu : 1u;
        wht = s->format == CS_SURFACE_RGBA8 ? 0xFFFFFFFFu : 0u;
        if (!cs_product_scene(&band, W, H, c.bt, product)) {
            out->cause=CS_WINDOW_SCENE_ERR_PAINT; return CS_WINDOW_SCENE_ERR_PAINT;
        }
        rc = draw_window(&c, window, blk, wht);
        if (rc != 0) {
            out->cause = rc;
            return CS_WINDOW_SCENE_ERR_PAINT;
        }
    }
    out->stage = CS_WINDOW_SCENE_PAINTED;
    rc = (int)cs_cursor_advance(cursor, token, &nt);
    if (rc != 0) {
        out->cause = rc;
        return CS_WINDOW_SCENE_ERR_CURSOR;
    }
    eff(out, nt, dirty, CS_WINDOW_SCENE_ADVANCED, 0);
    rc = (int)cs_cursor_show(cursor, nt, art, x, y);
    if (rc != 0) {
        out->cause = rc;
        return CS_WINDOW_SCENE_ERR_CURSOR;
    }
    out->stage = CS_WINDOW_SCENE_VISIBLE;
    return CS_WINDOW_SCENE_OK;
}
