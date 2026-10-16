/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original failure-control link, not the renderer. */
#include "../apps/native-drawing/workload.h"
#include "../apps/boot-scene/scene.h"
#include <stdio.h>
static unsigned mode,calls;
int cs_qd_rect(cs_qd_rect_port *port,const uint8_t rect[8],uint16_t word)
{
    (void)rect;++calls;
    if(word==CS_QD_PAINT_RECT){port->paints=1;port->pixels[0]=0xA5;}
    if(word==CS_QD_ERASE_RECT){port->erases=1;port->pixels[1]=0x5A;}
    return !((mode==1 && calls==1) || (mode==2 && calls==2));
}
int main(void)
{
    cs_qd_rect_port port;cs_native_draw_result result;
    for(mode=1;mode<=3;++mode){
        calls=0;
        if(cs_native_draw(&port,&result) || result.success || result.value ||
            calls!=(mode==1?1u:2u) || result.phase!=(mode==1?1u:mode==2?2u:4u) ||
            port.pixels[0]!=0xA5 || (mode!=1 && port.pixels[1]!=0x5A)){
            fprintf(stderr,"native failure control%u failed\n",mode);return 1;
        }
        if(mode==3 && (result.black_pixels!=8 || result.pattern==0))return 1;
        uint8_t pixels[64];cs_surface screen;
        for(unsigned i=0;i<sizeof pixels;++i)pixels[i]=0x6D;
        if(cs_surface_init(&screen,pixels,sizeof pixels,4,4,16,CS_SURFACE_RGBA8)!=CS_SURFACE_OK)return 1;
        calls=0;
        if(cs_scene_render(&screen,4,4,0,0)!=CS_SCENE_WORKLOAD)return 1;
        for(unsigned i=0;i<sizeof pixels;++i)if(pixels[i]!=0x6D)return 1;
    }
    return 0;
}
