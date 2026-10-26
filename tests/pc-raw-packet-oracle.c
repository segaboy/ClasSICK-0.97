/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "platform/pc/ps2-mouse.h"
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define C(x) do {++checks;if(!(x)){printf("FAIL:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void)
{
    cs_ps2_mouse m;cs_ps2_mouse_packet p,before;unsigned h,x,y,cases=0;
    C(sizeof(p)==24);memset(&p,0xA5,sizeof(p));before=p;
    for(h=0;h<64;h++)if(h&8u)for(x=0;x<256;x++)for(y=0;y<256;y++){
        C(cs_ps2_mouse_begin(&m)==CS_PS2_MOUSE_MORE);
        C(cs_ps2_mouse_feed(&m,h,0,&p)==CS_PS2_MOUSE_MORE);C(memcmp(&p,&before,sizeof(p))==0);
        C(cs_ps2_mouse_feed(&m,x,0,&p)==CS_PS2_MOUSE_MORE);C(memcmp(&p,&before,sizeof(p))==0);
        C(cs_ps2_mouse_feed(&m,y,0,&p)==CS_PS2_MOUSE_PACKET);
        C(p.raw_header==h && p.raw_x==x && p.raw_y==y);before=p;cases++;
    }
    C(cs_ps2_mouse_begin(&m)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,0x2A,0,&p)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,0xFF,0,&p)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,1,0,&p)==CS_PS2_MOUSE_PACKET);
    C(p.dx==255 && p.dy==-255 && p.buttons==2 && p.raw_header==0x2A && p.raw_x==255 && p.raw_y==1);
    before=p;C(cs_ps2_mouse_begin(&m)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,0xC8,0,&p)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,23,0,&p)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,24,0,&p)==CS_PS2_MOUSE_DROPPED);C(memcmp(&p,&before,sizeof(p))==0);
    C(cs_ps2_mouse_feed(&m,8,0,&p)==CS_PS2_MOUSE_MORE);
    C(cs_ps2_mouse_feed(&m,5,1,&p)==CS_PS2_MOUSE_DROPPED);C(memcmp(&p,&before,sizeof(p))==0);
    C(cs_ps2_mouse_feed(&m,256,0,&p)==CS_PS2_MOUSE_ARGUMENT);C(memcmp(&p,&before,sizeof(p))==0);
    m.used=3;C(cs_ps2_mouse_feed(&m,0,0,&p)==CS_PS2_MOUSE_STATE);C(memcmp(&p,&before,sizeof(p))==0);
    printf("raw oracle:%u reports,%u assertions\n",cases,checks);return 0;
}
