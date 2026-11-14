/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Hosted shared-owner sequence fixture. */
#include <stdio.h>
#include <stdlib.h>
#include "../../apps/window-scene/input.h"

int main(int argc,char **argv)
{
    unsigned char canvas[21888],saved[48];
    cs_surface surface;cs_cursor_state cursor;cs_cursor_token token;cs_cursor_bits bits;
    cs_cursor_art art;cs_window_config config;cs_product_state product;cs_window_input owner;
    cs_pointer_sample sample;int count,i,result;FILE *f;
    static const int dx[11]={0,16,16,0,0,16,0,0,0,0,0};
    static const int dy[11]={0,0,0,-16,0,0,16,16,16,0,0};
    static const unsigned buttons[11]={1,1,1,1,0,0,0,0,0,1,0};
    if(argc!=3)return 1;count=atoi(argv[1]);if(count<0 || count>11)return 2;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    product.value=97;product.inputs=0;product.progress=16;product.phases=6;product.success=1;product.pixels=0;
    if(cs_surface_init(&surface,canvas,sizeof(canvas),512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK)return 3;
    cs_window_cursor_bits_init(&bits);art.bits=&bits;art.width=15;art.height=15;art.hot_x=7;art.hot_y=7;
    if(cs_cursor_init(&cursor)!=CS_CURSOR_OK || cs_cursor_bind(&cursor,&surface,saved,sizeof(saved),&token)!=CS_CURSOR_OK)return 4;
    if(cs_window_input_begin(&owner,&cursor,token,&art,&config,&product,0)!=CS_WINDOW_INPUT_OK)return 5;
    sample.dx=-156;sample.dy=-91;sample.buttons=0;
    result=cs_window_input_sample(&owner,&sample);
    if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED)return 6;
    for(i=0;i<count;++i) {
        sample.dx=dx[i];sample.dy=dy[i];sample.buttons=buttons[i];
        result=cs_window_input_sample(&owner,&sample);
        if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED)return 7;
    }
    f=fopen(argv[2],"wb");if(!f)return 8;
    if(fwrite("P4\n512 342\n",1,11,f)!=11 || fwrite(canvas,1,sizeof(canvas),f)!=sizeof(canvas)) {
        fclose(f);return 9;
    }
    return fclose(f)==0?0:10;
}
