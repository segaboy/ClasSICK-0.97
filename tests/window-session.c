/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../platform/pc/window-session.h"

enum { W = 520, H = 350 };
static _Alignas(cs_pc_window_session) unsigned char fb[W * H * 4];
static uint8_t canvas[CS_PWS_CANVAS + 4096], payload[64], row[W * 4];
static cs_pc_window_session ses;
static cs_pc_window_config cfg;
static unsigned reads, writes, checks, cases, fails, frames;
static uint8_t wire[3], wire_status;
static size_t wire_at, wire_count;
static uint32_t start_session(cs_pc_window_session *st, const cs_pc_window_config *c)
{
    uint32_t result = cs_pc_window_session_start(st, c);
    if (result == CS_PWS_OK) ++frames;
    return result;
}

#define CHECK(c) do { ++checks; if (!(c)) { ++fails; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static uint8_t port_read(void *context, uint16_t port)
{
    CHECK(context == &reads); ++reads;
    if (port == 0x64u) return wire_at < wire_count ? wire_status : 0u;
    CHECK(port == 0x60u && wire_at < wire_count);
    return wire_at < wire_count ? wire[wire_at++] : 0u;
}
static void port_write(void *context, uint16_t port, uint8_t value)
{ CHECK(context == &reads); (void)port; (void)value; ++writes; }

#ifdef CS_PWS_FAULTS
static void freset(void);
#endif
static void setup(void)
{
    if (ses.magic == CS_PWS_MAGIC) {
        cs_cursor_info info; cs_rect old;
        (void)cs_pc_window_session_stop(&ses);
        if (cs_cursor_query(&ses.cursor, &info) == CS_CURSOR_OK && info.phase != CS_CURSOR_UNBOUND)
            CHECK(cs_cursor_abandon(&ses.cursor, &old) == CS_CURSOR_OK);
    }
    wire_at = wire_count = 0u; wire_status = 0u;
    memset(fb, 0xA5, sizeof fb); memset(canvas, 0xA5, sizeof canvas);
    memset(payload, 0xA5, sizeof payload); memset(row, 0xA5, sizeof row);
    memset(&ses, 0x5A, sizeof ses); memset(&cfg, 0, sizeof cfg);
    cfg.target.base = fb; cfg.target.size = sizeof fb; cfg.target.width = W;
    cfg.target.height = H; cfg.target.pitch = W; cfg.target.format = CS_FB_BGR_RESERVED;
    cfg.io.read = port_read; cfg.io.write = port_write; cfg.io.context = &reads;
    cfg.canvas = canvas; cfg.canvas_capacity = sizeof canvas;
    cfg.payload = payload; cfg.payload_capacity = sizeof payload;
    cfg.row = row; cfg.row_capacity = sizeof row;
    reads = writes = 0u;
#ifdef CS_PWS_FAULTS
    freset();
#endif
}

static void published_pixels(void)
{
    int correct=1;
    for(unsigned y=0;y<H;++y)for(unsigned x=0;x<W;++x){
        uint8_t v=255;
        if(x>=4u && x<516u && y>=4u && y<346u){
            unsigned sx=x-4u,sy=y-4u;
            if(((uint32_t)canvas[sy*64u+sx/8u]>>(7u-sx%8u)&1u)!=0u)v=0;
        }
        size_t n=((size_t)y*W+x)*4u;
        if(fb[n]!=v || fb[n+1]!=v || fb[n+2]!=v || fb[n+3]!=0)correct=0;
    }
    CHECK(correct);
}

static int same(const void *p, unsigned char v, size_t n)
{
    const unsigned char *b = p; size_t i;
    for (i = 0u; i < n; ++i) if (b[i] != v) return 0;
    return 1;
}
static int pristine(void)
{
    return same(fb, 0xA5, sizeof fb) && same(canvas, 0xA5, sizeof canvas)
        && same(payload, 0xA5, sizeof payload) && same(row, 0xA5, sizeof row)
        && same(&ses, 0x5A, sizeof ses) && reads == 0u && writes == 0u;
}
static void refuse(cs_pc_window_session *s, const cs_pc_window_config *c)
{
    cs_pc_window_config before = cfg;
    ++cases;
    CHECK(start_session(s, c) == CS_PWS_ARGUMENT);
    CHECK(pristine());
    CHECK(before.canvas == cfg.canvas && before.canvas_capacity == cfg.canvas_capacity
        && before.row == cfg.row && before.target.size == cfg.target.size);
    setup();
}

static void test_preflight(void)
{
    uintptr_t mask = (uintptr_t)_Alignof(cs_pc_window_session) - 1u;
    setup();
    refuse(NULL, &cfg); refuse(&ses, NULL);
    refuse((cs_pc_window_session *)((uintptr_t)&ses + 1u), &cfg);
    refuse((cs_pc_window_session *)((UINTPTR_MAX - sizeof ses / 2u) & ~mask), &cfg);
    refuse(&ses, (const cs_pc_window_config *)(const void *)&ses);
    refuse((cs_pc_window_session *)(void *)fb, &cfg);
    cfg.io.read = NULL; refuse(&ses, &cfg);
    cfg.io.write = NULL; refuse(&ses, &cfg);
    cfg.canvas = NULL; refuse(&ses, &cfg);
    cfg.payload = NULL; refuse(&ses, &cfg);
    cfg.row = NULL; refuse(&ses, &cfg);
    cfg.canvas_capacity = CS_PWS_CANVAS - 1; refuse(&ses, &cfg);
    cfg.payload_capacity = CS_PWS_PAYLOAD - 1; refuse(&ses, &cfg);
#if SIZE_MAX > UINT32_MAX
    cfg.payload_capacity = (size_t)UINT32_MAX + 1u; refuse(&ses, &cfg);
#endif
    cfg.row_capacity = W * 4 - 1; refuse(&ses, &cfg);
    cfg.target.width = 511; refuse(&ses, &cfg);
    cfg.target.height = 341; refuse(&ses, &cfg);
    cfg.target.format = 9; refuse(&ses, &cfg);
    cfg.target.size = sizeof fb - 1u; refuse(&ses, &cfg);
    cfg.canvas = (uint8_t *)(UINTPTR_MAX - 100u); refuse(&ses, &cfg);
    cfg.canvas = (uint8_t *)(UINTPTR_MAX - cfg.canvas_capacity + 1u); refuse(&ses, &cfg);
    cfg.payload = canvas + 100; refuse(&ses, &cfg);
    cfg.row = canvas + CS_PWS_CANVAS + 16; refuse(&ses, &cfg);
    cfg.canvas = fb; refuse(&ses, &cfg);
    cfg.payload = (uint8_t *)&ses + 8; refuse(&ses, &cfg);
}

static void test_start(void)
{
    setup(); ++cases;
    CHECK(start_session(&ses, &cfg) == CS_PWS_OK);
    CHECK(reads == 0u && writes == 0u);
    CHECK(ses.magic == CS_PWS_MAGIC && ses.phase == CS_PWS_RUNNING && ses.result == CS_PWS_OK);
    CHECK(ses.stage == CS_PWS_BUS && ses.cause == 0 && ses.delivery.kind == CS_PS2_DELIVERY_NONE);
    CHECK(ses.ram_frames == 1u && ses.published_frames == 1u);
    CHECK(ses.presentation.phase == CS_MONO_COMPLETE && ses.presentation.rows == H);
    CHECK(fb[0] == 0xFF && fb[1] == 0xFF && fb[2] == 0xFF && fb[3] == 0u);
    CHECK(!same(canvas, 0xA5, CS_PWS_CANVAS));
    CHECK(same(canvas + CS_PWS_CANVAS, 0xA5, sizeof canvas - CS_PWS_CANVAS));
    CHECK(ses.owner.window.x == 80 && ses.owner.window.y == 70);
    CHECK(ses.owner.window.width == 224 && ses.owner.window.height == 144);
    CHECK(ses.owner.product.value == 97u && ses.owner.product.progress == 16u
        && ses.owner.product.phases == 6u && ses.owner.product.pixels == NULL);
    CHECK(ses.queue.capacity == 8u && ses.queue.storage == ses.records);
    for (unsigned i = 0u; i < 32u; ++i) {
        uint8_t bit = (uint8_t)((i & 1u) || i >= 14u ? 0u : i == 6u ? 0xFEu : 0x10u);
        CHECK(ses.bits.mask[i] == bit && ses.bits.image[i] == bit);
    }
}

static void test_clock_stop(void)
{
    unsigned io; uint32_t r;
    unsigned char snapshot[sizeof ses];
    setup(); ++cases;
    CHECK(start_session(&ses, &cfg) == CS_PWS_OK);
    ses.delivery.kind = CS_PS2_DELIVERY_PACKET;
    ses.delivery.packet.dx = 17;
    CHECK(cs_pc_window_session_poll(&ses, 5u, 0u) == CS_PWS_CLOCK && reads == 0u && writes == 0u);
    CHECK(ses.phase == CS_PWS_FAULTED && ses.stage == CS_PWS_TIME && ses.ram_frames == 1u);
    CHECK(ses.delivery.kind == CS_PS2_DELIVERY_NONE && ses.delivery.packet.dx == 0);
    memcpy(snapshot, &ses, sizeof ses);
    CHECK(cs_pc_window_session_poll(&ses, 6u, 1u) == CS_PWS_CLOCK);
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_CLOCK && reads == 0u && writes == 0u);
    CHECK(memcmp(snapshot, &ses, sizeof ses) == 0);
    setup(); ++cases;
    CHECK(start_session(&ses, &cfg) == CS_PWS_OK);
    r = cs_pc_window_session_poll(&ses, 10u, 1u);
    CHECK(r == CS_PWS_OK);
    CHECK(ses.delivery.kind == CS_PS2_DELIVERY_NONE && ses.consumed == 0u && ses.published_frames == 1u);
    if (r == CS_PWS_OK) {
        io = reads + writes;
        CHECK(cs_pc_window_session_poll(&ses, 9u, 1u) == CS_PWS_CLOCK && reads + writes == io);
    }
    setup(); ++cases;
    CHECK(start_session(&ses, &cfg) == CS_PWS_OK);
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_OK && ses.phase == CS_PWS_STOPPED);
    memcpy(snapshot, &ses, sizeof ses);
    io = reads + writes;
    CHECK(cs_pc_window_session_poll(&ses, 1u, 1u) == CS_PWS_OK);
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_OK && reads + writes == io);
    CHECK(memcmp(snapshot, &ses, sizeof ses) == 0);
    CHECK(ses.published_frames == 1u && ses.result == CS_PWS_OK);
    CHECK(cs_pc_window_session_poll(NULL, 1u, 1u) == CS_PWS_ARGUMENT);
    CHECK(cs_pc_window_session_stop(NULL) == CS_PWS_ARGUMENT);
    ses.phase = 99u;
    CHECK(cs_pc_window_session_poll(&ses, 2u, 1u) == CS_PWS_STATE);
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_STATE && ses.phase == 99u);
    ses.magic = 0u;
    CHECK(cs_pc_window_session_poll(&ses, 3u, 1u) == CS_PWS_ARGUMENT);
}

#ifndef CS_PWS_FAULTS
/* Exercise real controller receipt -> real owner -> real row presenter.
 * Initialization handshake is covered by existing bus tests; install its READY
 * state here, then deliver individual controller bytes through callbacks. */
static uint64_t realtime;
static void real_bytes(uint8_t status, uint8_t a, uint8_t b, uint8_t c, size_t n)
{
    uint32_t before = ses.published_frames;
    wire[0] = a; wire[1] = b; wire[2] = c;
    wire_status = status; wire_at = 0u; wire_count = n;
    for (size_t i = 0u; i < n; ++i) {
        CHECK(cs_pc_window_session_poll(&ses, ++realtime, 1u) == CS_PWS_OK);
        if (n == 3u && i < 2u) CHECK(ses.published_frames == before);
    }
    CHECK(wire_at == n);
    published_pixels();
}
static void test_real_reports(void)
{
    uint32_t before; ++cases; setup();
    CHECK(start_session(&ses, &cfg) == CS_PWS_OK);
    ses.bus.phase = CS_PS2_BUS_READY; ses.bus.result = CS_PS2_READY;
    ses.bus.keyboard.phase = CS_PS2_PHASE_READY; ses.bus.keyboard.result = CS_PS2_READY;
    real_bytes(0x21u, 0x09u, 0u, 0u, 3u); /* unknown first down suppressed */
    CHECK(ses.owner.product.inputs == 0u && ses.owner.pointer.suppress == 1u);
    before=ses.published_frames;
    unsigned char saved_report[sizeof ses.presentation];
    memcpy(saved_report,&ses.presentation,sizeof saved_report);
    real_bytes(0x21u, 0x08u, 0u, 0u, 3u);
    CHECK(ses.owner.pointer.suppress == 0u);
    CHECK(ses.published_frames==before && memcmp(saved_report,&ses.presentation,sizeof saved_report)==0);
    real_bytes(0x21u, 0x18u, 120u, 93u, 3u); /* -136,+93 -> (120,78) */
    CHECK(ses.owner.pointer.x == 120 && ses.owner.pointer.y == 78);
    real_bytes(0x21u, 0x09u, 0u, 0u, 3u);
    CHECK(ses.owner.window.dragging == 1u && ses.owner.product.inputs == 1u);
    real_bytes(0x21u, 0x29u, 20u, 246u, 3u);
    CHECK(ses.owner.window.x == 100 && ses.owner.window.y == 80);
    before = ses.published_frames;
    real_bytes(0x01u, 0x29u, 0u, 0u, 1u);
    CHECK(ses.drained == 1u);
    real_bytes(0x01u, 0xF0u, 0u, 0u, 1u);
    real_bytes(0x01u, 0x29u, 0u, 0u, 1u);
    CHECK(ses.drained == 1u);
    real_bytes(0x01u, 0x76u, 0u, 0u, 1u);
    CHECK(ses.drained == 1u);
    real_bytes(0x01u, 0xF0u, 0u, 0u, 1u);
    real_bytes(0x01u, 0x76u, 0u, 0u, 1u);
    CHECK(ses.drained == 1u);
    CHECK(ses.published_frames == before && ses.owner.product.inputs == 1u);
    real_bytes(0xE1u, 0x08u, 0u, 0u, 1u);
    CHECK(ses.delivery.kind == CS_PS2_DELIVERY_LOST && ses.consumed == 1u);
    CHECK(ses.owner.pointer.suppress == 1u && ses.owner.window.dragging == 0u);
    CHECK(ses.published_frames == before); /* Cancel changes no visible pixels. */
    frames += ses.published_frames - 1u;
}
#endif

#ifdef CS_PWS_FAULTS
uint32_t cs_test_bus_poll(cs_ps2_bus *, const cs_ps2_io *, uint64_t, cs_input_queue *, cs_ps2_bus_delivery *);
cs_input_result cs_test_input_pop(cs_input_queue *, cs_input_record *);
int cs_test_packet(cs_window_input *, const cs_ps2_mouse_packet *);
int cs_test_cancel(cs_window_input *);
int cs_test_present(const cs_fb_target *, const cs_surface *, uint8_t *, size_t, uint32_t, cs_pc_mono_report *);
int cs_test_begin(cs_window_input *, cs_cursor_state *, cs_cursor_token, const cs_cursor_art *,
    const cs_window_config *, const cs_product_state *, uint32_t);
uint32_t cs_test_bus_begin(cs_ps2_bus *);

static struct {
    int script, pop_mode, packet_r, cancel_r, present_r, begin_r, bus_begin_r;
    uint32_t bus_r, kind; cs_ps2_mouse_packet pk;
    unsigned pops, packets, cancels, presents, frames;
} F;
static char trail[32];
static size_t trail_n;
static uint64_t tick = 1u;
static void note(char c) { if (trail_n + 1u < sizeof trail) { trail[trail_n++] = c; trail[trail_n] = 0; } }
static void untrail(void) { trail_n = 0u; trail[0] = 0; }
static void freset(void)
{
    unsigned total = F.frames;
    memset(&F, 0, sizeof F); F.frames = total;
    F.packet_r = F.cancel_r = F.present_r = F.begin_r = F.bus_begin_r = -1;
    untrail();
}

uint32_t cs_test_bus_poll(cs_ps2_bus *bus, const cs_ps2_io *io, uint64_t now,
    cs_input_queue *queue, cs_ps2_bus_delivery *d)
{
    note('Q');
    CHECK(bus == &ses.bus && io == &ses.io && queue == &ses.queue && d == &ses.delivery);
    CHECK(d->kind == CS_PS2_DELIVERY_NONE && d->packet.dx == 0 && d->packet.dy == 0 && d->packet.buttons == 0u);
    CHECK(ses.consumed == 0u && ses.drained == 0u && ses.stage == CS_PWS_POLL);
    if (!F.script) return cs_ps2_bus_poll_delivery(bus, io, now, queue, d);
    if (F.kind != CS_PS2_DELIVERY_NONE) { d->kind = F.kind; d->packet = F.pk; }
    return F.bus_r;
}
cs_input_result cs_test_input_pop(cs_input_queue *queue, cs_input_record *out)
{
    note('K'); ++F.pops;
    CHECK(queue == &ses.queue && out != NULL && ses.stage == CS_PWS_DRAIN);
    if (F.pop_mode == 0) return cs_input_pop(queue, out);
    if (F.pop_mode == 3) return CS_INPUT_ERR_STATE;
    if (F.pop_mode == 1 && F.pops > 2u) return CS_INPUT_ERR_EMPTY;
    out->event.source = 1u; out->event.action = (uint32_t)CS_KEY_PRESS; out->event.repeat = 0u;
    out->event.key = F.pops == 1u ? (uint32_t)CS_KEY_SPACE : (uint32_t)CS_KEY_ESCAPE;
    out->sequence = F.pops;
    return CS_INPUT_OK;
}
int cs_test_packet(cs_window_input *owner, const cs_ps2_mouse_packet *p)
{
    note('M'); ++F.packets;
    CHECK(owner == &ses.owner && p != NULL && ses.consumed == 1u && ses.stage == CS_PWS_DISPATCH);
    CHECK(p->dx == F.pk.dx && p->dy == F.pk.dy && p->buttons == F.pk.buttons);
    return F.packet_r >= 0 ? F.packet_r : cs_pc_window_packet(owner, p);
}
int cs_test_cancel(cs_window_input *owner)
{
    note('C'); ++F.cancels;
    CHECK(owner == &ses.owner);
    return F.cancel_r >= 0 ? F.cancel_r : cs_window_input_cancel(owner);
}
int cs_test_present(const cs_fb_target *t, const cs_surface *c, uint8_t *r, size_t cap,
    uint32_t initial, cs_pc_mono_report *out)
{
    int result;
    note(initial == 1u ? 'I' : 'P'); ++F.presents;
    CHECK(t == &ses.target && c == &ses.canvas && r == row && cap == sizeof row && out == &ses.presentation);
    CHECK(initial == (F.presents == 1u ? 1u : 0u));
    if (F.present_r >= 0) {
        fb[0] = 0x42u; /* injected device prefix must survive session refusal */
        out->phase = CS_MONO_FAILED; out->rows = 5u; out->required = 342u; out->cause = CS_FB_STORAGE;
        return F.present_r;
    }
    result = cs_pc_mono_present(t, c, r, cap, initial, out);
    if (result == CS_MONO_OK) ++F.frames;
    return result;
}
int cs_test_present_rects(const cs_fb_target *t,const cs_surface *c,uint8_t *r,size_t cap,
    const cs_rect *rects,size_t count,cs_pc_mono_report *out)
{
    note('P');++F.presents;
    CHECK(t==&ses.target && c==&ses.canvas && r==row && cap==sizeof row && out==&ses.presentation);
    CHECK(rects!=NULL && count>0 && count<=3);
    if(F.present_r>=0){
        fb[0]=0x42u;out->phase=CS_MONO_FAILED;out->rows=5;out->required=342;out->cause=CS_FB_STORAGE;
        return F.present_r;
    }
    int result=cs_pc_mono_present_rects(t,c,r,cap,rects,count,out);
    if(result==CS_MONO_OK)++F.frames;
    return result;
}
int cs_test_begin(cs_window_input *owner, cs_cursor_state *cursor, cs_cursor_token token,
    const cs_cursor_art *art, const cs_window_config *w, const cs_product_state *p, uint32_t buttons)
{
    cs_cursor_info info;
    note('B');
    CHECK(owner == &ses.owner && cursor == &ses.cursor && art == &ses.art && buttons == 0u);
    CHECK(cs_cursor_query(cursor, &info) == CS_CURSOR_OK && info.phase == CS_CURSOR_HIDDEN);
    CHECK(info.token.binding == token.binding && info.token.revision == token.revision);
    CHECK(art->width == 7u && art->height == 7u && art->hot_x == 3u && art->hot_y == 3u && art->bits == &ses.bits);
    CHECK(w->x == 80 && w->y == 70 && w->width == 224 && w->height == 144 && w->title == 18);
    CHECK(p->value == 97u && p->inputs == 0u && p->progress == 16u && p->phases == 6u
        && p->success == 1u && p->pixels == NULL);
    CHECK(ses.queue.capacity == 8u && ses.queue.storage == ses.records && F.cancels == 0u);
    return F.begin_r >= 0 ? F.begin_r : cs_window_input_begin(owner, cursor, token, art, w, p, buttons);
}
uint32_t cs_test_bus_begin(cs_ps2_bus *bus)
{
    note('U');
    CHECK(bus == &ses.bus && ses.published_frames == 1u && reads == 0u && writes == 0u);
    return F.bus_begin_r >= 0 ? (uint32_t)F.bus_begin_r : cs_ps2_bus_begin(bus);
}

static uint32_t step(uint32_t kind, int32_t dx, int32_t dy, uint32_t buttons)
{
    unsigned before = F.presents; uint32_t ram = ses.ram_frames, r;
    F.script = 1; F.bus_r = CS_PS2_READY; F.kind = kind;
    F.pk.dx = dx; F.pk.dy = dy; F.pk.buttons = buttons;
    r = cs_pc_window_session_poll(&ses, ++tick, 1u);
    if (r == CS_PWS_OK && kind != CS_PS2_DELIVERY_NONE) {
        CHECK(F.presents>=before && F.presents<=before+1u && ses.consumed==1u);
        CHECK(ses.stage==(F.presents==before?CS_PWS_DISPATCH:CS_PWS_PUBLISH));
        if(F.present_r<0)published_pixels();
        CHECK(ses.ram_frames == (ram == UINT32_MAX ? ram : ram + 1u));
    }
    return r;
}
static void go(int32_t x, int32_t y, uint32_t buttons)
{
    int n;
    for (n = 0; n < 16 && (ses.owner.pointer.x != x || ses.owner.pointer.y != y); ++n) {
        int32_t dx = x - ses.owner.pointer.x, dy = ses.owner.pointer.y - y;
        dx = dx > 100 ? 100 : dx < -100 ? -100 : dx;
        dy = dy > 100 ? 100 : dy < -100 ? -100 : dy;
        CHECK(step(CS_PS2_DELIVERY_PACKET, dx, dy, buttons) == CS_PWS_OK);
    }
    CHECK(ses.owner.pointer.x == x && ses.owner.pointer.y == y);
}
static void fstart(void)
{
    setup(); ++cases;
    CHECK(start_session(&ses, &cfg) == CS_PWS_OK);
    CHECK(strcmp(trail, "BCIU") == 0 && F.presents == 1u && F.cancels == 1u);
    untrail();
}
static void fstartup(int which, uint32_t result, uint32_t stage, int32_t cause, const char *expect)
{
    setup(); ++cases;
    if (which == 0) F.begin_r = CS_WINDOW_INPUT_STATE;
    if (which == 1) F.cancel_r = CS_WINDOW_INPUT_FAULT;
    if (which == 2) F.present_r = CS_MONO_PRESENT;
    if (which == 3) F.bus_begin_r = CS_PS2_STATE;
    CHECK(start_session(&ses, &cfg) == result && strcmp(trail, expect) == 0);
    CHECK(ses.phase == CS_PWS_FAULTED && ses.stage == stage && ses.cause == cause);
    CHECK(ses.ram_frames == (uint32_t)(which >= 2) && ses.published_frames == (uint32_t)(which == 3));
    untrail();
    CHECK(cs_pc_window_session_poll(&ses, 1u, 1u) == result);
    CHECK(cs_pc_window_session_stop(&ses) == result && trail_n == 0u && reads == 0u && writes == 0u);
}

static void test_faults(void)
{
    uint32_t pattern; int32_t wx; unsigned i;
    fstartup(0, CS_PWS_STARTUP, CS_PWS_BEGIN, CS_WINDOW_INPUT_STATE, "B");
    fstartup(1, CS_PWS_STARTUP, CS_PWS_CANCEL, CS_WINDOW_INPUT_FAULT, "BC");
    fstartup(2, CS_PWS_PRESENT, CS_PWS_PUBLISH, CS_MONO_PRESENT, "BCI");
    fstartup(3, CS_PWS_STARTUP, CS_PWS_BUS, CS_PS2_STATE, "BCIU");
    fstart(); /* stale receipt cleared; NONE never dispatches or publishes */
    ses.delivery.kind = CS_PS2_DELIVERY_PACKET; ses.delivery.packet.dx = 77;
    CHECK(step(CS_PS2_DELIVERY_NONE, 0, 0, 0u) == CS_PWS_OK && strcmp(trail, "QK") == 0);
    CHECK(ses.delivery.kind == CS_PS2_DELIVERY_NONE && ses.delivery.packet.dx == 0 && ses.consumed == 0u);
    CHECK(ses.stage == CS_PWS_DRAIN && ses.published_frames == 1u && F.presents == 1u);
    untrail(); /* held LEFT after start is ignored until a real release */
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 1u) == CS_PWS_OK && strcmp(trail, "QKM") == 0);
    go(120, 78, 1u);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 10, 0, 1u) == CS_PWS_OK);
    CHECK(ses.owner.window.x == 80 && ses.owner.window.y == 70);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 0u) == CS_PWS_OK);
    go(120, 78, 0u);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 1u) == CS_PWS_OK);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 20, -10, 1u) == CS_PWS_OK);
    CHECK(ses.owner.window.x == 100 && ses.owner.window.y == 80);
    for (i = 0u; i < 4u; ++i) CHECK(step(CS_PS2_DELIVERY_PACKET, -200, 0, 1u) == CS_PWS_OK);
    CHECK(ses.owner.pointer.x == 0 && ses.owner.window.x < 100);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 0u) == CS_PWS_OK);
    wx = ses.owner.window.x;
    go(wx + 112, ses.owner.window.y + 80, 0u);
    pattern = ses.owner.window.pattern;
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 1u) == CS_PWS_OK);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 0u) == CS_PWS_OK);
    CHECK(ses.owner.window.pattern != pattern && ses.owner.window.x == wx);
    ses.ram_frames = ses.published_frames = UINT32_MAX; i = F.presents;
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 0u) == CS_PWS_OK);
    CHECK(step(CS_PS2_DELIVERY_PACKET, 0, 0, 0u) == CS_PWS_OK);
    CHECK(F.presents == i && ses.ram_frames == UINT32_MAX && ses.published_frames == UINT32_MAX);
    untrail(); i = F.cancels;
    CHECK(step(CS_PS2_DELIVERY_LOST, 0, 0, 0u) == CS_PWS_OK && strcmp(trail, "QKC") == 0 && F.cancels == i + 1u);
    F.pop_mode = 1; F.pops = 0u; i = F.presents;
    CHECK(step(CS_PS2_DELIVERY_NONE, 0, 0, 0u) == CS_PWS_OK && ses.drained == 2u && F.pops == 3u);
    CHECK(F.presents == i && ses.phase == CS_PWS_RUNNING);
    F.pops = 0u;
    CHECK(step(CS_PS2_DELIVERY_PACKET, 1, 0, 0u) == CS_PWS_OK && ses.drained == 2u);
    F.pop_mode = 2; F.pops = 0u;
    CHECK(step(CS_PS2_DELIVERY_NONE, 0, 0, 0u) == CS_PWS_OK && F.pops == 8u && ses.drained == 8u);
    F.pop_mode = 0;
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_OK); untrail();
    CHECK(cs_pc_window_session_poll(&ses, ++tick, 1u) == CS_PWS_OK);
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_OK && trail_n == 0u);
    fstart(); F.script = 1; F.kind = CS_PS2_DELIVERY_NONE; F.bus_r = CS_PS2_ACTIVE;
    CHECK(cs_pc_window_session_poll(&ses, 50u, 1u) == CS_PWS_OK);
    CHECK(cs_pc_window_session_poll(&ses, 50u, 1u) == CS_PWS_OK && ses.stalled == 1u);
    CHECK(cs_pc_window_session_poll(&ses, 51u, 1u) == CS_PWS_OK && ses.stalled == 0u);
    for (i = 1u; i < (unsigned)CS_PWS_STALL_LIMIT && cs_pc_window_session_poll(&ses, 51u, 1u) == CS_PWS_OK; ++i) {}
    CHECK(i == (unsigned)CS_PWS_STALL_LIMIT && ses.stalled == (uint32_t)CS_PWS_STALL_LIMIT - 1u);
    untrail();
    CHECK(cs_pc_window_session_poll(&ses, 51u, 1u) == CS_PWS_CLOCK && trail_n == 0u);
    CHECK(ses.stalled == (uint32_t)CS_PWS_STALL_LIMIT && ses.stage == CS_PWS_TIME);
    fstart();
    CHECK(cs_pc_window_session_poll(&ses, 1u, 2u) == CS_PWS_CLOCK && trail_n == 0u);
    fstart();
    CHECK(step(CS_PS2_DELIVERY_NONE, 0, 0, 0u) == CS_PWS_OK); untrail();
    CHECK(cs_pc_window_session_poll(&ses, tick - 1u, 1u) == CS_PWS_CLOCK && trail_n == 0u);
    fstart(); F.pop_mode = 3;
    CHECK(step(CS_PS2_DELIVERY_PACKET, 1, 0, 0u) == CS_PWS_QUEUE && ses.cause == CS_INPUT_ERR_STATE);
    CHECK(ses.stage == CS_PWS_DRAIN && ses.consumed == 0u && F.packets == 0u && F.cancels == 1u && F.presents == 1u);
    untrail();
    CHECK(cs_pc_window_session_poll(&ses, ++tick, 1u) == CS_PWS_QUEUE);
    CHECK(cs_pc_window_session_stop(&ses) == CS_PWS_QUEUE && trail_n == 0u);
    fstart(); F.script = 1; F.kind = CS_PS2_DELIVERY_PACKET; F.bus_r = CS_PS2_TIMEOUT;
    CHECK(cs_pc_window_session_poll(&ses, ++tick, 1u) == CS_PWS_CONTROLLER && strcmp(trail, "Q") == 0);
    CHECK(ses.cause == CS_PS2_TIMEOUT && ses.stage == CS_PWS_POLL && ses.bus_result == CS_PS2_TIMEOUT);
    fstart();
    CHECK(step(7u, 0, 0, 0u) == CS_PWS_CONTROLLER && strcmp(trail, "QK") == 0);
    CHECK(ses.cause == CS_PS2_STATE && ses.stage == CS_PWS_DISPATCH && ses.consumed == 0u);
    fstart(); F.packet_r = CS_WINDOW_INPUT_FAULT;
    CHECK(step(CS_PS2_DELIVERY_PACKET, 3, 0, 0u) == CS_PWS_OWNER && ses.cause == CS_WINDOW_INPUT_FAULT);
    CHECK(ses.stage == CS_PWS_DISPATCH && ses.consumed == 1u && F.presents == 1u && ses.ram_frames == 1u);
    fstart(); F.present_r = CS_MONO_PRESENT;
    CHECK(step(CS_PS2_DELIVERY_PACKET, 3, 0, 0u) == CS_PWS_PRESENT && ses.cause == CS_MONO_PRESENT);
    CHECK(ses.stage == CS_PWS_PUBLISH && ses.presentation.rows == 5u && ses.presentation.phase == CS_MONO_FAILED);
    CHECK(ses.presentation.cause == CS_FB_STORAGE && ses.ram_frames == 2u && ses.published_frames == 1u);
    CHECK(fb[0] == 0x42u && ses.owner.pointer.x == 259 && F.cancels == 1u);
    untrail();
    CHECK(cs_pc_window_session_poll(&ses, ++tick, 1u) == CS_PWS_PRESENT && trail_n == 0u);
    fstart(); F.cancel_r = CS_WINDOW_INPUT_FAULT;
    CHECK(step(CS_PS2_DELIVERY_LOST, 0, 0, 0u) == CS_PWS_OWNER);
    CHECK(ses.cause == CS_WINDOW_INPUT_FAULT && F.presents == 1u);
}
#endif

int main(void)
{
    test_preflight(); test_start(); test_clock_stop();
#ifdef CS_PWS_FAULTS
    test_faults(); frames = F.frames;
#else
    test_real_reports();
#endif
    printf("TEST-0072 %s: cases=%u checks=%u frames=%u failures=%u session_bytes=%zu\n",
#ifdef CS_PWS_FAULTS
        "fault",
#else
        "normal",
#endif
        cases, checks, frames, fails, sizeof ses);
    return fails ? 1 : 0;
}
