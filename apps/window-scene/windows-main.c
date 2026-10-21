/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/windows/window-preview.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    cs_win_window *w;
    MSG msg;
    BOOL r;
    int rc;
    uint32_t left;
    (void)argv;
    if (argc != 1) {
        fprintf(stderr, "usage: window-scene-windows\n");
        return 2;
    }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    w = (cs_win_window *)calloc(1, sizeof *w);
    if (w == NULL) return 1;
    left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1u : 0u;
    if (!cs_win_window_open(w, 1, left)) {
        free(w);
        return 1;
    }
    while ((r = GetMessageW(&msg, NULL, 0, 0)) > 0) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) break;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (r < 0) w->failed = 1;
    rc = w->failed ? 1 : 0;
    cs_win_window_close(w);
    free(w);
    return rc;
}
