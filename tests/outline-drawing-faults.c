/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Lead failure/retained-effect checks. */
#include "../apps/outline-drawing/workload.h"
#include "../personality/system1/quickdraw-outline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned calls,refuse,overlap;
#define CHECK(c) do{if(!(c)){fprintf(stderr,"line%d: %s\n",__LINE__,#c);exit(1);}}while(0)
/* Test-only outline replacement; real selection and native view setup execute. */
int cs_qd_view_outline(cs_qd_view *v,const uint8_t rect[8],int32_t w,int32_t h)
{
    CHECK(v && v->port && rect && w>0 && h>0);++calls;
    if(calls==refuse)return 0;
    if(overlap){
        unsigned count=calls==1u?456u:384u;
        for(unsigned i=0;i<count;++i)
            v->port->pixels[i>>3]=(uint8_t)(v->port->pixels[i>>3]|(uint8_t)(0x80u>>(i&7u)));
    }else v->port->pixels[0]=0x80;
    ++v->port->paints;return 1;
}
int main(void)
{
    cs_selected_draw_workspace *w=malloc(sizeof *w);cs_selected_draw_result r;
    CHECK(w);
    for(unsigned n=0;n<4u;++n){
        refuse=n<3u?n:0u;overlap=n==3u;calls=0;
        memset(w,0xCC,sizeof *w);memset(&r,0xA5,sizeof r);
        CHECK(cs_outline_draw(w,&r)==0 && !r.draw.success && !r.draw.value);
        for(size_t i=0;i<sizeof w->pixels;++i)CHECK(w->pixels[i]==0);
        CHECK(r.draw.phase==(overlap?4u:refuse==0u?3u:refuse));
        CHECK(r.selections==(refuse==1u?1u:2u));
        CHECK(r.current==(r.selections==1u?0x7000u:0x706Cu));
        CHECK(calls==(refuse?refuse:2u));
        if(refuse!=1u)CHECK(w->ports[0].pixels[0]&0x80u);
        if(overlap)CHECK(r.draw.black_pixels==456u && w->ports[1].paints==1u);
    }
    free(w);puts("outline failure phases/composite suppression/retained effects PASS");return 0;
}
