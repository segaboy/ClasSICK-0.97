/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0038 native workload. */
#include "workload.h"
#include "../../personality/system1/quickdraw-view.h"

int cs_native_draw(cs_qd_rect_port *port,cs_native_draw_result *result)
{
    static const uint8_t clip[8]={0,8,0,8,0,56,0,120};
    static const uint8_t paint[8]={0x80,0,0x80,0,0x7F,0xFF,0x7F,0xFF};
    static const uint8_t erase[8]={0,8,0,8,0,40,0,104};
    cs_qd_view view;
    if(!port || !result)return 0;
    for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i)port->pixels[i]=0;
    port->paints=0;port->erases=0;
    result->success=0;result->phase=1;result->value=0;
    result->pattern=0;result->black_pixels=0;
    if(!cs_qd_view_init(&view,port))return 0;
    if(!cs_qd_view_offset(&view,8,8))return 0;
    if(!cs_qd_view_clip(&view,clip))return 0;
    if(!cs_qd_view_rect(&view,paint,CS_QD_PAINT_RECT))return 0;
    result->phase=2;
    if(!cs_qd_view_rect(&view,erase,CS_QD_ERASE_RECT))return 0;
    result->phase=3;
    uint32_t hash=UINT32_C(2166136261),black=0;
    for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i){
        uint8_t value=port->pixels[i];
        hash=(hash^value)*UINT32_C(16777619);
        for(unsigned bit=0;bit<8;++bit)black+=(uint32_t)((value>>bit)&1u);
    }
    result->pattern=hash;result->black_pixels=black;result->phase=4;
    if(port->paints!=1 || port->erases!=1 || black!=2304)return 0;
    result->phase=5;
    result->value=97;result->success=1;result->phase=6;
    return 1;
}
