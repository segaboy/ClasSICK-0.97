/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Copy actual guest-produced pixels. */
#include "probe.h"
#include "../product-scene/scene.h"
int cs_drawing_app_scene(const cs_surface *screen,const cs_loaded_app_result *result,
    const cs_qd_rect_port *port,uint32_t presses)
{
    if(!screen || !result || !port || screen->width!=512 || screen->height!=342 ||
        screen->format!=CS_SURFACE_MONO1_MSB)return 0;
    cs_product_state state;
    state.value=result->value;state.inputs=presses;state.progress=0;
    state.phases=result->phase;state.success=result->success;state.pixels=port->pixels;
    return cs_product_scene(screen,512,342,0,&state);
}
