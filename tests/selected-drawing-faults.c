/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Lead-labelled completion/failure checks. */
#include "../apps/selected-drawing/workload.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned calls,refuse;
#define CHECK(c) do{if(!(c)){fprintf(stderr,"line %d: %s\n",__LINE__,#c);exit(1);}}while(0)
/* Test-only helper replacement. Real selection/view execute; original ports
 * retain deliberately wrong nonzero pixels on completed operations. */
int cs_qd_rect(cs_qd_rect_port *p,const uint8_t rect[8],uint16_t word)
{
    CHECK(p && rect);++calls;
    if(calls==refuse)return 0;
    p->pixels[0]=0x80;
    if(word==CS_QD_PAINT_RECT)++p->paints;
    else{CHECK(word==CS_QD_ERASE_RECT);++p->erases;}
    return 1;
}
int main(void)
{
    cs_selected_draw_workspace *w=malloc(sizeof *w);
    cs_selected_draw_result r;
    CHECK(w);
    for(refuse=0;refuse<=4;++refuse){
        memset(w,0xCC,sizeof *w);memset(&r,0xA5,sizeof r);calls=0;
        CHECK(cs_selected_draw(w,&r)==0 && !r.draw.success && !r.draw.value);
        for(size_t i=0;i<sizeof w->pixels;++i)CHECK(!w->pixels[i]);
        CHECK(r.draw.phase==(refuse==0?4u:refuse<3?1u:2u));
        CHECK(r.selections==(refuse==1||refuse==2?1u:2u));
        CHECK(r.current==(r.selections==1?0x7000u:0x706Cu));
        CHECK(calls==(refuse?refuse:4u));
        if(refuse!=1)CHECK(w->ports[0].pixels[0]==0x80 && w->ports[0].paints==1);
        if(!refuse)CHECK(r.draw.black_pixels==1 && w->ports[1].pixels[0]==0x80);
    }
    free(w);puts("selected drawing failure phases/presentation/retained native effects PASS");return 0;
}
