/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original display of the loaded app result. */
#include "scene.h"
#include "../../platform/mac128k/rom.h"
static int rect(const cs_surface *s,int32_t x,int32_t y,int32_t w,int32_t h,uint32_t v)
{cs_rect r={x,y,x+w,y+h};return cs_surface_fill(s,r,v)==CS_SURFACE_OK;}
int cs_loaded_app_scene(const cs_surface *s,const cs_loaded_app_result *r,uint32_t presses)
{
    if(!s || !r || !cs_mac_scene(s,r->value,presses))return 0;
    /* Independently drawn APP letterforms, not a historical font. */
    static const uint8_t letters[3][5]={{2,5,7,5,5},{6,5,6,4,4},{6,5,6,4,4}};
    int ok=1;
    for(unsigned g=0;g<3;++g)for(unsigned row=0;row<5;++row)for(unsigned col=0;col<3;++col)
        if(letters[g][row]&(4u>>col))ok&=rect(s,222+(int32_t)g*16+(int32_t)col*4,
            64+(int32_t)row*4,4,4,1);
    ok&=rect(s,52,132,408,72,0);
    for(unsigned bit=0;bit<32;++bit)ok&=rect(s,64+(int32_t)bit*12,144,10,48,(r->pattern>>(31u-bit))&1u);
    /* Six observed phases, then a distinct success/error outline. */
    for(unsigned phase=1;phase<=6;++phase)ok&=rect(s,64+(int32_t)(phase-1u)*64,214,40,8,r->phase>=phase?1u:0u);
    if(!r->success){ok&=rect(s,20,124,16,88,1);ok&=rect(s,476,124,16,88,1);}
    return ok;
}
