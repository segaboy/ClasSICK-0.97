/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "window.h"
#include "../pc/acpi.h"
#include "../pc/pmtimer.h"
#include "../../core/memory/arena.h"

typedef struct { uintptr_t at; size_t size; } cs_uwin_span;

typedef struct {
    const cs_native_memory *memory;
    cs_uwin_span spans[3];
} cs_uwin_reader;

typedef struct {
    uint32_t phase, result, module, cause, ready, stage, current;
    uint32_t used, bits, port;
    uint64_t polls, ticks;
    const cs_pc_window_session *session;
} cs_uwin_state;

static int uwin_span(uintptr_t at, size_t size, size_t align)
{
    return at != 0 && size != 0 && size <= UINTPTR_MAX - at && at % align == 0;
}

static int uwin_overlap(const cs_uwin_span *a, const cs_uwin_span *b)
{
    return a->at < b->at + b->size && b->at < a->at + a->size;
}

static int uwin_translate(const cs_native_window_binding *b, uint64_t physical,
    uint64_t size, uintptr_t *out)
{
    uint64_t offset;
    if (size == 0 || physical < b->physical) return 0;
    offset = physical - b->physical;
    if (size > b->size || offset > b->size - size) return 0;
    *out = b->base + (uintptr_t)offset;
    return 1;
}

static int uwin_read(void *context, uint64_t physical, unsigned char *out, size_t length)
{
    cs_uwin_reader *r = (cs_uwin_reader *)context;
    const cs_native_memory *m = r->memory;
    uint64_t offset;
    uintptr_t at;
    size_t i;
    if (length != 0 && physical >= m->window_physical) {
        offset = physical - m->window_physical;
        if (length <= m->window_size && offset <= m->window_size - length) {
            at = m->window_base + (uintptr_t)offset;
            for (i = 0; i < 3; i++)
                if (at < r->spans[i].at + r->spans[i].size &&
                    r->spans[i].at < at + length)
                    return 0;
        }
    }
    return cs_native_read((void *)(uintptr_t)m, physical, out, length);
}

static void put32(unsigned char *t, size_t at, uint32_t v)
{
    t[at] = (unsigned char)(v & 0xFFu);
    t[at + 1] = (unsigned char)((v >> 8) & 0xFFu);
    t[at + 2] = (unsigned char)((v >> 16) & 0xFFu);
    t[at + 3] = (unsigned char)((v >> 24) & 0xFFu);
}

static void put64(unsigned char *t, size_t at, uint64_t v)
{
    put32(t, at, (uint32_t)(v & 0xFFFFFFFFu));
    put32(t, at + 4, (uint32_t)(v >> 32));
}

static void uwin_snapshot(unsigned char *t, const cs_uwin_state *w)
{
    const cs_pc_window_session *s = w->session;
    size_t i;
    put32(t, 0, (uint32_t)CS_UWIN_TRACE_MAGIC);
    put32(t, 4, (uint32_t)CS_UWIN_TRACE_VERSION);
    put32(t, 8, (uint32_t)CS_UWIN_TRACE_BYTES);
    put32(t, 12, w->phase);
    put32(t, 16, w->result);
    put32(t, 20, w->module);
    put32(t, 24, w->cause);
    put32(t, 28, w->ready);
    put32(t, 32, w->stage);
    if (s) {
        put32(t, 36, s->bus_result);
        put32(t, 40, s->phase);
        put32(t, 44, s->stage);
        put32(t, 48, s->result);
        put32(t, 52, (uint32_t)s->cause);
        put32(t, 56, s->delivery.kind);
        put32(t, 60, s->consumed);
        put32(t, 64, (uint32_t)s->owner.pointer.x);
        put32(t, 68, (uint32_t)s->owner.pointer.y);
        put32(t, 72, s->owner.pointer.down);
        put32(t, 76, s->owner.pointer.suppress);
        put32(t, 80, (uint32_t)s->owner.window.x);
        put32(t, 84, (uint32_t)s->owner.window.y);
        put32(t, 88, s->owner.window.dragging);
        put32(t, 92, s->owner.window.pattern);
        put32(t, 96, s->owner.product.inputs);
        put32(t, 100, s->ram_frames);
        put32(t, 104, s->published_frames);
        if (w->current) {
            put32(t, 108, s->presentation.phase);
            put32(t, 112, s->presentation.rows);
            put32(t, 116, s->presentation.required);
            put32(t, 120, (uint32_t)s->presentation.cause);
        } else {
            for (i = 108; i < 124; i++) t[i] = 0;
        }
        put32(t, 124, s->drained);
    } else {
        for (i = 36; i < 128; i++) t[i] = 0;
    }
    put64(t, 128, w->polls);
    put64(t, 136, w->ticks);
    put32(t, 144, w->used);
    put32(t, 148, w->bits);
    put32(t, 152, w->port);
    put32(t, 156, 0u);
}

static uint32_t uwin_finish(unsigned char *t, cs_uwin_state *w, uint32_t phase,
    uint32_t result, uint32_t module, uint32_t cause)
{
    w->phase = phase;
    w->result = result;
    w->module = module;
    w->cause = cause;
    uwin_snapshot(t, w);
    return result;
}

uint32_t cs_native_window_loop(const cs_uefi_handoff *h, uint32_t ready,
    const cs_native_window_binding *b, uint32_t seconds, unsigned char *trace)
{
    const cs_native_devices *d;
    const cs_native_memory *m;
    const cs_ps2_io *k;
    cs_uwin_span ro[6], rw[3];
    cs_uwin_reader reader;
    cs_uwin_state w;
    cs_uefi_framebuffer_layout layout;
    cs_acpi_pm_timer pm;
    cs_pmtimer timer;
    cs_arena arena;
    cs_arena_span span;
    cs_pc_window_config cfg;
    cs_pc_window_session *s;
    volatile unsigned char *zero;
    uintptr_t at;
    uint64_t bb;
    size_t i, j, count, head;
    uint32_t r, raw, delta, has8042, found;

    if (!h || !b || !trace || seconds < 1u || seconds > (uint32_t)CS_LOOP_MAX_SECONDS)
        return CS_UWIN_ARGUMENT;
    if (!uwin_span((uintptr_t)h, sizeof *h, _Alignof(cs_uefi_handoff)) ||
        !uwin_span((uintptr_t)b, sizeof *b, _Alignof(cs_native_window_binding)))
        return CS_UWIN_ARGUMENT;
    d = b->devices;
    k = b->keyboard;
    if (!uwin_span((uintptr_t)d, sizeof *d, _Alignof(cs_native_devices)) ||
        !uwin_span((uintptr_t)k, sizeof *k, _Alignof(cs_ps2_io)))
        return CS_UWIN_ARGUMENT;
    m = d->memory;
    if (!uwin_span((uintptr_t)m, sizeof *m, _Alignof(cs_native_memory)))
        return CS_UWIN_ARGUMENT;
    if (!d->port || !k->read || !k->write || !d->framebuffer || !d->arena ||
        !m->map || m->map_size == 0)
        return CS_UWIN_ARGUMENT;
    if (b->size == 0 || b->size > UINT64_MAX - b->physical ||
        (uintptr_t)b->size > UINTPTR_MAX - b->base)
        return CS_UWIN_ARGUMENT;
    if (h->arena_size != (uint64_t)CS_LOADER_ARENA_BYTES ||
        h->trace_size != (uint64_t)CS_LOADER_TRACE_BYTES || h->framebuffer.size == 0)
        return CS_UWIN_ARGUMENT;
    if (!uwin_translate(b, h->bundle_base, (uint64_t)CS_LOADER_BUNDLE, &at) ||
        at != (uintptr_t)h)
        return CS_UWIN_ARGUMENT;
    if (!uwin_translate(b, h->arena_base, (uint64_t)CS_LOADER_ARENA_BYTES, &at) ||
        at != (uintptr_t)d->arena)
        return CS_UWIN_ARGUMENT;
    rw[0].at = at; rw[0].size = (size_t)CS_LOADER_ARENA_BYTES;
    if (!uwin_translate(b, h->trace_base, (uint64_t)CS_LOADER_TRACE_BYTES, &at) ||
        at + (uintptr_t)CS_UWIN_TRACE_OFFSET != (uintptr_t)trace)
        return CS_UWIN_ARGUMENT;
    rw[1].at = at; rw[1].size = (size_t)CS_LOADER_TRACE_BYTES;
    if (!uwin_translate(b, h->framebuffer.base, h->framebuffer.size, &at) ||
        at != (uintptr_t)d->framebuffer)
        return CS_UWIN_ARGUMENT;
    rw[2].at = at; rw[2].size = (size_t)h->framebuffer.size;
    if (!uwin_translate(b, h->map_base, (uint64_t)m->map_size, &at) ||
        at != (uintptr_t)m->map)
        return CS_UWIN_ARGUMENT;
    ro[0].at = (uintptr_t)h; ro[0].size = sizeof *h;
    ro[1].at = (uintptr_t)b; ro[1].size = sizeof *b;
    ro[2].at = (uintptr_t)d; ro[2].size = sizeof *d;
    ro[3].at = (uintptr_t)m; ro[3].size = sizeof *m;
    ro[4].at = (uintptr_t)k; ro[4].size = sizeof *k;
    ro[5].at = at; ro[5].size = m->map_size;
    for (i = 0; i < 3; i++) {
        if (!uwin_span(rw[i].at, rw[i].size, i == 0 ? CS_ARENA_MAX_ALIGNMENT : 1u))
            return CS_UWIN_ARGUMENT;
        for (j = 0; j < 6; j++)
            if (uwin_overlap(&rw[i], &ro[j])) return CS_UWIN_ARGUMENT;
        for (j = i + 1; j < 3; j++)
            if (uwin_overlap(&rw[i], &rw[j])) return CS_UWIN_ARGUMENT;
    }

    w.phase = 0; w.result = 0; w.module = 0; w.cause = 0;
    w.ready = ready; w.stage = h->stage; w.current = 0;
    w.used = 0; w.bits = 0; w.port = 0;
    w.polls = 0; w.ticks = 0; w.session = NULL;
    bb = h->bundle_base;
    if (h->magic != UINT64_C(0x4353303937484F46) ||
        h->version != (uint32_t)CS_UEFI_HANDOFF_VERSION || h->size != sizeof *h ||
        h->bundle_size != (uint64_t)CS_LOADER_BUNDLE ||
        bb > UINT64_MAX - (uint64_t)CS_LOADER_BUNDLE || h->image_size == 0 ||
        h->image_base == 0 || h->image_base >= (UINT64_C(1) << 47) ||
        h->image_size > (UINT64_C(1) << 47) - h->image_base ||
        h->map_base != bb + (uint64_t)CS_LOADER_MAP_OFFSET ||
        h->map_size > (uint64_t)CS_LOADER_MAP_BYTES ||
        h->map_size != (uint64_t)m->map_size ||
        h->map_stride != (uint64_t)m->map_stride || h->map_version != m->map_version ||
        h->arena_base != bb + (uint64_t)CS_LOADER_ARENA_OFFSET ||
        h->trace_base != bb + (uint64_t)CS_LOADER_TRACE_OFFSET ||
        h->stack_top != bb + (uint64_t)CS_LOADER_FAULT_OFFSET ||
        h->fault_top != bb + (uint64_t)CS_LOADER_ARENA_OFFSET)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_HANDOFF, CS_UWIN_GATE_MODULE, 0);
    if (h->stage != (uint32_t)CS_UEFI_EXITED || h->firmware_status != 0)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_NOT_EXITED, CS_UWIN_GATE_MODULE, 0);
    if (ready != 1u)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_NOT_READY, CS_UWIN_GATE_MODULE, 0);
    r = (uint32_t)cs_uefi_check_map(m->map, m->map_size, m->map_stride, m->map_version, &count);
    if (r != (uint32_t)CS_UEFI_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_MAP, CS_UWIN_GATE_MODULE, r);
    if (h->span_count < 1u || h->span_count > 8u)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_MAP, CS_UWIN_GATE_MODULE, 0);
    found = 0;
    for (i = 0; i < h->span_count; i++)
        if (h->spans[i].kind == 2u && h->spans[i].base == bb &&
            h->spans[i].size == (uint64_t)CS_LOADER_BUNDLE)
            found = 1;
    if (!found)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_MAP, CS_UWIN_GATE_MODULE, 0);
    r = (uint32_t)cs_uefi_check_framebuffer(&h->framebuffer, &layout);
    if (r != (uint32_t)CS_UEFI_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_TARGET, CS_UWIN_GATE_MODULE, r);
    if (h->framebuffer.width < 512u || h->framebuffer.height < 342u)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_TARGET, CS_UWIN_GATE_MODULE, 0);
    r = (uint32_t)cs_uefi_check_owned(m->map, m->map_size, m->map_stride, m->map_version,
        h->spans, (size_t)h->span_count, &h->framebuffer);
    if (r != (uint32_t)CS_UEFI_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_MAP, CS_UWIN_GATE_MODULE, r);

    /* The declared image must be covered by admitted non-bundle owned spans. */
    {
        uint64_t cursor = h->image_base, end = cursor + h->image_size;
        for (i = 0; i < h->span_count && cursor < end; ++i) {
            uint64_t next = cursor;
            for (j = 0; j < h->span_count; ++j) {
                const cs_uefi_owned_span *owned = &h->spans[j];
                uint64_t stop = owned->base + owned->size;
                if (owned->base != bb && owned->base <= cursor && stop > next)
                    next = stop;
            }
            if (next == cursor) break;
            cursor = next;
        }
        if (cursor < end)
            return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_MAP, CS_UWIN_GATE_MODULE, 0);
    }


    if (m->window_size == 0 || m->window_size > UINT64_MAX - m->window_physical ||
        (uintptr_t)m->window_size > UINTPTR_MAX - m->window_base)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_TIMER, CS_UWIN_GATE_MODULE, 0);
    reader.memory = m;
    for (i = 0; i < 3; i++) {
        reader.spans[i].at = rw[i].at;
        reader.spans[i].size = rw[i].size;
    }
    r = (uint32_t)cs_acpi_find_pm_timer(uwin_read, &reader, h->rsdp, &pm);
    if (r != (uint32_t)CS_ACPI_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_TIMER, CS_UWIN_GATE_MODULE, r);
    w.bits = pm.bits;
    w.port = pm.port;
    has8042 = 0;
    r = (uint32_t)cs_acpi_8042(uwin_read, &reader, pm.fadt, &has8042);
    if (r != (uint32_t)CS_ACPI_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_NO_8042, CS_UWIN_GATE_MODULE, r);
    if (has8042 != 1u)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_NO_8042, CS_UWIN_GATE_MODULE, 0);
    raw = d->port(d->port_context, pm.port);
    r = (uint32_t)cs_pmtimer_init(&timer, pm.bits, raw);
    if (r != (uint32_t)CS_PMTIMER_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_CLOCK, CS_UWIN_CLOCK_MODULE, r);

    r = (uint32_t)cs_arena_init(&arena, d->arena, (size_t)CS_LOADER_ARENA_BYTES);
    if (r != (uint32_t)CS_ARENA_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_ARENA, CS_UWIN_GATE_MODULE, r);
    head = (sizeof(cs_pc_window_session) + CS_ARENA_MAX_ALIGNMENT - 1u) &
        ~(CS_ARENA_MAX_ALIGNMENT - 1u);
    r = (uint32_t)cs_arena_alloc(&arena, head + (size_t)CS_PWS_CANVAS + (size_t)CS_PWS_PAYLOAD +
        (size_t)4u * (size_t)h->framebuffer.width, CS_ARENA_MAX_ALIGNMENT, &span);
    if (r != (uint32_t)CS_ARENA_OK)
        return uwin_finish(trace, &w, CS_UWIN_GATE, CS_UWIN_ARENA, CS_UWIN_GATE_MODULE, r);
    w.used = (uint32_t)arena.used;
    s = (cs_pc_window_session *)(void *)span.data;
    zero = span.data;
    for (i = 0; i < sizeof *s; i++) zero[i] = 0;

    cfg.target.base = d->framebuffer;
    cfg.target.size = (size_t)h->framebuffer.size;
    cfg.target.width = h->framebuffer.width;
    cfg.target.height = h->framebuffer.height;
    cfg.target.pitch = h->framebuffer.pitch;
    cfg.target.format = h->framebuffer.format;
    cfg.io.read = k->read;
    cfg.io.write = k->write;
    cfg.io.context = k->context;
    cfg.canvas = span.data + head;
    cfg.canvas_capacity = (size_t)CS_PWS_CANVAS;
    cfg.payload = cfg.canvas + (size_t)CS_PWS_CANVAS;
    cfg.payload_capacity = (size_t)CS_PWS_PAYLOAD;
    cfg.row = cfg.payload + (size_t)CS_PWS_PAYLOAD;
    cfg.row_capacity = (size_t)4u * (size_t)h->framebuffer.width;
    r = cs_pc_window_session_start(s, &cfg);
    w.session = s;
    w.current = (s->stage == (uint32_t)CS_PWS_PUBLISH || s->stage == (uint32_t)CS_PWS_BUS) ? 1u : 0u;
    if (r != (uint32_t)CS_PWS_OK)
        return uwin_finish(trace, &w, CS_UWIN_FAULT, CS_UWIN_SESSION, CS_UWIN_SESSION_MODULE, r);

    for (;;) {
        (void)uwin_finish(trace, &w, CS_UWIN_ACTIVE, CS_UWIN_RUNNING, CS_UWIN_SESSION_MODULE, 0);
        if (b->observer && b->observer(b->context, trace, s) != 0) {
            r = cs_pc_window_session_stop(s);
            return uwin_finish(trace, &w, CS_UWIN_DONE, CS_UWIN_STOPPED, CS_UWIN_STOP_MODULE, r);
        }
        s->delivery.kind = (uint32_t)CS_PS2_DELIVERY_NONE;
        s->delivery.packet.dx = 0;
        s->delivery.packet.dy = 0;
        s->delivery.packet.buttons = 0;
        s->consumed = 0;
        s->drained = 0;
        w.current = 0;
        raw = d->port(d->port_context, pm.port);
        if (w.polls != UINT64_MAX) w.polls++;
        r = (uint32_t)cs_pmtimer_sample(&timer, raw, &delta);
        w.ticks = timer.ticks;
        if (r != (uint32_t)CS_PMTIMER_OK) {
            (void)cs_pc_window_session_poll(s, timer.ticks, 0u);
            return uwin_finish(trace, &w, CS_UWIN_FAULT, CS_UWIN_CLOCK, CS_UWIN_CLOCK_MODULE, r);
        }
        r = cs_pc_window_session_poll(s, timer.ticks, 1u);
        w.current = s->stage == (uint32_t)CS_PWS_PUBLISH ? 1u : 0u;
        if (r != (uint32_t)CS_PWS_OK)
            return uwin_finish(trace, &w, CS_UWIN_FAULT,
                r == (uint32_t)CS_PWS_CLOCK ? (uint32_t)CS_UWIN_CLOCK : (uint32_t)CS_UWIN_SESSION,
                r == (uint32_t)CS_PWS_CLOCK ? (uint32_t)CS_UWIN_CLOCK_MODULE : (uint32_t)CS_UWIN_SESSION_MODULE, r);
    }
}
