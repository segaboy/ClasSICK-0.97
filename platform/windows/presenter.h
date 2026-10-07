/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOWS_PRESENTER_H
#define CLASSICK_WINDOWS_PRESENTER_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "pixels.h"

/* Source and tight BGRX scratch are caller-owned. Preserves the HDC's state. */
cs_win_result cs_win_present(HDC dc, const cs_surface *source, uint8_t *scratch,
    size_t scratch_size, int32_t client_width, int32_t client_height);
#endif
