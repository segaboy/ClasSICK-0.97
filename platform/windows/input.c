/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input.h"

int cs_win_key_event(UINT message, WPARAM key, LPARAM flags, uint32_t source, cs_input_event *out)
{
    uint32_t logical;
    if (out == NULL || source == 0) return CS_WIN_INPUT_ERR_ARGUMENT;
    if (message != WM_KEYDOWN && message != WM_KEYUP) return CS_WIN_INPUT_IGNORED;
    if (key == VK_SPACE) logical = CS_KEY_SPACE;
    else if (key == VK_ESCAPE) logical = CS_KEY_ESCAPE;
    else return CS_WIN_INPUT_IGNORED;
    out->source = source;
    out->key = logical;
    out->action = message == WM_KEYDOWN ? CS_KEY_PRESS : CS_KEY_RELEASE;
    out->repeat = message == WM_KEYDOWN && (flags & ((LPARAM)1 << 30)) != 0 ? 1u : 0u;
    return CS_WIN_INPUT_MAPPED;
}
