/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original differential noninterference test.
 * Scripted IO only;not a physical latch/timing model. */
#include "motion.h"
#include <stdio.h>
#include <string.h>
int cs_ref_prepare(struct cs_mac_motion*,const cs_mac_motion_io*,cs_window_inbox*,uint32_t);
int cs_ref_arm(struct cs_mac_motion*);
int cs_ref_service(struct cs_mac_motion*);
typedef struct {uint8_t pb[3],a[3],b[3],rr3,regs[2][16];unsigned n,reads,writes;uint32_t log[16];} hw;
typedef struct {hw h;cs_window_inbox in;struct cs_mac_motion m;} test;
static uint8_t rd(void *p,uint32_t ch,uint32_t reg){hw*h=p;h->reads++;if(reg==3)return h->rr3;if(reg==0)return(ch?h->b:h->a)[h->n-1];return h->regs[ch][reg];}
static void wr(void *p,uint32_t ch,uint32_t reg,uint8_t v){hw*h=p;if(h->writes<16)h->log[h->writes]=ch<<16|reg<<8|v;h->writes++;h->regs[ch][reg]=v;}
static uint8_t pb(void*p){hw*h=p;h->reads++;return h->pb[h->n++];}
static int init(test*t,int ref){cs_mac_motion_io io;int v;memset(t,0,sizeof *t);t->h.pb[0]=t->h.pb[1]=t->h.pb[2]=8;io=(cs_mac_motion_io){&t->h,rd,wr,pb};
 if(cs_inbox_begin(&t->in)!=CS_INBOX_OK)return 0;
 v=ref?cs_ref_prepare(&t->m,&io,&t->in,0):cs_mac_motion_prepare(&t->m,&io,&t->in,0);if(v!=CS_MAC_MOTION_OK)return 0;
 v=ref?cs_ref_arm(&t->m):cs_mac_motion_arm(&t->m);if(v!=CS_MAC_MOTION_OK)return 0;
 v=ref?cs_ref_service(&t->m):cs_mac_motion_service(&t->m);if(v!=CS_MAC_MOTION_BASELINED)return 0;
 if(cs_inbox_claim(&t->in,1)!=CS_INBOX_BASELINE_ITEM||cs_inbox_complete(&t->in,1,0)!=CS_INBOX_OK)return 0;
 t->h.n=t->h.reads=t->h.writes=0;memset(t->h.log,0,sizeof t->h.log);return 1;}
static void inputs(test*t,unsigned code){unsigned i;static const uint8_t rr[4]={0,1,8,9};for(i=0;i<3;i++){
 t->h.pb[i]=(uint8_t)((((code>>(2*i))&3u)<<4)|(((code>>(6+i))&1u)<<3));
 t->h.a[i]=(uint8_t)(((code>>(9+2*i))&1u)<<3);t->h.b[i]=(uint8_t)(((code>>(10+2*i))&1u)<<3);}
 t->h.rr3=rr[(code>>15)&3u];}
static int state_equal(const struct cs_mac_motion*a,const struct cs_mac_motion*b){struct cs_mac_motion x,y;memcpy(&x,a,sizeof x);memcpy(&y,b,sizeof y);x.io.cookie=y.io.cookie=NULL;x.inbox=y.inbox=NULL;return memcmp(&x,&y,sizeof x)==0;}
int main(void){unsigned code,changed=0;test a,b;int ar,br;struct cs_mac_motion saved;cs_window_inbox in_before;
 for(code=0;code<131072u;code++){
  if(!init(&a,1)||!init(&b,0)){puts("init failure");return 1;}inputs(&a,code);inputs(&b,code);
  memcpy(&saved,&b.m,sizeof saved);saved.services++;memcpy(&in_before,&b.in,sizeof in_before);
  ar=cs_ref_service(&a.m);br=cs_mac_motion_service(&b.m);
  if((code>>15)==0u&&((code>>9)&63u)==0u&&((code>>6)&7u)==7u&&!(b.h.pb[0]==b.h.pb[1]&&b.h.pb[1]==b.h.pb[2])){
   changed++;if(ar!=CS_MAC_MOTION_LOST||a.m.first_loss.reason!=CS_MAC_LOSS_UNSTABLE||br!=CS_MAC_MOTION_EMPTY||memcmp(&saved,&b.m,sizeof saved)||memcmp(&in_before,&b.in,sizeof in_before)||b.h.reads!=10||b.h.writes!=1||b.h.log[0]!=0x38u){printf("idle failure %u\n",code);return 1;}
  }else if(ar!=br||!state_equal(&a.m,&b.m)||memcmp(&a.in,&b.in,sizeof a.in)||memcmp(&a.h,&b.h,sizeof a.h)){printf("noninterference failure %u base%d probe%d\n",code,ar,br);return 1;}
 }
 if(changed!=60){printf("wrong changed count %u\n",changed);return 1;}
#if CLASSICK_MAC_PENDING_DEFER_PROBE
 /* A live defer continuation is checked before the new rule. */
 if(!init(&a,1)||!init(&b,0))return 1;
 inputs(&a,7u<<6|1u);inputs(&b,7u<<6|1u);
 a.m.defer_pending=b.m.defer_pending=8;a.m.defer_snapshot=b.m.defer_snapshot=0x80800u;
 ar=cs_ref_service(&a.m);br=cs_mac_motion_service(&b.m);
 if(ar!=CS_MAC_MOTION_LOST||ar!=br||!state_equal(&a.m,&b.m)){puts("defer continuation failure");return 1;}
#endif
 printf("phase matrix:131072 transcripts,60 exact no-decision changes,defer%d;all other results,state and IO identical\n",CLASSICK_MAC_PENDING_DEFER_PROBE);return 0;}
