/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original cases, SPEC-0098. */
#include "../tools/vbox-viewer/motion.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c) do{++checks;if(!(c)){fprintf(stderr,"line %d: %s\n",__LINE__,#c);return 1;}}while(0)
int main(void)
{
    unsigned checks=0;cs_view_motion s={0},saved;cs_view_event e={0};int x,y,i;
    unsigned char h[16]={0x46,0x42,0x53,0x43,1,0,0,0,0,4,0,0,0,3,0,0};
    CHECK(cs_view_step(&s,100,100,1,0,1,0,&e)==0);
    x=100;y=100;
    for(i=0;i<1000;++i){
        int dx=(i%129)-64,dy=((i*47)%129)-64;x+=dx;y+=dy;
        CHECK(cs_view_step(&s,x,y,1,0,1,0,&e)==((dx||dy)?1:0));
        if(dx||dy)CHECK(e.dx==dx&&e.dy==dy&&!e.buttons);
    }
    CHECK(cs_view_step(&s,x,y,1,1,1,1,&e)==1&&e.buttons==1&&!e.dx&&!e.dy);
    CHECK(cs_view_step(&s,x+3,y-4,1,1,1,0,&e)==1&&e.dx==3&&e.dy==-4&&e.buttons==1);
    CHECK(cs_view_step(&s,x+3,y-4,1,1,1,0,&e)==0);
    CHECK(cs_view_step(&s,x+3,y-4,1,0,1,0,&e)==1&&e.buttons==0);
    CHECK(cs_view_step(&s,x+3,y-4,1,1,1,1,&e)==1);
    CHECK(cs_view_step(&s,x+100,y,1,1,1,0,&e)==2&&!e.buttons&&!e.dx&&!e.dy);
    CHECK(cs_view_step(&s,x,y,1,1,1,0,&e)==0);
    CHECK(cs_view_step(&s,x,y,1,0,1,0,&e)==0);
    CHECK(cs_view_step(&s,x,y,1,1,1,1,&e)==1);
    CHECK(cs_view_step(&s,x,y,1,1,0,0,&e)==2&&!e.buttons); /* stale/disabled */
    CHECK(cs_view_cancel(&s,&e)==0); /* no release flood */
    CHECK(cs_view_step(&s,x,y,1,1,1,0,&e)==0);
    CHECK(cs_view_step(&s,x,y,0,0,1,0,&e)==0);
    CHECK(cs_view_step(&s,x,y,1,1,1,1,&e)==1);
    CHECK(cs_view_cancel(&s,&e)==2&&!e.buttons); /* focus/capture loss */
    CHECK(cs_view_step(&s,x,y,1,1,1,1,&e)==0);
    CHECK(cs_view_step(&s,x,y,1,0,1,0,&e)==0);
    s=(cs_view_motion){0};
    CHECK(cs_view_step(&s,5,5,1,1,1,0,&e)==0&&s.suppressed); /* held entry */
    CHECK(cs_view_step(&s,5,5,1,0,1,0,&e)==0);
    CHECK(cs_view_step(&s,5,5,1,1,1,1,&e)==1);
    saved=s;CHECK(cs_view_step(&s,32768,0,1,0,1,0,&e)==-1&&!memcmp(&s,&saved,sizeof s));
    CHECK(cs_view_step(&s,0,-32769,1,0,1,0,&e)==-1);
    CHECK(cs_view_step(&s,0,0,1,0,1,1,&e)==-1);
    CHECK(cs_view_frame(h,CS_VIEW_FRAME_SIZE,0)==1);
    CHECK(!cs_view_frame(h,CS_VIEW_FRAME_SIZE,1));
    CHECK(!cs_view_frame(h,CS_VIEW_FRAME_SIZE-1,0));
    CHECK(!cs_view_frame(h,CS_VIEW_FRAME_SIZE+1,0));
    for(i=0;i<16;++i){unsigned char v=h[i];h[i]^=0x80;if(i<4||i>=8)CHECK(!cs_view_frame(h,CS_VIEW_FRAME_SIZE,0));h[i]=v;}
    h[4]=0;CHECK(!cs_view_frame(h,CS_VIEW_FRAME_SIZE,0));
    CHECK(!cs_view_frame(NULL,CS_VIEW_FRAME_SIZE,0));
    printf("PASS: %u viewer motion/header checks; no GUI/VM executed\n",checks);return 0;
}
