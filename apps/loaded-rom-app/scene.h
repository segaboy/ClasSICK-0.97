/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_LOADED_APP_SCENE_H
#define CLASSICK_LOADED_APP_SCENE_H
#include "probe.h"
#include "../../core/graphics/surface.h"
/* Native presentation of observed guest results, not a guest QuickDraw API. */
int cs_loaded_app_scene(const cs_surface *screen,const cs_loaded_app_result *result,uint32_t presses);
#endif
