/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input.h"

#define ARG CS_WINDOW_INPUT_ARGUMENT
#define STA CS_WINDOW_INPUT_STATE
#define FLT CS_WINDOW_INPUT_FAULT
#define PH(i) ((unsigned)(i).phase)

typedef struct { uintptr_t a, b; int w; } span;

static int mk(span *s, const void *p, size_t n, int w)
{
    uintptr_t a = (uintptr_t)p;
    if (!p || !n || n > (size_t)(UINTPTR_MAX - a)) return 1;
    s->a = a; s->b = a + (uintptr_t)n; s->w = w;
    return 0;
}

/* Nonzero when any pair overlaps and at least one side is writable. */
static int clash(const span *s, int n)
{
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if ((s[i].w || s[j].w) && s[i].a < s[j].b && s[j].a < s[i].b) return 1;
    return 0;
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

/* Numeric preflight of the whole borrowed graph; nonzero means ARGUMENT. */
static int graph(const void *own, size_t on, const cs_cursor_state *c,
    const cs_cursor_art *art, const void *x, size_t xn,
    const cs_product_state *pr, int ext)
{
    span s[10];
    int n = 0;
    const cs_surface *sf;
    cs_surface t;
    if (mk(&s[n++], own, on, 1) || mk(&s[n++], c, sizeof *c, 1) ||
        mk(&s[n++], art, sizeof *art, 0)) return 1;
    if (xn && mk(&s[n++], x, xn, 0)) return 1;
    if (ext && mk(&s[n++], pr, sizeof *pr, 0)) return 1;
    if (clash(s, n)) return 1;
    sf = c->surface;
    if (mk(&s[n++], sf, sizeof *sf, 0) || clash(s, n)) return 1;
    if (cs_surface_init(&t, sf->storage, sf->storage_size, sf->width, sf->height,
        sf->stride, sf->format) != CS_SURFACE_OK) return 1;
    if (mk(&s[n++], sf->storage, sf->storage_size, 1) ||
        mk(&s[n++], c->payload, c->payload_capacity, 1) ||
        mk(&s[n++], art->bits, sizeof *art->bits, 0)) return 1;
    if (pr->pixels && mk(&s[n++], pr->pixels, 1024, 0)) return 1;
    return clash(s, n) || !art_ok(art);
}

static int ptr_ok(const cs_pointer *p)
{
    return p->valid == CS_POINTER_VALID && p->x >= 0 && p->x < CS_POINTER_WIDTH &&
        p->y >= 0 && p->y < CS_POINTER_HEIGHT && p->down <= 1 && p->suppress <= 1 &&
        !(p->down && p->suppress);
}

static int win_ok(const cs_window *w)
{
    if (w->valid != CS_WINDOW_VALID || w->width < 2 || w->width > 512 ||
        w->height < 2 || w->height > 342 || w->title < 1 || w->title >= w->height ||
        w->x < 0 || w->x > 512 - w->width || w->y < 0 || w->y > 342 - w->height ||
        w->down > 1 || w->dragging > 1 || w->pattern > 1) return 0;
    if (!w->dragging) return !w->grab_x && !w->grab_y;
    return w->down && w->grab_x >= 0 && w->grab_x < w->width &&
        w->grab_y >= 0 && w->grab_y < w->title;
}

static int win_eq(const cs_window *a, const cs_window *b)
{
    return a->x == b->x && a->y == b->y && a->width == b->width &&
        a->height == b->height && a->title == b->title && a->grab_x == b->grab_x &&
        a->grab_y == b->grab_y && a->down == b->down && a->dragging == b->dragging &&
        a->pattern == b->pattern && a->valid == b->valid;
}

static void win_copy(cs_window *d, const cs_window *s)
{
    d->x = s->x; d->y = s->y; d->width = s->width; d->height = s->height;
    d->title = s->title; d->grab_x = s->grab_x; d->grab_y = s->grab_y;
    d->down = s->down; d->dragging = s->dragging; d->pattern = s->pattern;
    d->valid = s->valid;
}

/* Nonzero when the cursor cannot be queried or its token differs. */
static int cq(const cs_cursor_state *c, cs_cursor_token t, cs_cursor_info *i)
{
    return cs_cursor_query(c, i) != CS_CURSOR_OK || i->token.binding != t.binding ||
        i->token.revision != t.revision;
}

static int ready(const cs_window_input *o, const void *x, size_t xn)
{
    span a;
    cs_cursor_info i;
    if (mk(&a, o, sizeof *o, 1)) return ARG;
    if (o->valid != CS_WINDOW_INPUT_VALID || o->phase != CS_WINDOW_INPUT_READY ||
        o->stage != CS_WINDOW_INPUT_RENDERED) return STA;
    if (graph(o, sizeof *o, o->cursor, o->art, x, xn, &o->product, 0)) return ARG;
    if (o->product.phases > 6 || o->product.success > 1 ||
        !ptr_ok(&o->pointer) || o->counter.down > 1 || !win_ok(&o->window) ||
        !win_ok(&o->rendered_window)) return STA;
    if (o->pointer.down != o->counter.down || o->counter.down != o->window.down ||
        o->product.inputs != o->counter.presses ||
        o->rendered_inputs != o->product.inputs ||
        !win_eq(&o->window, &o->rendered_window)) return STA;
    if (cq(o->cursor, o->token, &i) || PH(i) != (unsigned)CS_CURSOR_VISIBLE) return STA;
    return 0;
}

static int render(cs_window_input *o, int full)
{
    int r;
    o->render.token = o->token;
    o->render.dirty.left = o->render.dirty.top = 0;
    o->render.dirty.right = o->render.dirty.bottom = 0;
    o->render.stage = 0; o->render.cause = 0;
    r = cs_window_scene_compose(o->cursor, o->token, o->art, &o->product,
        full ? NULL : &o->rendered_window, &o->window, o->rendered_inputs,
        o->pointer.x, o->pointer.y, &o->render);
    if (!r) {
        o->token = o->render.token;
        win_copy(&o->rendered_window, &o->window);
        o->rendered_inputs = o->product.inputs;
        o->phase = CS_WINDOW_INPUT_READY; o->stage = CS_WINDOW_INPUT_RENDERED;
        o->cause = 0;
        return 0;
    }
    if (o->render.stage >= CS_WINDOW_SCENE_ADVANCED) o->token = o->render.token;
    o->phase = CS_WINDOW_INPUT_FAULTED; o->stage = CS_WINDOW_INPUT_WINDOW;
    o->cause = r;
    return r;
}

/* Deliver only the consumers of o->last that have not completed. */
static int logic(cs_window_input *o)
{
    int r;
    cs_window_effect e;
    if (o->stage == CS_WINDOW_INPUT_POINTER) {
        r = 0;
        if (o->last.flags & CS_POINTER_PRESS) r = cs_mouse_button_sample(&o->counter, 1);
        else if (o->last.flags & CS_POINTER_RELEASE) r = cs_mouse_button_sample(&o->counter, 0);
        if (r > 1) { o->cause = r; return r; }
        o->product.inputs = o->counter.presses;
        o->stage = CS_WINDOW_INPUT_COUNTER;
    }
    if (o->stage == CS_WINDOW_INPUT_COUNTER) {
        r = cs_window_apply(&o->window, &o->last, &e);
        if (r > 1) { o->cause = r; return r; }
        o->stage = CS_WINDOW_INPUT_WINDOW;
    }
    return 0;
}

static int finish(cs_window_input *o, int pr, int full)
{
    int f = logic(o);
    if (!f) f = render(o, full);
    return f ? FLT : pr;
}

int cs_window_input_begin(cs_window_input *o, cs_cursor_state *c, cs_cursor_token t,
    const cs_cursor_art *art, const cs_window_config *cfg,
    const cs_product_state *base, uint32_t buttons)
{
    cs_pointer p;
    cs_mouse_button m;
    cs_window w;
    cs_cursor_info i;
    span q;
    if (mk(&q, base, sizeof *base, 0)) return ARG;
    if (graph(o, sizeof *o, c, art, cfg, sizeof *cfg, base, 1)) return ARG;
    if (buttons > CS_POINTER_BUTTON_MASK || base->inputs || base->phases > 6 ||
        base->success > 1) return ARG;
    if (cs_pointer_begin(&p, buttons) || cs_mouse_button_begin(&m, 0) ||
        cs_window_begin(&w, cfg)) return ARG;
    if (cq(c, t, &i) || PH(i) != (unsigned)CS_CURSOR_HIDDEN) return STA;
    o->pointer = p; o->counter = m;
    win_copy(&o->window, &w); win_copy(&o->rendered_window, &w);
    o->product.value = base->value; o->product.inputs = base->inputs;
    o->product.progress = base->progress; o->product.phases = base->phases;
    o->product.success = base->success; o->product.pixels = base->pixels;
    o->cursor = c; o->art = art; o->token = t;
    o->last.x = p.x; o->last.y = p.y;
    o->last.flags = p.suppress ? CS_POINTER_SUPPRESSED : 0u;
    o->last.buttons = buttons;
    o->render.token = t; o->render.stage = 0; o->render.cause = 0;
    o->render.dirty.left = o->render.dirty.top = 0;
    o->render.dirty.right = o->render.dirty.bottom = 0;
    o->rendered_inputs = 0; o->valid = CS_WINDOW_INPUT_VALID;
    o->phase = CS_WINDOW_INPUT_FAULTED; o->stage = CS_WINDOW_INPUT_WINDOW;
    o->cause = 0;
    return render(o, 1) ? FLT : CS_WINDOW_INPUT_OK;
}

int cs_window_input_sample(cs_window_input *o, const cs_pointer_sample *s)
{
    span a[2];
    cs_pointer_event ev;
    int r;
    if (mk(&a[0], o, sizeof *o, 1) || mk(&a[1], s, sizeof *s, 0) || clash(a, 2)) return ARG;
    r = ready(o, s, sizeof *s);
    if (r) return r;
    r = cs_pointer_sample_apply(&o->pointer, s, &ev);
    if (r > 1) return r == CS_POINTER_ARGUMENT ? ARG : STA;
    o->last = ev;
    o->stage = CS_WINDOW_INPUT_POINTER; o->phase = CS_WINDOW_INPUT_FAULTED;
    o->cause = 0;
    return finish(o, r, 0);
}

int cs_window_input_cancel(cs_window_input *o)
{
    cs_pointer_event ev;
    int r = ready(o, NULL, 0);
    if (r) return r;
    r = cs_pointer_cancel(&o->pointer, &ev);
    if (r > 1) return r == CS_POINTER_ARGUMENT ? ARG : STA;
    o->last = ev;
    o->stage = CS_WINDOW_INPUT_POINTER; o->phase = CS_WINDOW_INPUT_FAULTED;
    o->cause = 0;
    return finish(o, r, 0);
}

int cs_window_input_repair(cs_window_input *o, cs_cursor_token t)
{
    span a;
    cs_cursor_info i;
    cs_pointer_event ev;
    int r;
    if (mk(&a, o, sizeof *o, 1)) return ARG;
    if (o->valid != CS_WINDOW_INPUT_VALID || o->phase != CS_WINDOW_INPUT_FAULTED ||
        o->stage < CS_WINDOW_INPUT_POINTER || o->stage > CS_WINDOW_INPUT_WINDOW) return STA;
    if (graph(o, sizeof *o, o->cursor, o->art, NULL, 0, &o->product, 0)) return ARG;
    if (o->product.phases > 6 || o->product.success > 1 ||
        !ptr_ok(&o->pointer) || o->counter.down > 1 || !win_ok(&o->window) ||
        !win_ok(&o->rendered_window)) return STA;
    if (cq(o->cursor, t, &i) || (PH(i) != (unsigned)CS_CURSOR_HIDDEN &&
        PH(i) != (unsigned)CS_CURSOR_VISIBLE)) return STA;
    o->token = t;
    if (logic(o)) return FLT;
    r = cs_pointer_cancel(&o->pointer, &ev);
    if (r > 1) { o->cause = r; return FLT; }
    o->last = ev; o->stage = CS_WINDOW_INPUT_POINTER;
    if (logic(o)) return FLT;
    if (PH(i) == (unsigned)CS_CURSOR_VISIBLE) {
        r = (int)cs_cursor_hide(o->cursor, t);
        if (!r) r = (int)cs_cursor_query(o->cursor, &i);
        if (r) { o->cause = r; return FLT; }
        o->token = i.token;
    }
    return render(o, 1) ? FLT : CS_WINDOW_INPUT_OK;
}
