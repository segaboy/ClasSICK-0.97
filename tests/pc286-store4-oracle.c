/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Literal expectations frozen before worker implementation. */
#include "store4-model.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define C(x) do{checks++;if(!(x)){failures++;printf("FAIL line %d case %s\n",__LINE__,name);}}while(0)
typedef struct {const char *name;uint32_t at;int result;uint8_t bytes[8];} test_case;
static const test_case cases[]={
{"within0",0UL,0,{18,52,86,120,170,170,170,170}},
{"cross2",2UL,0,{170,170,18,52,86,120,170,170}},
{"within4",4UL,0,{170,170,170,170,18,52,86,120}},
{"odd1",1UL,5,{170,170,170,170,170,170,170,170}},
{"past6",6UL,5,{170,170,170,170,170,170,170,170}},
{"odd7",7UL,5,{170,170,170,170,170,170,170,170}},
{"max24",16777214UL,5,{170,170,170,170,170,170,170,170}},
{"outside24",16777216UL,5,{170,170,170,170,170,170,170,170}},
{"lastspan_zero",2UL,7,{170,170,170,170,170,170,170,170}},
{"lastspan_null",2UL,7,{170,170,170,170,170,170,170,170}},
{"span_gap",2UL,7,{170,170,170,170,170,170,170,170}},
{"span_overlap",2UL,7,{170,170,170,170,170,170,170,170}},
{"span_wrong_id",2UL,7,{170,170,170,170,170,170,170,170}},
{"extra_span",2UL,7,{170,170,170,170,170,170,170,170}},
{"span_32768",2UL,7,{170,170,170,170,170,170,170,170}},
{"stale",2UL,4,{170,170,170,170,170,170,170,170}},
{"zero_current",2UL,1,{170,170,170,170,170,170,170,170}},
{"zero_expected",2UL,1,{170,170,170,170,170,170,170,170}},
{"no_write",2UL,6,{170,170,170,170,170,170,170,170}},
{"high_flags",2UL,2,{170,170,170,170,170,170,170,170}},
{"bad_backing",2UL,3,{170,170,170,170,170,170,170,170}},
{"null_map",2UL,1,{170,170,170,170,170,170,170,170}},
{"null_catalog",2UL,1,{170,170,170,170,170,170,170,170}},
{"null_spans",2UL,1,{170,170,170,170,170,170,170,170}},
{"too_many_spans",2UL,1,{170,170,170,170,170,170,170,170}},
{"bad_map_stale",2UL,2,{170,170,170,170,170,170,170,170}},
{"bad_backing_stale",2UL,3,{170,170,170,170,170,170,170,170}},
{"stale_bad_address",1UL,4,{170,170,170,170,170,170,170,170}},
{"bad_address_binding",1UL,5,{170,170,170,170,170,170,170,170}},
{"permission_binding",2UL,6,{170,170,170,170,170,170,170,170}}
};
int main(void){unsigned t;const char *name="initial";
 for(t=0;t<sizeof cases/sizeof cases[0];t++){
  cs_bm_bank banks[2]={{1,3},{2,5}};cs_bm_region region={0,8,3,0,2};
  cs_bm_map map={&region,banks,1,2};cs_bc_bank cat[2]={{1,0x10000,65533},{2,0x20000,0}};
  uint8_t data[8];cs_bs_span spans[5];uint16_t n=4;uint32_t cur=3,expected=3;
  const cs_bm_map *mp=&map;const cs_bc_bank *cp=cat;const cs_bs_span *sp=spans;
  const test_case *c=&cases[t];cs_bs_result got;
  memset(data,170,sizeof data);name=c->name;
  spans[0]=(cs_bs_span){1,0,2,data};spans[1]=(cs_bs_span){1,2,1,data+2};
  spans[2]=(cs_bs_span){2,0,1,data+3};spans[3]=(cs_bs_span){2,1,4,data+4};spans[4]=spans[3];
  if(!strcmp(name,"lastspan_zero"))spans[3].bytes=0;
  if(!strcmp(name,"lastspan_null"))spans[3].data=NULL;
  if(!strcmp(name,"span_gap"))spans[3].offset=2;
  if(!strcmp(name,"span_overlap"))spans[3].offset=0;
  if(!strcmp(name,"span_wrong_id"))spans[3].bank=1;
  if(!strcmp(name,"extra_span"))n=5;
  if(!strcmp(name,"span_32768"))spans[3].bytes=32768UL;
  if(!strcmp(name,"stale")||!strcmp(name,"bad_map_stale")||!strcmp(name,"bad_backing_stale")||!strcmp(name,"stale_bad_address"))cur=4;
  if(!strcmp(name,"zero_current"))cur=0;
  if(!strcmp(name,"zero_expected"))expected=0;
  if(!strcmp(name,"no_write")||!strcmp(name,"permission_binding"))region.flags=1;
  if(!strcmp(name,"high_flags"))region.flags=0x80000003UL;
  if(!strcmp(name,"bad_backing")||!strcmp(name,"bad_backing_stale"))cat[1].physical_base=0x10001;
  if(!strcmp(name,"null_map"))mp=NULL;
  if(!strcmp(name,"null_catalog"))cp=NULL;
  if(!strcmp(name,"null_spans"))sp=NULL;
  if(!strcmp(name,"too_many_spans"))n=193;
  if(!strcmp(name,"bad_map_stale"))banks[1].bytes=0;
  if(!strcmp(name,"bad_address_binding")||!strcmp(name,"permission_binding"))spans[3].bytes=0;
  got=cs_bs_store4(mp,cp,2,0x10000,0x20000,sp,n,cur,expected,c->at,0x12345678UL);
  C((int)got==c->result);C(memcmp(data,c->bytes,8)==0);
 }
 /* Post-review regressions,distinct from the thirty frozen expectations. */
 {cs_bm_bank b={1,8};cs_bm_region rg={0,8,3,0,1};cs_bm_map m={&rg,&b,1,1};cs_bc_bank cat={1,0x10000,0};
 uint8_t d[8],saved[8];cs_bs_span ss[2]={{1,0,4,d},{1,4,4,NULL}};uint32_t next=0xAABBCCDDUL;
 memset(d,170,8);memcpy(saved,d,8);name="late-unrelated-null";
 C(cs_bs_store4(&m,&cat,1,0x10000,8,ss,2,1,1,0,0x12345678UL)==CS_BS_BINDING);C(memcmp(d,saved,8)==0);
 name="epoch";C(cs_bs_epoch_next(UINT32_MAX,&next)==CS_BS_EXHAUSTED);C(next==0xAABBCCDDUL);
 C(cs_bs_epoch_next(0,&next)==CS_BS_OK);C(next==1);C(cs_bs_epoch_next(1,NULL)==CS_BS_ARGUMENT);
 }
 {cs_bm_bank banks[64];cs_bc_bank cat[64];cs_bs_span spans[192];cs_bm_region rg={0,192,3,0,64};
 cs_bm_map m={&rg,banks,1,64};uint8_t d[192],want[192];unsigned i,j;
 for(i=0;i<64;i++){banks[i]=(cs_bm_bank){i+1,3};cat[i]=(cs_bc_bank){i+1,0x10000+4*i,0};
  for(j=0;j<3;j++)spans[3*i+j]=(cs_bs_span){i+1,j,1,d+3*i+j};}
 memset(d,170,sizeof d);memset(want,170,sizeof want);want[188]=18;want[189]=52;want[190]=86;want[191]=120;
 name="192-spans";C(cs_bs_store4(&m,cat,64,0x10000,256,spans,192,UINT32_MAX,UINT32_MAX,188,0x12345678UL)==CS_BS_OK);
 C(memcmp(d,want,sizeof d)==0);
 }
 printf("store4 oracle: %u assertions,%u failures;30 frozen cases plus post-review regressions\n",checks,failures);return failures?1:0;
}
