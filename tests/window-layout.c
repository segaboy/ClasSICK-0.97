/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Native ABI assertions only. */
#include "../core/ui/window.h"
_Static_assert(sizeof(cs_window)==44u,"window state");
_Static_assert(sizeof(cs_window_config)==20u,"window config");
_Static_assert(sizeof(cs_window_effect)==20u,"window effect");
