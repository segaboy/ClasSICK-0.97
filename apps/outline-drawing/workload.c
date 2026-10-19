/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0049 paired native outline workload. */
#include "workload.h"
#include "../../personality/system1/quickdraw-outline.h"
#include "../../personality/system1/quickdraw-select.h"

static uint32_t black_bits(const uint8_t *pixels)
{
    uint32_t black=0;
    for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i)
        for(unsigned bit=0;bit<8;++bit)black+=(uint32_t)((pixels[i]>>bit)&1u);
    return black;
}

int cs_outline_draw(cs_selected_draw_workspace *workspace,
                    cs_selected_draw_result *result)
{
    static const uint8_t clip[2][8]={{0,8,0,8,0,56,0,56},{0,8,0,72,0,56,0,120}};
    static const uint8_t outline[2][8]={{0,0,0,0,0,48,0,48},{0xFF,0xF8,0,0,0,56,0,48}};
    static const int32_t offset_x[2]={8,72};
    static const int32_t pen_w[2]={3,4};
    static const int32_t pen_h[2]={2,3};
    static const uint32_t ids[2]={UINT32_C(0x7000),UINT32_C(0x706C)};
    static const uint32_t expected[2]={456u,384u};
    cs_qd_view views[2];
    cs_qd_selection selection;
    cs_qd_select_plan plan;
    uint32_t current=0;
    if(!workspace || !result)return 0;
    for(uint32_t p=0;p<2u;++p){
        for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i)workspace->ports[p].pixels[i]=0;
        workspace->ports[p].paints=0;workspace->ports[p].erases=0;
    }
    for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i)workspace->pixels[i]=0;
    result->draw.success=0;result->draw.phase=1;result->draw.value=0;
    result->draw.pattern=0;result->draw.black_pixels=0;
    result->current=0;result->selections=0;
    selection.globals_begin=UINT32_C(0x7000);selection.a5=UINT32_C(0x7200);
    selection.g=UINT32_C(0x71FC);selection.count=2u;
    for(uint32_t p=0;p<2u;++p){
        if(!cs_qd_view_init(&views[p],&workspace->ports[p]) ||
            !cs_qd_view_offset(&views[p],offset_x[p],8) ||
            !cs_qd_view_clip(&views[p],clip[p]))return 0;
        selection.bindings[p].id=ids[p];selection.bindings[p].view=&views[p];
    }
    plan.value=0;plan.view=0;
    for(uint32_t k=0;k<2u;++k){
        if(!cs_qd_select_set(&selection,current,ids[k],&plan))return 0;
        current=plan.value;result->current=current;
        if(current!=ids[k])return 0;
        if(!cs_qd_select_get(&selection,current,&plan) || plan.value!=ids[k])return 0;
        uint32_t index=2u;
        for(uint32_t p=0;p<2u;++p)if(plan.view==&views[p])index=p;
        if(index!=k)return 0;
        result->selections=k+1u;
        if(!cs_qd_view_outline(&views[index],outline[index],pen_w[index],pen_h[index]))
            return 0;
        result->draw.phase=2u+k;
    }
    if(!cs_qd_select_get(&selection,current,&plan) || plan.value!=ids[1])return 0;
    if(current!=ids[1] || result->selections!=2u)return 0;
    for(uint32_t p=0;p<2u;++p){
        if(workspace->ports[p].paints!=1u || workspace->ports[p].erases!=0u)return 0;
        if(black_bits(workspace->ports[p].pixels)!=expected[p])return 0;
    }
    uint32_t hash=UINT32_C(2166136261);
    for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i){
        uint8_t value=(uint8_t)(workspace->ports[0].pixels[i]|workspace->ports[1].pixels[i]);
        workspace->pixels[i]=value;
        hash=(hash^value)*UINT32_C(16777619);
    }
    uint32_t black=black_bits(workspace->pixels);
    result->draw.pattern=hash;result->draw.black_pixels=black;result->draw.phase=4;
    /* Presentation is published only on success; native port effects survive. */
    if(black!=840u){for(uint32_t i=0;i<CS_QD_RECT_BYTES;++i)workspace->pixels[i]=0;return 0;}
    result->draw.phase=5;
    result->draw.value=97;result->draw.success=1;result->draw.phase=6;
    return 1;
}
