/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform/mac128k/polled-phase.h"
static unsigned checks;
#define CHECK(e) do { ++checks; if (!(e)) { printf("FAIL:%u:%s\n",(unsigned)__LINE__,#e); return 1; } } while (0)
/* Literal engineering transition table,independent of proposed decoder code. */
static const int table[4][4] = {{0,1,-1,9},{-1,0,9,1},{1,9,0,-1},{9,-1,1,0}};
static const uint32_t xpb[4]={0,0,16,16},ypb[4]={0,0,32,32},dcd[4]={0,8,0,8};
static uint32_t pb(unsigned x,unsigned y,unsigned b){return 0xC7u|xpb[x]|ypb[y]|(b?0u:8u);}
int main(void)
{
 cs_mac_polled_phase s,before;cs_pointer_sample o,old;
 unsigned x,y,nx,ny,b,nb,inv,i,cases=0;int tx,ty,dx,dy,kind;
 CHECK(sizeof s<=64);CHECK(CS_MAC_POLLED_VALID==0x504f4c31u);
 for(x=0;x<4;++x)for(y=0;y<4;++y)for(nx=0;nx<4;++nx)for(ny=0;ny<4;++ny)
 for(b=0;b<2;++b)for(nb=0;nb<2;++nb)for(inv=0;inv<4;++inv){
  (void)cs_mac_polled_begin(&s,inv);
  /* The brief fixes initialized state,not the successful begin return code. */
  CHECK(s.valid==CS_MAC_POLLED_VALID&&s.invert==inv&&s.have_phase==0);
  CHECK(s.services==0&&s.samples==0&&s.baselines==0&&s.losses==0);
  CHECK(cs_mac_polled_step(&s,pb(x,y,b),0x40u|dcd[x],0x44u|dcd[y],&o)==2);
  CHECK(o.dx==0&&o.dy==0&&o.buttons==b);
  CHECK(s.have_phase==1&&s.x_phase==x&&s.y_phase==y&&s.buttons==b);
  tx=table[x][nx];ty=table[y][ny];
  if(tx==9||ty==9){kind=3;dx=dy=0;}
  else{
   dx=((x^nx)&1u)?-tx:0;dy=((y^ny)&1u)?ty:0;
   if(inv&1u)dx=-dx;
   if(inv&2u)dy=-dy;
   kind=(dx||dy||b!=nb)?0:1;
  }
  CHECK(cs_mac_polled_step(&s,pb(nx,ny,nb),0x40u|dcd[nx],0x44u|dcd[ny],&o)==kind);
  CHECK(o.dx==dx&&o.dy==dy&&o.buttons==(kind==3?0u:nb));
  CHECK(s.services==2&&s.baselines==1&&s.samples==(kind==0?1u:0u)&&s.losses==(kind==3?1u:0u));
  if(kind==3){
   CHECK(s.have_phase==0);
   CHECK(cs_mac_polled_step(&s,pb(nx,ny,1),dcd[nx],dcd[ny],&o)==2);
   CHECK(o.dx==0&&o.dy==0&&o.buttons==1&&s.baselines==2&&s.samples==0);
  }else CHECK(s.have_phase==1&&s.x_phase==nx&&s.y_phase==ny&&s.buttons==nb);
  ++cases;
 }
 memset(&s,0xA5,sizeof s);before=s;
 CHECK(cs_mac_polled_begin(&s,4)==4&&memcmp(&s,&before,sizeof s)==0);
 CHECK(cs_mac_polled_begin(NULL,0)==4);
 (void)cs_mac_polled_begin(&s,0);(void)cs_mac_polled_step(&s,8,0,0,&o);
 for(i=0;i<6;++i){
  before=s;old=o;
  if(i==0)s.valid=0;else if(i==1)s.invert=4;else if(i==2)s.have_phase=2;
  else if(i==3)s.x_phase=4;else if(i==4)s.y_phase=4;else s.buttons=2;
  {cs_mac_polled_phase bad=s;
   CHECK(cs_mac_polled_step(&s,8,0,0,&o)==5&&memcmp(&s,&bad,sizeof s)==0&&memcmp(&o,&old,sizeof o)==0);
   CHECK(cs_mac_polled_step(&s,256,0,0,&o)==4&&memcmp(&s,&bad,sizeof s)==0&&memcmp(&o,&old,sizeof o)==0);
  }s=before;
 }
 before=s;old=o;
 CHECK(cs_mac_polled_step(NULL,8,0,0,&o)==4&&memcmp(&o,&old,sizeof o)==0);
 CHECK(cs_mac_polled_step(&s,8,0,0,NULL)==4&&memcmp(&s,&before,sizeof s)==0);
 CHECK(cs_mac_polled_step(&s,8,256,0,&o)==4&&memcmp(&s,&before,sizeof s)==0&&memcmp(&o,&old,sizeof o)==0);
 CHECK(cs_mac_polled_step(&s,8,0,256,&o)==4&&memcmp(&s,&before,sizeof s)==0&&memcmp(&o,&old,sizeof o)==0);
 CHECK(cs_mac_polled_step(&s,8,0,0,(cs_pointer_sample *)(void *)&s)==4&&memcmp(&s,&before,sizeof s)==0);
 CHECK(cs_mac_polled_step((cs_mac_polled_phase *)(uintptr_t)(UINTPTR_MAX-3u),8,0,0,&o)==4&&memcmp(&o,&old,sizeof o)==0);
 CHECK(cs_mac_polled_step(&s,8,0,0,(cs_pointer_sample *)(uintptr_t)(UINTPTR_MAX-3u))==4&&memcmp(&s,&before,sizeof s)==0);
 s.services=s.samples=s.baselines=s.losses=UINT32_MAX;
 CHECK(cs_mac_polled_step(&s,8,8,0,&o)==0&&o.dx==-1&&o.dy==0);
 CHECK(cs_mac_polled_step(&s,24,0,0,&o)==3);
 CHECK(cs_mac_polled_step(&s,16,0,0,&o)==2&&o.dx==0&&o.dy==0&&o.buttons==1);
 CHECK(s.services==UINT32_MAX&&s.samples==UINT32_MAX&&s.baselines==UINT32_MAX&&s.losses==UINT32_MAX);
 printf("polled phase oracle:%u cases,%u assertions\n",cases,checks);return 0;
}
