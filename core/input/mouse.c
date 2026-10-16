/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mouse.h"

int cs_mouse_button_begin(cs_mouse_button *button, uint32_t down)
{
    if (button == 0 || down > 1u) return CS_MOUSE_ARGUMENT;
    button->down = down;
    button->presses = 0;
    return CS_MOUSE_OK;
}

int cs_mouse_button_sample(cs_mouse_button *button, uint32_t down)
{
    if (button == 0 || down > 1u) return CS_MOUSE_ARGUMENT;
    if (button->down > 1u) return CS_MOUSE_STATE;
    if (button->down == down) return CS_MOUSE_OK;
    if (down != 0u && button->presses != UINT32_MAX) ++button->presses;
    button->down = down;
    return CS_MOUSE_CHANGED;
}
