/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "window-session.h"

/* SPEC-0061 session. Freestanding: no libc, allocation or globals. */

static uint32_t pws_inc(uint32_t value)
{
    return value == UINT32_MAX ? value : value + 1u;
}

static int pws_span(uintptr_t at, size_t size)
{
    return at != 0u && size != 0u && size <= (size_t)(UINTPTR_MAX - at);
}

static cs_rect pws_cursor_rect(int32_t x, int32_t y)
{
    cs_rect r;

    r.left = x < INT32_MIN + 3 ? INT32_MIN : x - 3;
    r.top = y < INT32_MIN + 3 ? INT32_MIN : y - 3;
    r.right = x > INT32_MAX - 4 ? INT32_MAX : x + 4;
    r.bottom = y > INT32_MAX - 4 ? INT32_MAX : y + 4;
    return r;
}

static int pws_overlap(uintptr_t a, size_t a_size, uintptr_t b, size_t b_size)
{
    return a <= b + (uintptr_t)(b_size - 1u) && b <= a + (uintptr_t)(a_size - 1u);
}

/* Numeric checks only; no caller writes, pixel reads or I/O. */
static int pws_preflight(const cs_pc_window_session *s,
    const cs_pc_window_config *c)
{
    uintptr_t at[6];
    size_t size[6];
    cs_fb_target probe;
    unsigned i, j;

    if (s == NULL || c == NULL)
        return 0;
    at[0] = (uintptr_t)s;
    size[0] = sizeof *s;
    at[1] = (uintptr_t)c;
    size[1] = sizeof *c;
    if ((at[0] & ((uintptr_t)_Alignof(cs_pc_window_session) - 1u)) != 0u
        || (at[1] & ((uintptr_t)_Alignof(cs_pc_window_config) - 1u)) != 0u
        || !pws_span(at[0], size[0]) || !pws_span(at[1], size[1])
        || pws_overlap(at[0], size[0], at[1], size[1]))
        return 0;
    if (c->io.read == NULL || c->io.write == NULL || c->canvas == NULL
        || c->payload == NULL || c->row == NULL
        || c->canvas_capacity < (size_t)CS_PWS_CANVAS
        || c->payload_capacity < (size_t)CS_PWS_PAYLOAD)
        return 0;
#if SIZE_MAX > UINT32_MAX
    if (c->payload_capacity > UINT32_MAX)
        return 0;
#endif
    if (cs_fb_init(&probe, c->target.base, c->target.size, c->target.width,
            c->target.height, c->target.pitch, c->target.format) != CS_FB_OK
        || c->target.width < (uint32_t)CS_PRODUCT_WIDTH
        || c->target.height < (uint32_t)CS_PRODUCT_HEIGHT
        || (uint64_t)c->row_capacity < (uint64_t)c->target.width * 4u)
        return 0;
    at[2] = (uintptr_t)c->canvas;
    size[2] = c->canvas_capacity;
    at[3] = (uintptr_t)c->payload;
    size[3] = c->payload_capacity;
    at[4] = (uintptr_t)c->row;
    size[4] = c->row_capacity;
    at[5] = (uintptr_t)c->target.base;
    size[5] = c->target.size;
    for (i = 2u; i < 6u; ++i) {
        if (!pws_span(at[i], size[i]))
            return 0;
        for (j = 0u; j < i; ++j)
            if (pws_overlap(at[i], size[i], at[j], size[j]))
                return 0;
    }
    return 1;
}

static uint32_t pws_fail(cs_pc_window_session *s, uint32_t result,
    uint32_t stage, int32_t cause)
{
    s->phase = CS_PWS_FAULTED;
    s->result = result;
    s->stage = stage;
    s->cause = cause;
    return result;
}

static void pws_clear(cs_ps2_bus_delivery *delivery)
{
    delivery->kind = CS_PS2_DELIVERY_NONE;
    delivery->packet.dx = 0;
    delivery->packet.dy = 0;
    delivery->packet.buttons = 0u;
}

uint32_t cs_pc_window_session_start(cs_pc_window_session *s,
    const cs_pc_window_config *c)
{
    cs_cursor_token token;
    cs_window_config window;
    cs_product_state product;
    int r;

    if (!pws_preflight(s, c))
        return CS_PWS_ARGUMENT;
    s->target.base = c->target.base;
    s->target.size = c->target.size;
    s->target.width = c->target.width;
    s->target.height = c->target.height;
    s->target.pitch = c->target.pitch;
    s->target.format = c->target.format;
    s->io.read = c->io.read;
    s->io.write = c->io.write;
    s->io.context = c->io.context;
    s->row = c->row;
    s->row_capacity = c->row_capacity;
    s->last_tick = 0u;
    s->magic = CS_PWS_MAGIC;
    s->phase = CS_PWS_FAULTED;
    s->result = CS_PWS_STARTUP;
    s->stage = CS_PWS_CURSOR;
    s->cause = 0;
    s->clock_seen = 0u;
    s->stalled = 0u;
    s->consumed = 0u;
    s->drained = 0u;
    s->ram_frames = 0u;
    s->published_frames = 0u;
    s->bus_result = 0u;
    pws_clear(&s->delivery);
    s->presentation.phase = 0u;
    s->presentation.rows = 0u;
    s->presentation.required = 0u;
    s->presentation.cause = 0;
    cs_window_cursor_bits_init(&s->bits);
    s->art.bits = &s->bits;
    s->art.width = 7u;
    s->art.height = 7u;
    s->art.hot_x = 3u;
    s->art.hot_y = 3u;
    token.binding = 0u;
    token.revision = 0u;
    r = (int)cs_surface_init(&s->canvas, c->canvas, c->canvas_capacity,
        CS_PRODUCT_WIDTH, CS_PRODUCT_HEIGHT, (size_t)64u, CS_SURFACE_MONO1_MSB);
    if (r == CS_SURFACE_OK)
        r = (int)cs_input_init(&s->queue, s->records,
            (size_t)CS_PWS_QUEUE_CAPACITY);
    if (r == CS_INPUT_OK)
        r = (int)cs_cursor_init(&s->cursor);
    if (r == CS_CURSOR_OK)
        r = (int)cs_cursor_bind(&s->cursor, &s->canvas, c->payload,
            c->payload_capacity, &token);
    if (r != CS_CURSOR_OK)
        return pws_fail(s, CS_PWS_STARTUP, CS_PWS_CURSOR, r);
    window.x = 80;
    window.y = 70;
    window.width = 224;
    window.height = 144;
    window.title = 18;
    product.value = 97u;
    product.inputs = 0u;
    product.progress = 16u;
    product.phases = 6u;
    product.success = 1u;
    product.pixels = NULL;
    s->stage = CS_PWS_BEGIN;
    r = cs_window_input_begin(&s->owner, &s->cursor, token, &s->art,
        &window, &product, 0u);
    if (r != CS_WINDOW_INPUT_OK && r != CS_WINDOW_INPUT_CHANGED)
        return pws_fail(s, CS_PWS_STARTUP, CS_PWS_BEGIN, r);
    s->stage = CS_PWS_CANCEL;
    r = cs_window_input_cancel(&s->owner);
    if (r != CS_WINDOW_INPUT_OK && r != CS_WINDOW_INPUT_CHANGED)
        return pws_fail(s, CS_PWS_STARTUP, CS_PWS_CANCEL, r);
    s->ram_frames = 1u;
    s->stage = CS_PWS_PUBLISH;
    r = cs_pc_mono_present(&s->target, &s->canvas, s->row, s->row_capacity,
        1u, &s->presentation);
    if (r != CS_MONO_OK || s->presentation.phase != CS_MONO_COMPLETE)
        return pws_fail(s, CS_PWS_PRESENT, CS_PWS_PUBLISH, r);
    s->published_frames = 1u;
    s->stage = CS_PWS_BUS;
    s->bus_result = cs_ps2_bus_begin(&s->bus);
    if (s->bus_result > CS_PS2_READY)
        return pws_fail(s, CS_PWS_STARTUP, CS_PWS_BUS, (int32_t)s->bus_result);
    s->phase = CS_PWS_RUNNING;
    s->result = CS_PWS_OK;
    return CS_PWS_OK;
}

uint32_t cs_pc_window_session_poll(cs_pc_window_session *s, uint64_t now,
    uint32_t clock_ok)
{
    cs_input_record record;
    uint32_t r, kind, i;
    int owner;
    cs_rect rects[3];
    size_t count;
    int32_t old_x, old_y;

    if (s == NULL || s->magic != CS_PWS_MAGIC)
        return CS_PWS_ARGUMENT;
    if (s->phase == CS_PWS_STOPPED)
        return CS_PWS_OK;
    if (s->phase == CS_PWS_FAULTED)
        return s->result;
    if (s->phase != CS_PWS_RUNNING)
        return CS_PWS_STATE;
    pws_clear(&s->delivery);
    s->consumed = 0u;
    s->drained = 0u;
    s->stage = CS_PWS_TIME;
    s->cause = 0;
    if (clock_ok != 1u || (s->clock_seen != 0u && now < s->last_tick))
        return pws_fail(s, CS_PWS_CLOCK, CS_PWS_TIME, 0);
    if (s->clock_seen == 0u || now != s->last_tick) {
        s->clock_seen = 1u;
        s->last_tick = now;
        s->stalled = 0u;
    } else {
        s->stalled = pws_inc(s->stalled);
        if (s->stalled >= (uint32_t)CS_PWS_STALL_LIMIT)
            return pws_fail(s, CS_PWS_CLOCK, CS_PWS_TIME, 0);
    }
    s->stage = CS_PWS_POLL;
    r = cs_ps2_bus_poll_delivery(&s->bus, &s->io, now, &s->queue, &s->delivery);
    s->bus_result = r;
    if (r > CS_PS2_READY)
        return pws_fail(s, CS_PWS_CONTROLLER, CS_PWS_POLL, (int32_t)r);
    s->stage = CS_PWS_DRAIN;
    for (i = 0u; i < (uint32_t)CS_PWS_QUEUE_CAPACITY; ++i) {
        cs_input_result q = cs_input_pop(&s->queue, &record);
        if (q == CS_INPUT_ERR_EMPTY)
            break;
        if (q != CS_INPUT_OK)
            return pws_fail(s, CS_PWS_QUEUE, CS_PWS_DRAIN, (int32_t)q);
        s->drained = pws_inc(s->drained);
    }
    kind = s->delivery.kind;
    if (kind == CS_PS2_DELIVERY_NONE)
        return CS_PWS_OK;
    s->stage = CS_PWS_DISPATCH;
    if (kind != CS_PS2_DELIVERY_PACKET && kind != CS_PS2_DELIVERY_LOST)
        return pws_fail(s, CS_PWS_CONTROLLER, CS_PWS_DISPATCH, CS_PS2_STATE);
    s->consumed = 1u;
    old_x = s->owner.pointer.x;
    old_y = s->owner.pointer.y;
    owner = kind == CS_PS2_DELIVERY_PACKET
        ? cs_pc_window_packet(&s->owner, &s->delivery.packet)
        : cs_window_input_cancel(&s->owner);
    if (owner != CS_WINDOW_INPUT_OK && owner != CS_WINDOW_INPUT_CHANGED)
        return pws_fail(s, CS_PWS_OWNER, CS_PWS_DISPATCH, owner);
    s->ram_frames = pws_inc(s->ram_frames);
    count = 0u;
    if (s->owner.render.dirty.left < s->owner.render.dirty.right
        && s->owner.render.dirty.top < s->owner.render.dirty.bottom)
        rects[count++] = s->owner.render.dirty;
    if (s->owner.pointer.x != old_x || s->owner.pointer.y != old_y) {
        rects[count++] = pws_cursor_rect(old_x, old_y);
        rects[count++] = pws_cursor_rect(s->owner.pointer.x, s->owner.pointer.y);
    }
    if (count == 0u)
        return CS_PWS_OK;
    s->stage = CS_PWS_PUBLISH;
    owner = cs_pc_mono_present_rects(&s->target, &s->canvas, s->row,
        s->row_capacity, rects, count, &s->presentation);
    if (owner != CS_MONO_OK || s->presentation.phase != CS_MONO_COMPLETE)
        return pws_fail(s, CS_PWS_PRESENT, CS_PWS_PUBLISH, owner);
    s->published_frames = pws_inc(s->published_frames);
    return CS_PWS_OK;
}

uint32_t cs_pc_window_session_stop(cs_pc_window_session *s)
{
    if (s == NULL || s->magic != CS_PWS_MAGIC)
        return CS_PWS_ARGUMENT;
    if (s->phase == CS_PWS_RUNNING) {
        s->phase = CS_PWS_STOPPED;
        s->result = CS_PWS_OK;
        return CS_PWS_OK;
    }
    if (s->phase == CS_PWS_STOPPED)
        return CS_PWS_OK;
    if (s->phase == CS_PWS_FAULTED)
        return s->result;
    return CS_PWS_STATE;
}
