/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Host-side shared-compositor fixture. */
#include <stdio.h>
#include "../../apps/window-scene/input.h"

int main(int argc,char **argv)
{
    unsigned char canvas[21888],saved[48];
    cs_surface surface;cs_cursor_state cursor;cs_cursor_token token;
    cs_cursor_bits bits;cs_cursor_art art;cs_window window;cs_window_config config;
    cs_product_state product;cs_window_scene_effect effect;FILE *f;
    if(argc!=2)return 1;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    product.value=97;product.inputs=0;product.progress=16;product.phases=6;product.success=1;product.pixels=0;
    if(cs_window_begin(&window,&config)!=CS_WINDOW_OK)return 2;
    if(cs_surface_init(&surface,canvas,sizeof(canvas),512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK)return 3;
    cs_window_cursor_bits_init(&bits);art.bits=&bits;art.width=15;art.height=15;art.hot_x=7;art.hot_y=7;
    if(cs_cursor_init(&cursor)!=CS_CURSOR_OK || cs_cursor_bind(&cursor,&surface,saved,sizeof(saved),&token)!=CS_CURSOR_OK)return 4;
    if(cs_window_scene_compose(&cursor,token,&art,&product,0,&window,0,100,80,&effect)!=CS_WINDOW_SCENE_OK)return 5;
    f=fopen(argv[1],"wb");if(!f)return 6;
    if(fwrite("P4\n512 342\n",1,11,f)!=11 || fwrite(canvas,1,sizeof(canvas),f)!=sizeof(canvas)) {
        fclose(f);return 7;
    }
    return fclose(f)==0?0:8;
}
