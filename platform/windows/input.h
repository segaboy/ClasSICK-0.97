/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOWS_INPUT_H
#define CLASSICK_WINDOWS_INPUT_H
#include "../../core/input/input.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
enum { CS_WIN_INPUT_MAPPED = 0, CS_WIN_INPUT_IGNORED = 1, CS_WIN_INPUT_ERR_ARGUMENT = 2 };
int cs_win_key_event(UINT message, WPARAM key, LPARAM flags, uint32_t source, cs_input_event *out);
#endif
