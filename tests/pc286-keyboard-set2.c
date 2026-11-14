/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc286/keyboard-set2.h"
#include <stdio.h>
#include <string.h>

static int feed(cs_pc286_set2_pointer *s,uint8_t b,int result,int32_t dx,int32_t dy,uint32_t buttons)
{
    cs_pointer_sample out={12345,-54321,0xabcdef01u};
    int r=cs_pc286_set2_feed(s,b,&out);
    if(r!=result)return 1;
    if(r==1)return out.dx!=dx || out.dy!=dy || out.buttons!=buttons;
    return out.dx!=12345 || out.dy!=-54321 || out.buttons!=0xabcdef01u;
}
#define CHECK(b,r,x,y,z) do {if(feed(&s,b,r,x,y,z))return __LINE__;} while(0)
int main(void)
{
    cs_pc286_set2_pointer s,before;cs_pointer_sample out={1,2,3};unsigned i;
    if(cs_pc286_set2_begin(NULL) || !cs_pc286_set2_begin(&s))return 1;
    CHECK(0x29,1,0,0,1);CHECK(0x29,0,0,0,0); /* repeat is not another press */
    CHECK(0xe0,0,0,0,0);CHECK(0x74,1,16,0,1);
    CHECK(0xe0,0,0,0,0);CHECK(0xf0,0,0,0,0);CHECK(0x74,0,0,0,0);
    CHECK(0xf0,0,0,0,0);CHECK(0x29,1,0,0,0); /* real break releases */
    CHECK(0xf0,0,0,0,0);CHECK(0x29,0,0,0,0); /* unmatched break */
    CHECK(0x6b,1,-16,0,0);CHECK(0x75,1,0,-16,0);CHECK(0x72,1,0,16,0);
    CHECK(0xe0,0,0,0,0);CHECK(0x29,0,0,0,0); /* extended Space ignored */
    CHECK(0xe1,0,0,0,0);for(i=0;i<7u;++i)CHECK(0x29,0,0,0,0);
    CHECK(0x29,1,0,0,1);
    before=s;
    if(cs_pc286_set2_feed(&s,0xff,&out)!=-1 || memcmp(&s,&before,sizeof(s)) ||
       out.dx!=1 || out.dy!=2 || out.buttons!=3)return 4;
    s.held=2;before=s;
    if(cs_pc286_set2_feed(&s,0x29,&out)!=-1 || memcmp(&s,&before,sizeof(s)) ||
       out.dx!=1 || out.dy!=2 || out.buttons!=3)return 2;
    s.held=0;
    if(cs_pc286_set2_feed(NULL,0x29,&out)!=-1 || cs_pc286_set2_feed(&s,0x29,NULL)!=-1 || s.held)return 3;
    if(!cs_pc286_set2_begin(&s))return 7;
    if(cs_pc286_set1_feed(&s,0x39,&out)!=1 || out.buttons!=1 ||
       cs_pc286_set1_feed(&s,0x39,&out)!=0 || s.held!=1)return 8;
    if(cs_pc286_set1_feed(&s,0xe0,&out)!=0 || cs_pc286_set1_feed(&s,0x4d,&out)!=1 ||
       out.dx!=16 || out.dy!=0 || out.buttons!=1)return 9;
    if(cs_pc286_set1_feed(&s,0xe0,&out)!=0 || cs_pc286_set1_feed(&s,0xcd,&out)!=0 ||
       cs_pc286_set1_feed(&s,0xb9,&out)!=1 || out.buttons!=0)return 10;
    if(cs_pc286_set1_feed(&s,0xb9,&out)!=0 || s.held!=0)return 11;
    puts("PASS set-1/set-2 make/break, repeat, movement, prefixes and refusal checks");return 0;
}
