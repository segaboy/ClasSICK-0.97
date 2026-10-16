/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Copy actual guest-produced pixels. */
#include "probe.h"
int cs_drawing_app_scene(const cs_surface *screen,const cs_loaded_app_result *result,
    const cs_qd_rect_port *port,uint32_t presses)
{
    cs_surface checked;
    if(!screen || !result || !port || screen->width!=512 || screen->height!=342 ||
        screen->format!=CS_SURFACE_MONO1_MSB ||
        cs_surface_init(&checked,screen->storage,screen->storage_size,screen->width,
            screen->height,screen->stride,screen->format)!=CS_SURFACE_OK)return 0;
    /* Native objects/output/borrowed pixels are live and disjoint by contract. */
    if(!cs_loaded_app_scene(&checked,result,presses))return 0;
    /* The application's image replaces the old checksum-bit visualization.
     * Preserve the panel border, all phase/progress marks and counter. */
    for(size_t y=132;y<204;++y){uint8_t *row=checked.storage+y*checked.stride;
        row[6]&=0xF0u;for(size_t x=7;x<57;++x)row[x]=0;row[57]&=0x0Fu;}
    for(size_t y=0;y<CS_QD_RECT_HEIGHT;++y)for(size_t x=0;x<16;++x)
        checked.storage[(136u+y)*checked.stride+24u+x]=port->pixels[y*16u+x];
    return 1;
}
