/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m0110-reply.h"

int cs_m0110_reply(uint32_t reply, cs_m0110_event *out)
{
    uint32_t position;
    if (out == 0 || reply > 255u) return CS_M0110_ARGUMENT;
    if (reply == 0x7bu) return CS_M0110_EMPTY;
    position = (reply & 0x7fu) >> 1;
    if ((reply & 1u) == 0u || position > 0x39u) return CS_M0110_INVALID;
    out->position = position;
    out->pressed = (reply & 0x80u) == 0u ? 1u : 0u;
    return CS_M0110_EVENT;
}
