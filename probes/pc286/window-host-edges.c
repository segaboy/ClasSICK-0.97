/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include "../../apps/window-scene/input.h"
int main(int argc,char **argv)
{
    unsigned char canvas[21888],saved[48];
    cs_surface surface;cs_cursor_state cursor;cs_cursor_token token;cs_cursor_bits bits;
    cs_cursor_art art;cs_window_config config;cs_product_state product;cs_window_input owner;
    cs_pointer_sample sample;int dx,dy,result;unsigned buttons;FILE *f;
    if(argc!=3)return 1;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    product.value=97;product.inputs=0;product.progress=16;product.phases=6;product.success=1;product.pixels=0;
    if(cs_surface_init(&surface,canvas,sizeof(canvas),512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK)return 2;
    cs_window_cursor_bits_init(&bits);art.bits=&bits;art.width=15;art.height=15;art.hot_x=7;art.hot_y=7;
    if(cs_cursor_init(&cursor)!=CS_CURSOR_OK || cs_cursor_bind(&cursor,&surface,saved,sizeof(saved),&token)!=CS_CURSOR_OK)return 3;
    if(cs_window_input_begin(&owner,&cursor,token,&art,&config,&product,0)!=CS_WINDOW_INPUT_OK)return 4;
    f=fopen(argv[1],"r");if(!f)return 5;
    while((result=fscanf(f,"%d %d %u",&dx,&dy,&buttons))==3) {
        sample.dx=dx;sample.dy=dy;sample.buttons=buttons;
        result=cs_window_input_sample(&owner,&sample);
        if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED){fclose(f);return 6;}
    }
    if(result!=EOF || ferror(f)){fclose(f);return 7;}fclose(f);
    f=fopen(argv[2],"wb");if(!f)return 8;
    if(fwrite("P4\n512 342\n",1,11,f)!=11 || fwrite(canvas,1,sizeof(canvas),f)!=sizeof(canvas)){fclose(f);return 9;}
    if(fclose(f))return 10;
    printf("%ld %ld %ld %ld %lu %lu %lu\n",(long)owner.pointer.x,(long)owner.pointer.y,
        (long)owner.window.x,(long)owner.window.y,(unsigned long)owner.window.pattern,
        (unsigned long)owner.window.down,(unsigned long)owner.counter.presses);
    return 0;
}
