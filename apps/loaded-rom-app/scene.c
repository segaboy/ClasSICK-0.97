/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original display of the loaded app result. */
#include "scene.h"
#include "../../platform/mac128k/rom.h"
static int rect(const cs_surface *s,int32_t x,int32_t y,int32_t w,int32_t h,uint32_t v)
{
    /* Validated scene surface; reuse the project's original masked-byte fill. */
    if(x<0 || y<0 || w<=0 || h<=0 || w>s->width || h>s->height ||
        x>s->width-w || y>s->height-h || v>1u)return 0;
    size_t first=(size_t)x/8u,last=(size_t)(x+w-1)/8u;
    uint8_t left=(uint8_t)(UINT32_C(0xFF)>>((uint32_t)x&7u));
    uint8_t right=(uint8_t)(UINT32_C(0xFF)<<(7u-((uint32_t)(x+w-1)&7u)));
    uint8_t *row=s->storage+(size_t)y*s->stride+first;
    for(int32_t j=0;j<h;++j){
        for(size_t col=first;col<=last;++col){
            uint8_t mask=0xFFu;
            if(col==first)mask&=left;
            if(col==last)mask&=right;
            uint8_t *p=row+(col-first);
            *p=v?(uint8_t)(*p|mask):(uint8_t)(*p&(uint8_t)~mask);
        }
        if(j+1<h)row+=s->stride;
    }
    return 1;
}
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

int cs_loaded_app_scene_input(const cs_surface *s,uint32_t presses)
{
    cs_surface checked;
    if(!s || s->format!=CS_SURFACE_MONO1_MSB || s->width!=512 || s->height!=342 ||
        cs_surface_init(&checked,s->storage,s->storage_size,s->width,s->height,
            s->stride,s->format)!=CS_SURFACE_OK)return 0;
    s=&checked;
    /* Same original project glyphs as apps/rom-scene, not a historical font. */
    static const uint16_t digits[10]={0x7B6Fu,0x2492u,0x73E7u,0x73CFu,0x5BC9u,
        0x79CFu,0x79EFu,0x7249u,0x7BEFu,0x7BCFu};
    int ok=rect(s,352,286,80,20,0);
    for(unsigned i=0;i<4;++i){
        uint16_t bits=digits[presses%10u];presses/=10u;
        for(unsigned row=0;row<5;++row)for(unsigned col=0;col<3;++col)
            if(bits&(UINT16_C(1)<<(14u-row*3u-col)))
                ok&=rect(s,352+(int32_t)(3u-i)*20+(int32_t)col*4,
                    286+(int32_t)row*4,4,4,1);
    }
    return ok;
}
