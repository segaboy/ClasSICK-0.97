/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/windows/input.h"
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"win-input:%d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(void)
{
    cs_input_event out = {41,42,43,44};
    const UINT messages[] = {WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, WM_SYSKEYUP, WM_CHAR, WM_PAINT};
    const WPARAM keys[] = {VK_SPACE, VK_ESCAPE, 'A', (WPARAM)UINT32_MAX};
    const LPARAM flags[] = {0, 1, (LPARAM)1 << 30, (LPARAM)INT32_MIN, -1};
    CHECK(cs_win_key_event(WM_CHAR, 0, 0, 1, NULL) == CS_WIN_INPUT_ERR_ARGUMENT);
    CHECK(cs_win_key_event(WM_CHAR, 0, 0, 0, &out) == CS_WIN_INPUT_ERR_ARGUMENT);
    CHECK(out.source == 41 && out.key == 42 && out.action == 43 && out.repeat == 44);
    for (size_t m = 0; m < sizeof(messages)/sizeof(messages[0]); ++m)
    for (size_t k = 0; k < sizeof(keys)/sizeof(keys[0]); ++k)
    for (size_t f = 0; f < sizeof(flags)/sizeof(flags[0]); ++f) {
        int mapped = m < 2 && k < 2;
        out = (cs_input_event){41,42,43,44};
        CHECK(cs_win_key_event(messages[m], keys[k], flags[f], 17, &out)
            == (mapped ? CS_WIN_INPUT_MAPPED : CS_WIN_INPUT_IGNORED));
        if (!mapped) CHECK(out.source == 41 && out.key == 42 && out.action == 43 && out.repeat == 44);
        else {
            CHECK(out.source == 17 && out.key == (k == 0 ? CS_KEY_SPACE : CS_KEY_ESCAPE));
            CHECK(out.action == (m == 0 ? CS_KEY_PRESS : CS_KEY_RELEASE));
            CHECK(out.repeat == (m == 0 && (f == 2 || f == 4) ? 1u : 0u));
        }
    }
    puts("SPEC-0004 Windows mapping: PASS (120 independent table cases)");
    return 0;
}
