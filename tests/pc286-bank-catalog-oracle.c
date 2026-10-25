/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen before worker artifact review. */
#include "probes/pc286/bank-catalog.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
static void ok(int yes,const char *why){++checks;if(!yes){++failures;fprintf(stderr,"FAIL %s\n",why);}}
int main(void)
{
    cs_bm_bank logical[6]={{1,32767},{2,1},{3,32767},{4,1},{5,32767},{6,32769}};
    cs_bm_region region={0,131072,3,0,6};cs_bm_map map={&region,logical,1,6};
    cs_bc_bank native[6]={{1,0x10000,0},{2,0x17fff,0},{3,0x18000,0},{4,0x1ffff,0},{5,0x20000,0},{6,0x27fff,0}};
    unsigned char before[sizeof native], lb[sizeof logical],rb[sizeof region];
    memcpy(before,native,sizeof native);memcpy(lb,logical,sizeof logical);memcpy(rb,&region,sizeof region);
    ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_OK,"128K exact owner extent");
    ok(!memcmp(before,native,sizeof native)&&!memcmp(lb,logical,sizeof logical)&&!memcmp(rb,&region,sizeof region),"valid no side effects");
    ok(cs_bc_validate(&map,0,6,0x10000,0x20000)==CS_BC_ARGUMENT,"null catalog first");
    ok(cs_bc_validate(0,native,6,0x10000,0x20000)==CS_BC_ARGUMENT,"null map");
    ok(cs_bc_validate(0,0,0,0,0)==CS_BC_ARGUMENT,"both null");
    ok(cs_bc_validate(&map,native,0,0,0)==CS_BC_BACKING,"zero count");
    ok(cs_bc_validate(&map,native,5,0x10000,0x20000)==CS_BC_BACKING,"missing bank");
    ok(cs_bc_validate(&map,native,7,0x10000,0x20000)==CS_BC_BACKING,"extra count before dereference");
    ok(cs_bc_validate(&map,native,6,0,0)==CS_BC_BACKING,"zero owner extent");
    ok(cs_bc_validate(&map,native,6,0x100000,1)==CS_BC_BACKING,"owner outside envelope");
    ok(cs_bc_validate(&map,native,6,0xFFFFFFFFUL,1)==CS_BC_BACKING,"huge owner base");
    ok(cs_bc_validate(&map,native,6,0x10000,0xFFFFFFFFUL)==CS_BC_BACKING,"owner size overflow");
    ok(cs_bc_validate(&map,native,6,0x10001,0x1ffff)==CS_BC_BACKING,"before owner start");
    ok(cs_bc_validate(&map,native,6,0x10000,0x1ffff)==CS_BC_BACKING,"one byte beyond owner end");
    native[5].physical_base=0xFFFFF;ok(cs_bc_validate(&map,native,6,0,0x100000)==CS_BC_BACKING,"20bit final span overflow");native[5].physical_base=0x27fff;
    native[5].physical_base=0xFFFFFFFFUL;ok(cs_bc_validate(&map,native,6,0,0x100000)==CS_BC_BACKING,"huge bank base");native[5].physical_base=0x27fff;
    native[1].id=1;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"duplicate backing identity");native[1].id=2;
    native[5].id=65;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"unmapped identity");native[5].id=6;
    native[1].physical_base=0x10000;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"distinct IDs same canonical base");
    native[1].physical_base=0x10001;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"contained bank");native[1].physical_base=0x17fff;
    native[5].physical_base=0x10010;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"large partial overlap");native[5].physical_base=0x27fff;
    native[5].native_offset=32767;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_OK,"exact native offset end");
    native[5].native_offset=32768;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"native offset wraps");
    native[5].native_offset=65536;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"native offset outside16");
    native[5].native_offset=0xFFFFFFFFUL;ok(cs_bc_validate(&map,native,6,0x10000,0x20000)==CS_BC_BACKING,"huge native offset");native[5].native_offset=0;
    logical[5].bytes=0;ok(cs_bc_validate(&map,native,5,0,0)==CS_BC_MAP,"late bad map before backing");
    ok(cs_bc_validate(&map,0,5,0,0)==CS_BC_ARGUMENT,"null catalog wins bad map");logical[5].bytes=32769;
    ok(!memcmp(before,native,sizeof native)&&!memcmp(lb,logical,sizeof logical)&&!memcmp(rb,&region,sizeof region),"refusals no side effects");
    region=(cs_bm_region){0,2,3,0,2};logical[0]=(cs_bm_bank){1,1};logical[1]=(cs_bm_bank){2,1};map.bank_count=2;
    native[0]=(cs_bc_bank){1,0xEFFFF,65535};native[1]=(cs_bc_bank){2,0xFFFFF,65535};
    ok(cs_bc_validate(&map,native,2,0xEFFFF,0x10001)==CS_BC_OK,"final byte1MiB and offset65535");
    native[1].physical_base=0x100000;ok(cs_bc_validate(&map,native,2,0,0x100000)==CS_BC_BACKING,"one byte after envelope");
    region=(cs_bm_region){0,65534,3,0,1};logical[0]=(cs_bm_bank){1,65534};map.bank_count=1;native[0]=(cs_bc_bank){1,0x10002,2};
    ok(cs_bc_validate(&map,native,1,0x10000,65536)==CS_BC_OK,"65534 byte native bank exact end");
    native[0].native_offset=3;ok(cs_bc_validate(&map,native,1,0x10000,65536)==CS_BC_BACKING,"65534 byte bank offset+1 wrap");
    printf("%u frozen checks, %u failures\n",checks,failures);return failures?1:0;
}
