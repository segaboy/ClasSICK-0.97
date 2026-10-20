/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "cursor.h"

_Static_assert(sizeof(cs_cursor_state) <= 64u, "cursor state over 64 bytes");
_Static_assert(sizeof(cs_cursor_bits) == 64u, "cursor bits not 64 bytes");

#define CUR_FRAME_MAX 8192
#define CUR_CANVAS_W 512
#define CUR_CANVAS_H 342
#define CUR_U32_MAX 0xFFFFFFFFu
#define CUR_PH(p) (1u << (p))
#define CUR_SPANS 5

/* Accumulated fixed/derived byte ranges for the ALIAS steps. */
typedef struct {
    uintptr_t b[CUR_SPANS];
    uintptr_t e[CUR_SPANS];
    unsigned n, w;
} cur_spans;

static void cur_spans_init(cur_spans *sp)
{
    sp->n = 0u;
    sp->w = 0u;
}

/* Refuses a wrapping range, or overlap where either range is writable.
 * Read-only ranges may overlap each other. */
static int span_add(cur_spans *sp, const void *p, size_t n, unsigned writable)
{
    uintptr_t b = (uintptr_t)p, e;
    unsigned i;

    if (n > UINTPTR_MAX - b)
        return 0;
    e = b + (uintptr_t)n;
    for (i = 0u; i < sp->n; i++) {
        if ((writable != 0u || ((sp->w >> i) & 1u) != 0u)
            && b < sp->e[i] && sp->b[i] < e)
            return 0;
    }
    /* Five retained ranges suffice: a sixth range is terminal read-only
     * art bits, checked against all retained writers without being stored. */
    if (sp->n == CUR_SPANS)
        return writable == 0u;
    sp->b[sp->n] = b;
    sp->e[sp->n] = e;
    if (writable != 0u)
        sp->w |= 1u << sp->n;
    sp->n++;
    return 1;
}

static int32_t cur_min(int32_t a, int32_t b) { return a < b ? a : b; }
static int32_t cur_max(int32_t a, int32_t b) { return a > b ? a : b; }

static int32_t cur_origin(int32_t extent, int32_t canvas)
{
    return extent > canvas ? (extent - canvas) / 2 : 0;
}

static size_t cur_required(uint32_t format)
{
    return format == (uint32_t)CS_SURFACE_MONO1_MSB
        ? (size_t)CS_CURSOR_MONO_PAYLOAD : (size_t)CS_CURSOR_RGBA_PAYLOAD;
}

/* Section 6.1: reads only state fields. */
static int cur_state_ok(const cs_cursor_state *st)
{
    int32_t l = st->save_left, t = st->save_top;
    int32_t r = st->save_right, b = st->save_bottom;
    int32_t fw = st->frame_w, fh = st->frame_h;
    int32_t ox = st->origin_x, oy = st->origin_y;

    if (st->magic != (uint32_t)CS_CURSOR_MAGIC || st->reserved != 0u
        || st->phase < CS_CURSOR_UNBOUND || st->phase > CS_CURSOR_REPAIR)
        return 0;
    if (st->phase == CS_CURSOR_UNBOUND)
        return st->surface == NULL && st->payload == NULL
            && st->binding == 0u && st->revision == 0u
            && st->payload_capacity == 0u && st->format == 0u
            && st->fault == 0u && ox == 0 && oy == 0 && fw == 0 && fh == 0
            && l == 0 && t == 0 && r == 0 && b == 0;
    if (st->surface == NULL || st->payload == NULL || st->binding == 0u
        || (st->format != CS_SURFACE_MONO1_MSB
            && st->format != CS_SURFACE_RGBA8))
        return 0;
    if (st->next_binding != (st->binding == CUR_U32_MAX ? 0u : st->binding + 1u)
        || st->revision == 0u)
        return 0;
    if (st->payload_capacity < cur_required(st->format)
        || st->payload_capacity > UINTPTR_MAX - (uintptr_t)st->payload)
        return 0;
    if (fw < 1 || fw > CUR_FRAME_MAX || fh < 1 || fh > CUR_FRAME_MAX
        || ox != cur_origin(fw, CUR_CANVAS_W) || oy != cur_origin(fh, CUR_CANVAS_H))
        return 0;
    if (st->fault > 1u || (st->fault == 1u
        && (st->revision != CUR_U32_MAX || st->phase == CS_CURSOR_VISIBLE)))
        return 0;
    if (l == 0 && t == 0 && r == 0 && b == 0)
        return 1;
    if (st->phase == CS_CURSOR_HIDDEN)
        return 0;
    return ox <= l && l < r && r <= cur_min(fw, ox + CUR_CANVAS_W) && r - l <= 16
        && oy <= t && t < b && b <= cur_min(fh, oy + CUR_CANVAS_H) && b - t <= 16;
}

/* Section 6 step 7 checked descriptor fields (no snapshot). */
static int cur_surface_ok(const cs_surface *s)
{
    size_t w, min_stride;

    if (s->storage == NULL
        || (s->format != CS_SURFACE_MONO1_MSB && s->format != CS_SURFACE_RGBA8)
        || s->width < 1 || s->width > CUR_FRAME_MAX
        || s->height < 1 || s->height > CUR_FRAME_MAX)
        return 0;
    w = (size_t)s->width;
    min_stride = s->format == CS_SURFACE_MONO1_MSB ? (w + 7u) / 8u : 4u * w;
    /* stride*H <= storage_size without forming the product. */
    return s->stride >= min_stride
        && s->stride <= s->storage_size / (size_t)s->height;
}

static void cur_clear(cs_cursor_state *st)
{
    st->surface = NULL;
    st->payload = NULL;
    st->magic = (uint32_t)CS_CURSOR_MAGIC;
    st->binding = 0u;
    st->revision = 0u;
    st->payload_capacity = 0u;
    st->origin_x = 0;
    st->origin_y = 0;
    st->frame_w = 0;
    st->frame_h = 0;
    st->save_left = 0;
    st->save_top = 0;
    st->save_right = 0;
    st->save_bottom = 0;
    st->phase = (uint8_t)CS_CURSOR_UNBOUND;
    st->format = 0u;
    st->fault = 0u;
    st->reserved = 0u;
}

static void cur_zero_save(cs_cursor_state *st)
{
    st->save_left = 0;
    st->save_top = 0;
    st->save_right = 0;
    st->save_bottom = 0;
}

static void cur_put_rect(cs_rect *out, const cs_cursor_state *st)
{
    out->left = st->save_left;
    out->top = st->save_top;
    out->right = st->save_right;
    out->bottom = st->save_bottom;
}

/* Steps 3-5 for token calls; every phase is in runs, busy or STATE. */
static cs_cursor_result cur_gate(const cs_cursor_state *st,
    const cs_cursor_token *tok, unsigned runs, unsigned busy)
{
    unsigned bit;

    if (!cur_state_ok(st))
        return CS_CURSOR_ERR_STATE;
    bit = CUR_PH(st->phase);
    if ((bit & (runs | busy)) == 0u)
        return CS_CURSOR_ERR_STATE;
    if (tok->binding != st->binding || tok->revision != st->revision)
        return CS_CURSOR_ERR_STALE;
    if ((bit & busy) != 0u)
        return CS_CURSOR_ERR_BUSY;
    return CS_CURSOR_OK;
}

static int cur_art_ok(const cs_cursor_art *art)
{
    const cs_cursor_bits *bits = art->bits;
    unsigned w = art->width, h = art->height, r, k, allowed, m, im;

    if (w < 1u || w > 16u || h < 1u || h > 16u
        || (unsigned)art->hot_x >= w || (unsigned)art->hot_y >= h)
        return 0;
    for (r = 0u; r < 16u; r++) {
        for (k = 0u; k < 2u; k++) {
            m = bits->mask[2u * r + k];
            im = bits->image[2u * r + k];
            if (r >= h || w <= 8u * k)
                allowed = 0u;
            else if (w >= 8u * k + 8u)
                allowed = 0xFFu;
            else
                allowed = (0xFFu << (8u - (w - 8u * k))) & 0xFFu;
            if ((im & ~m) != 0u || ((m | im) & ~allowed) != 0u)
                return 0;
        }
    }
    return 1;
}

/* Steps 7-11 for show/move (art non-NULL) and hide (art NULL). */
static cs_cursor_result cur_prepare(const cs_cursor_state *st, cur_spans *sp,
    const cs_cursor_art *art, int32_t x, int32_t y)
{
    const cs_surface *s = st->surface;

    if (!span_add(sp, s, sizeof *s, 0u))
        return CS_CURSOR_ERR_ALIAS;
    if (!cur_surface_ok(s) || s->width != st->frame_w
        || s->height != st->frame_h
        || (uint32_t)s->format != (uint32_t)st->format)
        return CS_CURSOR_ERR_SURFACE;
    if (art != NULL && art->bits == NULL)
        return CS_CURSOR_ERR_ART;
    if (!span_add(sp, s->storage, s->storage_size, 1u)
        || !span_add(sp, st->payload, st->payload_capacity, 1u))
        return CS_CURSOR_ERR_ALIAS;
    if (art == NULL)
        return CS_CURSOR_OK;
    if (!span_add(sp, art->bits, sizeof *art->bits, 0u))
        return CS_CURSOR_ERR_ALIAS;
    if (!cur_art_ok(art))
        return CS_CURSOR_ERR_ART;
    if (x < 0 || x >= CUR_CANVAS_W || y < 0 || y >= CUR_CANVAS_H)
        return CS_CURSOR_ERR_COORD;
    return CS_CURSOR_OK;
}

/* Bits of Mono byte `byte` that cover columns [l,r). */
static uint8_t cur_bits_in(int32_t byte, int32_t l, int32_t r)
{
    int32_t c0 = byte * 8;
    int32_t lo = l > c0 ? l - c0 : 0;
    int32_t hi = r < c0 + 8 ? r - c0 : 8;

    return (uint8_t)((0xFFu >> lo) & (0xFFu << (8 - hi)) & 0xFFu);
}

/* Exact restore of the saved rect; Mono preserves bits outside [l,r). */
static void cur_restore(const cs_cursor_state *st)
{
    const cs_surface *s = st->surface;
    const uint8_t *src = st->payload;
    int32_t l = st->save_left, r = st->save_right, y, c;
    uint8_t *row;
    uint8_t m;

    for (y = st->save_top; y < st->save_bottom; y++) {
        row = s->storage + (size_t)y * s->stride;
        if (st->format == CS_SURFACE_MONO1_MSB) {
            for (c = l / 8; c <= (r - 1) / 8; c++) {
                m = cur_bits_in(c, l, r);
                row[(size_t)c] = (uint8_t)((row[(size_t)c] & ~m) | (*src & m));
                src++;
            }
        } else {
            for (c = 4 * l; c < 4 * r; c++)
                row[(size_t)c] = *src++;
        }
    }
}

/* Clip, record the rect, save the dense background, then draw opaque bits. */
static void cur_save_draw(cs_cursor_state *st, const cs_cursor_art *art,
    int32_t x, int32_t y)
{
    const cs_surface *s = st->surface;
    const cs_cursor_bits *bits = art->bits;
    uint8_t *dst = st->payload;
    int32_t ax = st->origin_x + x - art->hot_x;
    int32_t ay = st->origin_y + y - art->hot_y;
    int32_t l = cur_max(ax, st->origin_x);
    int32_t t = cur_max(ay, st->origin_y);
    int32_t r = cur_min(ax + art->width,
        cur_min(st->frame_w, st->origin_x + CUR_CANVAS_W));
    int32_t b = cur_min(ay + art->height,
        cur_min(st->frame_h, st->origin_y + CUR_CANVAS_H));
    int32_t row_y, c, i, lo, hi;
    int mono = st->format == CS_SURFACE_MONO1_MSB;
    uint8_t *row;
    uint8_t v;
    uint32_t at, k; /* Art index<=31; frame byte index<=32767. */
    unsigned bit;

    if (l >= r || t >= b) {
        l = 0;
        t = 0;
        r = 0;
        b = 0;
    }
    st->save_left = (int16_t)l;
    st->save_top = (int16_t)t;
    st->save_right = (int16_t)r;
    st->save_bottom = (int16_t)b;
    for (row_y = t; row_y < b; row_y++) {
        row = s->storage + (size_t)row_y * s->stride;
        lo = mono ? l / 8 : 4 * l;
        hi = mono ? (r - 1) / 8 + 1 : 4 * r;
        for (c = lo; c < hi; c++)
            *dst++ = row[(size_t)c];
        for (c = l; c < r; c++) {
            i = c - ax;
            at = (uint32_t)(2 * (row_y - ay) + i / 8);
            bit = 0x80u >> (i % 8);
            if ((bits->mask[at] & bit) == 0u)
                continue;
            if (mono) {
                k = (uint32_t)(c / 8);
                v = (uint8_t)(0x80u >> (c % 8));
                if ((bits->image[at] & bit) != 0u)
                    row[k] = (uint8_t)(row[k] | v);
                else
                    row[k] = (uint8_t)(row[k] & ~v);
            } else {
                k = (uint32_t)c * 4u;
                v = (bits->image[at] & bit) != 0u ? (uint8_t)0x00u : (uint8_t)0xFFu;
                row[k] = v;
                row[k + 1u] = v;
                row[k + 2u] = v;
                row[k + 3u] = (uint8_t)0xFFu;
            }
        }
    }
}

cs_cursor_result cs_cursor_init(cs_cursor_state *st)
{
    cur_spans sp;

    if (st == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u))
        return CS_CURSOR_ERR_ALIAS;
    cur_clear(st);
    st->next_binding = 1u;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_bind(cs_cursor_state *st, const cs_surface *surface,
    uint8_t *payload, size_t capacity, cs_cursor_token *out)
{
    cur_spans sp;
    uint32_t id;

    if (st == NULL || surface == NULL || payload == NULL || out == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u) || !span_add(&sp, out, sizeof *out, 1u)
        || !span_add(&sp, surface, sizeof *surface, 0u))
        return CS_CURSOR_ERR_ALIAS;
    if (!cur_state_ok(st) || st->phase != CS_CURSOR_UNBOUND)
        return CS_CURSOR_ERR_STATE;
    if (st->next_binding == 0u)
        return CS_CURSOR_ERR_EXHAUSTED;
    if (!cur_surface_ok(surface))
        return CS_CURSOR_ERR_SURFACE;
    /* (capacity >> 1) > 0x7FFFFFFF is capacity > 0xFFFFFFFF on any size_t. */
    if (capacity < cur_required((uint32_t)surface->format)
        || (capacity >> 1) > 0x7FFFFFFFu)
        return CS_CURSOR_ERR_CAPACITY;
    if (!span_add(&sp, surface->storage, surface->storage_size, 1u)
        || !span_add(&sp, payload, capacity, 1u))
        return CS_CURSOR_ERR_ALIAS;

    id = st->next_binding;
    st->surface = surface;
    st->payload = payload;
    st->magic = (uint32_t)CS_CURSOR_MAGIC;
    st->binding = id;
    st->next_binding = id == CUR_U32_MAX ? 0u : id + 1u;
    st->revision = 1u;
    st->payload_capacity = (uint32_t)capacity;
    st->origin_x = (int16_t)cur_origin(surface->width, CUR_CANVAS_W);
    st->origin_y = (int16_t)cur_origin(surface->height, CUR_CANVAS_H);
    st->frame_w = (int16_t)surface->width;
    st->frame_h = (int16_t)surface->height;
    cur_zero_save(st);
    st->phase = (uint8_t)CS_CURSOR_HIDDEN;
    st->format = (uint8_t)surface->format;
    st->fault = 0u;
    st->reserved = 0u;
    out->binding = id;
    out->revision = 1u;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_show(cs_cursor_state *st, cs_cursor_token tok,
    const cs_cursor_art *art, int32_t x, int32_t y)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL || art == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u) || !span_add(&sp, art, sizeof *art, 0u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_HIDDEN), CUR_PH(CS_CURSOR_REPAIR));
    if (rc != CS_CURSOR_OK)
        return rc;
    if (st->fault == 1u)
        return CS_CURSOR_ERR_EXHAUSTED;
    rc = cur_prepare(st, &sp, art, x, y);
    if (rc != CS_CURSOR_OK)
        return rc;
    cur_save_draw(st, art, x, y);
    st->phase = (uint8_t)CS_CURSOR_VISIBLE;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_move(cs_cursor_state *st, cs_cursor_token tok,
    const cs_cursor_art *art, int32_t x, int32_t y)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL || art == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u) || !span_add(&sp, art, sizeof *art, 0u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_VISIBLE), CUR_PH(CS_CURSOR_REPAIR));
    if (rc != CS_CURSOR_OK)
        return rc;
    rc = cur_prepare(st, &sp, art, x, y);
    if (rc != CS_CURSOR_OK)
        return rc;
    /* Restore first so overlapping old/new rects save true background. */
    cur_restore(st);
    cur_save_draw(st, art, x, y);
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_hide(cs_cursor_state *st, cs_cursor_token tok)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_VISIBLE), CUR_PH(CS_CURSOR_REPAIR));
    if (rc != CS_CURSOR_OK)
        return rc;
    rc = cur_prepare(st, &sp, NULL, 0, 0);
    if (rc != CS_CURSOR_OK)
        return rc;
    cur_restore(st);
    cur_zero_save(st);
    st->phase = (uint8_t)CS_CURSOR_HIDDEN;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_advance(cs_cursor_state *st, cs_cursor_token tok,
    cs_cursor_token *out)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL || out == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u) || !span_add(&sp, out, sizeof *out, 1u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_HIDDEN),
        CUR_PH(CS_CURSOR_VISIBLE) | CUR_PH(CS_CURSOR_REPAIR));
    if (rc != CS_CURSOR_OK)
        return rc;
    if (st->revision == CUR_U32_MAX)
        return CS_CURSOR_ERR_EXHAUSTED;
    st->revision++;
    out->binding = st->binding;
    out->revision = st->revision;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_discard(cs_cursor_state *st, cs_cursor_token tok,
    cs_rect *out_old)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL || out_old == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u)
        || !span_add(&sp, out_old, sizeof *out_old, 1u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_VISIBLE) | CUR_PH(CS_CURSOR_HIDDEN),
        CUR_PH(CS_CURSOR_REPAIR));
    if (rc != CS_CURSOR_OK)
        return rc;
    cur_put_rect(out_old, st);
    st->phase = (uint8_t)CS_CURSOR_REPAIR;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_repaired(cs_cursor_state *st, cs_cursor_token tok,
    cs_rect covered, cs_cursor_token *out)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL || out == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u) || !span_add(&sp, out, sizeof *out, 1u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_REPAIR), 0u);
    if (rc != CS_CURSOR_OK)
        return rc;
    if (covered.left > covered.right || covered.top > covered.bottom)
        return CS_CURSOR_ERR_COVERAGE;
    /* Canonical rects are all-zero or nonempty; empty is always contained. */
    if (st->save_left != st->save_right
        && (covered.left > st->save_left || covered.top > st->save_top
            || covered.right < st->save_right || covered.bottom < st->save_bottom))
        return CS_CURSOR_ERR_COVERAGE;
    cur_zero_save(st);
    if (st->revision != CUR_U32_MAX)
        st->revision++;
    else
        st->fault = 1u;
    st->phase = (uint8_t)CS_CURSOR_HIDDEN;
    out->binding = st->binding;
    out->revision = st->revision;
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_unbind(cs_cursor_state *st, cs_cursor_token tok)
{
    cur_spans sp;
    cs_cursor_result rc;

    if (st == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u))
        return CS_CURSOR_ERR_ALIAS;
    rc = cur_gate(st, &tok, CUR_PH(CS_CURSOR_HIDDEN),
        CUR_PH(CS_CURSOR_VISIBLE) | CUR_PH(CS_CURSOR_REPAIR));
    if (rc != CS_CURSOR_OK)
        return rc;
    cur_clear(st);
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_abandon(cs_cursor_state *st, cs_rect *out_old)
{
    cur_spans sp;

    if (st == NULL || out_old == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 1u)
        || !span_add(&sp, out_old, sizeof *out_old, 1u))
        return CS_CURSOR_ERR_ALIAS;
    if (!cur_state_ok(st) || st->phase == CS_CURSOR_UNBOUND)
        return CS_CURSOR_ERR_STATE;
    cur_put_rect(out_old, st);
    cur_clear(st);
    return CS_CURSOR_OK;
}

cs_cursor_result cs_cursor_query(const cs_cursor_state *st, cs_cursor_info *out)
{
    cur_spans sp;

    if (st == NULL || out == NULL)
        return CS_CURSOR_ERR_ARGUMENT;
    cur_spans_init(&sp);
    if (!span_add(&sp, st, sizeof *st, 0u) || !span_add(&sp, out, sizeof *out, 1u))
        return CS_CURSOR_ERR_ALIAS;
    if (!cur_state_ok(st))
        return CS_CURSOR_ERR_STATE;
    out->token.binding = st->binding;
    out->token.revision = st->revision;
    cur_put_rect(&out->saved, st);
    out->phase = st->phase;
    out->fault = st->fault;
    out->reserved[0] = 0u;
    out->reserved[1] = 0u;
    return CS_CURSOR_OK;
}
