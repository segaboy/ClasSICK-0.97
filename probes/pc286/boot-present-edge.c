/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Compile the exact diagnostic adapter into a separate boundary fixture. */
#define start cs_unused_window_start
#include "boot-window-shared.c"
#undef start
#include "present-case.h"

void start(void)
{
    unsigned i;
    for(i=0;i<sizeof(canvas);++i)canvas[i]=(unsigned char)(i*37u+11u);
    video_mode();white_display();
    present(TEST_LEFT,TEST_TOP,TEST_RIGHT,TEST_BOTTOM);
    post(0xac);for (;;) {}
}
