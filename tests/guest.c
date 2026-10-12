/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/memory/guest.h"
#define REQUIRE(e) do {if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
static void maps(void)
{
    uint8_t a[8]={0},b[8]={0};cs_guest_region regions[2]={{a,a,0x100,8,3},{b,0,0x108,8,5}};
    cs_guest_map map={regions,2};REQUIRE(cs_guest_validate(&map)==CS_GUEST_OK);
    REQUIRE(cs_guest_validate(NULL)==CS_GUEST_ARGUMENT);
    for(unsigned n=0;n<14;++n) {
        cs_guest_region bad[2];memcpy(bad,regions,sizeof bad);cs_guest_map m={bad,2};
        switch(n) {
            case 0:m.count=0;break;case 1:m.count=9;break;case 2:m.regions=NULL;break;
            case 3:bad[0].bytes=NULL;break;case 4:bad[0].writable=NULL;break;
            case 5:bad[0].writable=b;break;case 6:bad[0].base=0x101;break;
            case 7:bad[0].size=7;break;case 8:bad[0].size=0;break;
            case 9:bad[0].base=0xFFFFFC;break;case 10:bad[1].base=0x106;break;
            case 11:bad[0].flags=0;break;case 12:bad[0].flags=2;break;
            default:bad[0].flags=9;break;
        }
        REQUIRE(cs_guest_validate(&m)==CS_GUEST_MAP);
    }
    cs_guest_region edge={a,a,0xFFFFF8,8,3};cs_guest_map m={&edge,1};
    REQUIRE(cs_guest_validate(&m)==CS_GUEST_OK);
    uint32_t value=0;REQUIRE(cs_guest_read(&m,0xFFFFFF,1,1,&value)==CS_GUEST_OK);
    REQUIRE(cs_guest_read(&m,0x1000000,1,1,&value)==CS_GUEST_ADDRESS);
}
static void accesses(void)
{
    uint8_t bytes[12];memset(bytes,0xCC,sizeof bytes);
    cs_guest_region r={bytes+2,bytes+2,0x100,8,3};cs_guest_map m={&r,1};
    REQUIRE(cs_guest_write(&m,0x100,4,0x11223344)==CS_GUEST_OK);
    REQUIRE(cs_guest_write(&m,0x104,2,0xAABB)==CS_GUEST_OK);
    REQUIRE(cs_guest_write(&m,0x107,1,0xEE)==CS_GUEST_OK);
    REQUIRE(bytes[0]==0xCC && bytes[1]==0xCC && bytes[2]==0x11 && bytes[3]==0x22 &&
        bytes[4]==0x33 && bytes[5]==0x44 && bytes[6]==0xAA && bytes[7]==0xBB &&
        bytes[8]==0xCC && bytes[9]==0xEE && bytes[10]==0xCC && bytes[11]==0xCC);
    uint32_t value=0;REQUIRE(cs_guest_read(&m,0x100,4,1,&value)==CS_GUEST_OK && value==0x11223344);
    REQUIRE(cs_guest_read(&m,0x102,4,1,&value)==CS_GUEST_OK && value==0x3344AABB);
    REQUIRE(cs_guest_read(&m,0x101,1,1,&value)==CS_GUEST_OK && value==0x22);
    for(unsigned n=0;n<7;++n) {
        uint32_t at=0x100;unsigned width=4;cs_guest_result expected=CS_GUEST_ADDRESS;
        switch(n) {case 0:at++;break;case 1:at=0xFE;break;case 2:at=0x106;break;
            case 3:at=0x1000100;break;case 4:width=3;expected=CS_GUEST_WIDTH;break;
            case 5:width=0;expected=CS_GUEST_WIDTH;break;default:at=UINT32_MAX;break;}
        uint8_t saved[12];memcpy(saved,bytes,12);value=0xDEADBEEF;
        REQUIRE(cs_guest_read(&m,at,width,1,&value)==expected && value==0xDEADBEEF);
        REQUIRE(cs_guest_write(&m,at,width,0)==expected && memcmp(saved,bytes,12)==0);
    }
    REQUIRE(cs_guest_read(&m,0x100,4,1,NULL)==CS_GUEST_ARGUMENT);
    REQUIRE(cs_guest_read(&m,0x100,4,4,&value)==CS_GUEST_ARGUMENT);
    REQUIRE(cs_guest_read(&m,0x100,4,9,&value)==CS_GUEST_ARGUMENT);
}
static void permissions(void)
{
    uint8_t a[8]={1,2,3,4,5,6,7,8},b[8]={0};
    cs_guest_region r[2]={{a,0,0x100,8,5},{b,b,0x108,8,3}};cs_guest_map m={r,2};
    uint32_t value=0xDEADBEEF;cs_guest_view v={b,b},old=v;
    REQUIRE(cs_guest_read(&m,0x100,2,5,&value)==CS_GUEST_OK && value==0x102);
    value=0xDEADBEEF;
    REQUIRE(cs_guest_read(&m,0x108,2,5,&value)==CS_GUEST_PERMISSION && value==0xDEADBEEF);
    REQUIRE(cs_guest_write(&m,0x100,2,0)==CS_GUEST_PERMISSION && a[0]==1 && a[1]==2);
    REQUIRE(cs_guest_read(&m,0x106,4,1,&value)==CS_GUEST_ADDRESS);
    REQUIRE(cs_guest_window(&m,0x106,4,1,&v)==CS_GUEST_ADDRESS && v.bytes==old.bytes && v.writable==old.writable);
    REQUIRE(cs_guest_window(&m,0x101,6,1,&v)==CS_GUEST_OK && v.bytes==a+1 && v.writable==NULL);
    REQUIRE(cs_guest_window(&m,0x108,8,2,&v)==CS_GUEST_OK && v.bytes==b && v.writable==b);
    REQUIRE(cs_guest_window(&m,0x108,0,2,&v)==CS_GUEST_ADDRESS);
    REQUIRE(cs_guest_window(&m,0x108,8,0,&v)==CS_GUEST_ARGUMENT);
    REQUIRE(cs_guest_window(&m,0x108,8,1,NULL)==CS_GUEST_ARGUMENT);
    r[1].base=0x106;value=0xDEADBEEF;
    REQUIRE(cs_guest_read(&m,0x100,2,1,&value)==CS_GUEST_MAP && value==0xDEADBEEF);
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(!strcmp(argv[1],"maps")) maps();
    else if(!strcmp(argv[1],"accesses")) accesses();
    else if(!strcmp(argv[1],"permissions")) permissions();
    else return 2;
    return 0;
}
