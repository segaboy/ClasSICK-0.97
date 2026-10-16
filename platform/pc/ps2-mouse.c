/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "ps2-mouse.h"

static void reset(cs_ps2_mouse *mouse)
{
    mouse->used = 0;
    mouse->header = 0;
    mouse->x = 0;
}

int cs_ps2_mouse_begin(cs_ps2_mouse *mouse)
{
    if (mouse == 0) return CS_PS2_MOUSE_ARGUMENT;
    reset(mouse);
    return CS_PS2_MOUSE_MORE;
}

int cs_ps2_mouse_feed(cs_ps2_mouse *mouse, uint32_t byte, uint32_t corrupt,
    cs_ps2_mouse_packet *packet)
{
    uint32_t header, x;
    if (mouse == 0 || packet == 0 || byte > 255u || corrupt > 1u)
        return CS_PS2_MOUSE_ARGUMENT;
    if (mouse->used > 2u || mouse->header > 255u || mouse->x > 255u
            || (mouse->used > 0u && (mouse->header & 8u) == 0u))
        return CS_PS2_MOUSE_STATE;
    if (corrupt != 0u) {
        reset(mouse);
        return CS_PS2_MOUSE_DROPPED;
    }
    if (mouse->used == 0u) {
        if ((byte & 8u) == 0u) return CS_PS2_MOUSE_DROPPED;
        mouse->header = byte;
        mouse->used = 1;
        mouse->x = 0;
        return CS_PS2_MOUSE_MORE;
    }
    if (mouse->used == 1u) {
        mouse->x = byte;
        mouse->used = 2;
        return CS_PS2_MOUSE_MORE;
    }
    header = mouse->header;
    x = mouse->x;
    reset(mouse);
    /* Original policy: an overflowed axis drops the whole report. */
    if ((header & 0xC0u) != 0u) return CS_PS2_MOUSE_DROPPED;
    packet->dx = (int32_t)x - ((header & 0x10u) != 0u ? 256 : 0);
    packet->dy = (int32_t)byte - ((header & 0x20u) != 0u ? 256 : 0);
    packet->buttons = header & 7u;
    return CS_PS2_MOUSE_PACKET;
}
