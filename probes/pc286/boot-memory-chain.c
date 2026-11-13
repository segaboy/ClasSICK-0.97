/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original bounded diagnostic; fixed windows are not a production provider. */
#include "span-build.h"
#include "span-cap.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This diagnostic requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_chain_boot_data_pointer_is_far, sizeof(uint8_t *) == 4);
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(unsigned char code) { post(code); for (;;) { } }

void start(void)
{
    cs_bm_bank banks[2]; cs_bm_region region; cs_bm_map map;
    cs_bc_bank catalog[2]; cs_bk_chunk chunks[3]; cs_bs_span spans[3];
    cs_bx_env env; cs_bq_registry registry={0,0,0}; cs_bq_owner owner;
    cs_bq_cap cap, readable, stale; cs_bq_run run; uint16_t count=0, emitted=0;
    banks[0].id=1; banks[0].bytes=32768UL; banks[1].id=2; banks[1].bytes=6;
    region.base=0x1000UL; region.bytes=32774UL; region.flags=3;
    region.first_bank=0; region.bank_count=2;
    map.regions=&region; map.banks=banks; map.region_count=1; map.bank_count=2;
    catalog[0].id=1; catalog[0].physical_base=0x20000UL; catalog[0].native_offset=0;
    catalog[1].id=2; catalog[1].physical_base=0x30010UL; catalog[1].native_offset=16;
    if (cs_bk_plan(&map,catalog,2,0x20000UL,0x10016UL,chunks,3,&count)!=CS_BK_OK || count!=3 ||
        cs_bk_bind(chunks,count,0,spans,3)!=CS_BK_OK) fail(0xc0);
    env.map=&map; env.catalog=catalog; env.spans=spans;
    env.ram_first=0x20000UL; env.ram_bytes=0x10016UL;
    env.current_epoch=1; env.expected_epoch=1; env.catalog_count=2; env.span_count=3;
    spans[0].data[32765]=0x44; spans[2].data[2]=0x88;
    if (cs_bq_registry_init(&registry,2)!=CS_BQ_OK ||
        cs_bq_owner_open(&registry,&env,4,&owner)!=CS_BQ_OK ||
        cs_bq_open(&owner,0x8ffeUL,4,3,&cap)!=CS_BQ_OK) fail(0xc1);
    if (cs_bq_borrow(&cap,3,4,&run,&emitted)!=CS_BQ_OK || !emitted ||
        run.bank!=1 || run.offset!=32766UL || run.length!=1 || run.write==0) fail(0xc2);
    run.write[0]=0xaa;
    if (cs_bq_borrow(&cap,3,4,&run,&emitted)!=CS_BQ_OK || !emitted ||
        run.bank!=1 || run.offset!=32767UL || run.length!=1 || run.write==0) fail(0xc3);
    run.write[0]=0xbb;
    if (cs_bq_borrow(&cap,3,4,&run,&emitted)!=CS_BQ_OK || !emitted ||
        run.bank!=2 || run.offset!=0 || run.length!=2 || run.write==0) fail(0xc4);
    run.write[0]=0xcc; run.write[1]=0xdd;
    if (cs_bq_borrow(&cap,3,4,&run,&emitted)!=CS_BQ_OK || emitted!=0 ||
        cs_bq_open(&owner,0x8ffeUL,4,1,&readable)!=CS_BQ_OK) fail(0xc5);
#ifdef CS_CHAIN_MUTATE_PERMISSION
    if (cs_bq_borrow(&readable,3,4,&run,&emitted)!=CS_BQ_OK) fail(0xc6);
#else
    if (cs_bq_borrow(&readable,3,4,&run,&emitted)!=CS_BQ_PERMISSION) fail(0xc6);
#endif
    if (cs_bq_borrow(&readable,1,4,&run,&emitted)!=CS_BQ_OK || !emitted ||
        run.length!=1 || run.write!=0 || run.read[0]!=0xaa) fail(0xc7);
    if (cs_bq_open(&owner,0x8ffeUL,4,1,&stale)!=CS_BQ_OK ||
        cs_bq_owner_advance(&owner)!=CS_BQ_OK) fail(0xc8);
#ifdef CS_CHAIN_MUTATE_STALE
    if (cs_bq_borrow(&stale,1,4,&run,&emitted)!=CS_BQ_OK) fail(0xc9);
#else
    if (cs_bq_borrow(&stale,1,4,&run,&emitted)!=CS_BQ_STALE) fail(0xc9);
#endif
    post(0xa9); for (;;) { }
}
