/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen before worker artifact review. */
#include "probes/pc286/bank-plan.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
static void ok(int yes,const char *why){++checks;if(!yes){++failures;fprintf(stderr,"FAIL %s\n",why);}}
static void piece(const cs_bm_plan *p,unsigned i,uint32_t bank,uint32_t offset,uint32_t length,const unsigned char *data)
{ok(p->pieces[i].bank==bank,"bank");ok(p->pieces[i].offset==offset,"offset");ok(p->pieces[i].length==length,"length");for(unsigned j=0;j<4;++j)ok(p->pieces[i].data[j]==data[j],"data/zero tail");}
static void refusal(const cs_bm_map *m,uint32_t at,cs_bm_result expected)
{cs_bm_plan out;unsigned char before[sizeof(out)];memset(&out,0xA5,sizeof(out));memcpy(before,&out,sizeof(out));ok(cs_bm_plan4(m,at,0x12345678UL,&out)==expected,"refusal result");ok(memcmp(before,&out,sizeof(out))==0,"refusal no effect");}
int main(void)
{
    cs_bm_bank banks[8]={{1,32767},{2,1},{3,32767},{4,1},{5,32767},{6,32769},{7,2},{8,2}};
    cs_bm_region regions[3]={{0,131072,3,0,6},{0,0,0,0,0},{0,0,0,0,0}};
    cs_bm_map map={regions,banks,1,6};cs_bm_plan p;
    const unsigned char a[4]={0x12,0,0,0},b[4]={0x34,0,0,0},c[4]={0x56,0x78,0,0},all[4]={0x12,0x34,0x56,0x78},zero[4]={0,0,0,0};
    ok(cs_bm_validate(&map)==CS_BM_OK,"128K geometry");
    ok(cs_bm_check(&map,0,131072,1)==CS_BM_OK,"large logical range");
    ok(cs_bm_check(&map,1,1,1)==CS_BM_OK,"byte range odd allowed");
    ok(cs_bm_plan4(&map,32766,0x12345678UL,&p)==CS_BM_OK,"three-bank plan");ok(p.count==3,"three pieces");
    piece(&p,0,1,32766,1,a);piece(&p,1,2,0,1,b);piece(&p,2,3,0,2,c);piece(&p,3,0,0,0,zero);
    ok(cs_bm_plan4(&map,65534,0x12345678UL,&p)==CS_BM_OK,"second boundary");ok(p.count==3,"second count");piece(&p,0,3,32766,1,a);piece(&p,1,4,0,1,b);piece(&p,2,5,0,2,c);
    ok(cs_bm_plan4(&map,131068,0x12345678UL,&p)==CS_BM_OK,"last long");ok(p.count==1,"last count");piece(&p,0,6,32765,4,all);for(unsigned i=1;i<4;++i)piece(&p,i,0,0,0,zero);
    refusal(&map,131070,CS_BM_ADDRESS);refusal(&map,1,CS_BM_ADDRESS);refusal(&map,0x1000000UL,CS_BM_ADDRESS);
    ok(cs_bm_check(&map,0,0,1)==CS_BM_ADDRESS,"zero length");ok(cs_bm_check(&map,0,1,0)==CS_BM_ARGUMENT,"zero rights");ok(cs_bm_check(&map,0,1,0x80000001UL)==CS_BM_ARGUMENT,"high access bit");
    banks[5].bytes=32768;refusal(&map,0,CS_BM_MAP);banks[5].bytes=32769;
    banks[5].id=1;refusal(&map,0,CS_BM_MAP);banks[5].id=6;
    banks[0].id=65;refusal(&map,0,CS_BM_MAP);banks[0].id=1;
    banks[0].bytes=0;refusal(&map,0,CS_BM_MAP);banks[0].bytes=65536;refusal(&map,0,CS_BM_MAP);banks[0].bytes=32767;
    regions[0].flags=0x80000003UL;refusal(&map,0,CS_BM_MAP);regions[0].flags=2;refusal(&map,0,CS_BM_MAP);
    regions[0].flags=1;refusal(&map,0,CS_BM_PERMISSION);ok(cs_bm_check(&map,0,1,2)==CS_BM_PERMISSION,"range permissions");regions[0].flags=7;ok(cs_bm_check(&map,0,1,4)==CS_BM_OK,"execute right");regions[0].flags=3;
    regions[0].first_bank=1;refusal(&map,0,CS_BM_MAP);regions[0].first_bank=0;
    map.bank_count=7;refusal(&map,0,CS_BM_MAP);map.bank_count=6;
    map.region_count=0;refusal(&map,0,CS_BM_MAP);map.region_count=9;refusal(&map,0,CS_BM_MAP);map.region_count=1;
    map.bank_count=0;refusal(&map,0,CS_BM_MAP);map.bank_count=65;refusal(&map,0,CS_BM_MAP);map.bank_count=6;
    ok(cs_bm_validate(0)==CS_BM_ARGUMENT,"null map");ok(cs_bm_check(0,0,1,1)==CS_BM_ARGUMENT,"null check");ok(cs_bm_plan4(0,0,0,0)==CS_BM_ARGUMENT,"null output precedence");
    regions[0]=(cs_bm_region){0xFFFFF0UL,16,3,0,1};banks[0]=(cs_bm_bank){7,16};map.bank_count=1;
    ok(cs_bm_plan4(&map,0xFFFFFCUL,0x12345678UL,&p)==CS_BM_OK,"24bit final long");piece(&p,0,7,12,4,all);refusal(&map,0xFFFFFEUL,CS_BM_ADDRESS);
    regions[0]=(cs_bm_region){0,2,3,0,1};regions[1]=(cs_bm_region){2,2,3,1,1};banks[0]=(cs_bm_bank){1,2};banks[1]=(cs_bm_bank){2,2};map.region_count=2;map.bank_count=2;
    ok(cs_bm_validate(&map)==CS_BM_OK,"adjacent geometry");refusal(&map,0,CS_BM_ADDRESS);ok(cs_bm_check(&map,0,4,1)==CS_BM_ADDRESS,"adjacent range refusal");
    regions[1].first_bank=0;refusal(&map,0,CS_BM_MAP);regions[1].first_bank=1;regions[1].base=0;refusal(&map,0,CS_BM_MAP);
    regions[0]=(cs_bm_region){0,4,3,0,4};map.region_count=1;map.bank_count=4;for(unsigned i=0;i<4;++i)banks[i]=(cs_bm_bank){i+1u,1};
    ok(cs_bm_plan4(&map,0,0x12345678UL,&p)==CS_BM_OK,"four banks");ok(p.count==4,"four pieces");
    for(unsigned i=0;i<4;++i){unsigned char byte[4]={all[i],0,0,0};piece(&p,i,i+1u,0,1,byte);}
    printf("%u frozen checks, %u failures\n",checks,failures);return failures?1:0;
}
