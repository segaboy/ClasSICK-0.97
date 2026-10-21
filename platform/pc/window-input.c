/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "window-input.h"

int cs_pc_window_packet(cs_window_input *owner,
    const cs_ps2_mouse_packet *packet)
{
    cs_pointer_sample sample;
    if(owner==NULL || packet==NULL) return CS_WINDOW_INPUT_ARGUMENT;
    /* Validate the decoder's complete output domain before negation/effects.
       -256 is valid even though the positive endpoint is only 255. */
    if(packet->dx < -256 || packet->dx > 255
            || packet->dy < -256 || packet->dy > 255
            || packet->buttons > 7u) return CS_WINDOW_INPUT_ARGUMENT;
    sample.dx=packet->dx;
    sample.dy=-packet->dy;
    sample.buttons=packet->buttons;
    return cs_window_input_sample(owner,&sample);
}
