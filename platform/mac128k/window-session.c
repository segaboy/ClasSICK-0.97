/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "window-session.h"

static int cs_mac_span(uintptr_t base,uintptr_t size)
{
    return size != 0u && size <= UINTPTR_MAX - base;
}

static int cs_mac_overlap(uintptr_t a,uintptr_t an,uintptr_t b,uintptr_t bn)
{
    return a < b + bn && b < a + an;
}

static int cs_mac_fault(cs_mac_window_session *s,int32_t cause)
{
    s->phase = CS_MAC_WINDOW_FAILED;
    s->cause = cause;
    return CS_MAC_WINDOW_FAULT;
}

int cs_mac_window_begin(cs_mac_window_session *session,uint8_t *canvas,
    size_t capacity,const cs_foreground_hooks *hooks)
{
    uintptr_t sa,ca,ha,sn,cn,hn;
    cs_foreground_hooks h;
    cs_cursor_token token;
    cs_window_config config;
    cs_product_state product;
    cs_cursor_result cr;
    int r;
    uint32_t i;

    if (session == NULL || canvas == NULL || hooks == NULL)
        return CS_MAC_WINDOW_ARGUMENT;
    if (capacity < (size_t)CS_MAC_WINDOW_BYTES)
        return CS_MAC_WINDOW_ARGUMENT;
#if SIZE_MAX > UINTPTR_MAX
    if (capacity > (size_t)UINTPTR_MAX)
        return CS_MAC_WINDOW_ARGUMENT;
#endif
    sa = (uintptr_t)(void *)session;
    ca = (uintptr_t)(void *)canvas;
    ha = (uintptr_t)(const void *)hooks;
    sn = (uintptr_t)sizeof(*session);
    cn = (uintptr_t)capacity;
    hn = (uintptr_t)sizeof(*hooks);
    if (!cs_mac_span(sa,sn) || !cs_mac_span(ca,cn) || !cs_mac_span(ha,hn))
        return CS_MAC_WINDOW_ARGUMENT;
    if (sa % (uintptr_t)_Alignof(cs_mac_window_session) != 0u ||
        ha % (uintptr_t)_Alignof(cs_foreground_hooks) != 0u)
        return CS_MAC_WINDOW_ARGUMENT;
    if (cs_mac_overlap(sa,sn,ca,cn) || cs_mac_overlap(sa,sn,ha,hn) ||
        cs_mac_overlap(ca,cn,ha,hn))
        return CS_MAC_WINDOW_ARGUMENT;

    h = *hooks;
    if (h.enter == NULL || h.leave == NULL)
        return CS_MAC_WINDOW_ARGUMENT;
    session->magic = CS_MAC_WINDOW_MAGIC;
    session->phase = CS_MAC_WINDOW_BUILDING;
    session->stage = CS_MAC_WINDOW_STAGE_START;
    session->cause = 0;

    session->stage = CS_MAC_WINDOW_STAGE_SURFACE;
    r = (int)cs_surface_init(&session->surface,canvas,
        (size_t)CS_MAC_WINDOW_BYTES,CS_MAC_WINDOW_WIDTH,CS_MAC_WINDOW_HEIGHT,
        (size_t)CS_MAC_WINDOW_STRIDE,CS_SURFACE_MONO1_MSB);
    if (r != (int)CS_SURFACE_OK)
        return cs_mac_fault(session,(int32_t)r);

    /* Original PC 7x7 plus: mask equals image. */
    for (i = 0u; i < 32u; ++i) {
        uint8_t v = 0u;
        if ((i & 1u) == 0u && i < 14u)
            v = (uint8_t)(i == 6u ? 0xFEu : 0x10u);
        session->bits.mask[i] = v;
        session->bits.image[i] = v;
    }
    session->art.bits = &session->bits;
    session->art.width = 7u;
    session->art.height = 7u;
    session->art.hot_x = 3u;
    session->art.hot_y = 3u;

    session->stage = CS_MAC_WINDOW_STAGE_CURSOR_INIT;
    cr = cs_cursor_init(&session->cursor);
    if (cr != CS_CURSOR_OK)
        return cs_mac_fault(session,(int32_t)cr);
    session->stage = CS_MAC_WINDOW_STAGE_CURSOR_BIND;
    cr = cs_cursor_bind(&session->cursor,&session->surface,session->payload,
        sizeof(session->payload),&token);
    if (cr != CS_CURSOR_OK)
        return cs_mac_fault(session,(int32_t)cr);

    config.x = 80; config.y = 70; config.width = 224; config.height = 144;
    config.title = 18;
    product.value = 97u; product.inputs = 0u; product.progress = 16u;
    product.phases = 6u; product.success = 1u; product.pixels = NULL;
    session->stage = CS_MAC_WINDOW_STAGE_INPUT;
    r = cs_window_input_begin(&session->input,&session->cursor,token,
        &session->art,&config,&product,0u);
    if (r != CS_WINDOW_INPUT_OK && r != CS_WINDOW_INPUT_CHANGED)
        return cs_mac_fault(session,(int32_t)r);

    /* Foreground begin performs the single owner cancellation. */
    session->stage = CS_MAC_WINDOW_STAGE_FOREGROUND;
    r = cs_foreground_begin(&session->foreground,&session->inbox,
        &session->capture,&session->input,&h);
    if (r != CS_FG_OK)
        return cs_mac_fault(session,(int32_t)r);

    session->stage = CS_MAC_WINDOW_STAGE_READY;
    session->phase = CS_MAC_WINDOW_READY;
    return CS_MAC_WINDOW_OK;
}
