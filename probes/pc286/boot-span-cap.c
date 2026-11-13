/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original bounded BootHush diagnostic; fixed far windows are test fixtures,
 * not a production provider or proof of physical memory ownership. */
#include "span-cap.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This diagnostic requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_cap_boot_data_pointer_is_far, sizeof(uint8_t *) == 4);

static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(unsigned char code) { post(code); for (;;) { } }

void start(void)
{
    cs_bm_bank banks[2]; cs_bm_region region; cs_bm_map map;
    cs_bc_bank cat1[2], cat2[2]; cs_bs_span spans1[2], spans2[2];
    cs_bx_env env1, env2; cs_bq_registry registry = {0, 0, 0};
    cs_bq_owner owner; cs_bq_cap cap, stale, read_only; cs_bq_run run;
    uint16_t emitted; unsigned i;
    uint8_t *a = (uint8_t *)0x20000000UL, *b = (uint8_t *)0x30000000UL;
    uint8_t *c = (uint8_t *)0x40000000UL, *d = (uint8_t *)0x50000000UL;

    banks[0].id=1; banks[0].bytes=6; banks[1].id=2; banks[1].bytes=6;
    region.base=0x1000UL; region.bytes=12; region.flags=3;
    region.first_bank=0; region.bank_count=2;
    map.regions=&region; map.banks=banks; map.region_count=1; map.bank_count=2;
    cat1[0].id=1; cat1[0].physical_base=0x20000UL; cat1[0].native_offset=0;
    cat1[1].id=2; cat1[1].physical_base=0x30000UL; cat1[1].native_offset=0;
    cat2[0].id=1; cat2[0].physical_base=0x40000UL; cat2[0].native_offset=0;
    cat2[1].id=2; cat2[1].physical_base=0x50000UL; cat2[1].native_offset=0;
    spans1[0].bank=1; spans1[0].offset=0; spans1[0].bytes=6; spans1[0].data=a;
    spans1[1].bank=2; spans1[1].offset=0; spans1[1].bytes=6; spans1[1].data=b;
    spans2[0].bank=1; spans2[0].offset=0; spans2[0].bytes=6; spans2[0].data=c;
    spans2[1].bank=2; spans2[1].offset=0; spans2[1].bytes=6; spans2[1].data=d;
    env1.map=&map; env1.catalog=cat1; env1.spans=spans1;
    env1.ram_first=0x20000UL; env1.ram_bytes=0x10006UL;
    env1.current_epoch=1; env1.expected_epoch=1; env1.catalog_count=2; env1.span_count=2;
    env2.map=&map; env2.catalog=cat2; env2.spans=spans2;
    env2.ram_first=0x40000UL; env2.ram_bytes=0x10006UL;
    env2.current_epoch=1; env2.expected_epoch=1; env2.catalog_count=2; env2.span_count=2;
    for (i=0; i<6; ++i) { a[i]=0; b[i]=0; c[i]=0; d[i]=0; }
    a[3]=0x44; b[2]=0x88; c[3]=0x55; d[2]=0x99;

    if (cs_bq_registry_init(&registry, 2) != CS_BQ_OK ||
        cs_bq_owner_open(&registry, &env1, 4, &owner) != CS_BQ_OK ||
        cs_bq_open(&owner, 0x1004UL, 4, 3, &cap) != CS_BQ_OK) fail(0xd0);
    if (cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_OK || !emitted ||
        run.bank != 1 || run.offset != 4 || run.length != 2 || run.write == 0) fail(0xd1);
    run.write[0]=0xaa; run.write[1]=0xbb;
    if (cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_OK || !emitted ||
        run.bank != 2 || run.offset != 0 || run.length != 2 || run.write == 0) fail(0xd2);
    run.write[0]=0xcc; run.write[1]=0xdd;
    if (cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_OK || emitted != 0 ||
        a[3]!=0x44 || a[4]!=0xaa || a[5]!=0xbb || b[0]!=0xcc || b[1]!=0xdd || b[2]!=0x88) fail(0xd3);
    if (cs_bq_open(&owner, 0x1004UL, 4, 1, &read_only) != CS_BQ_OK) fail(0xd4);
#ifdef CS_CAP_MUTATE_PERMISSION
    if (cs_bq_borrow(&read_only, 3, 4, &run, &emitted) != CS_BQ_OK) fail(0xd5);
#else
    if (cs_bq_borrow(&read_only, 3, 4, &run, &emitted) != CS_BQ_PERMISSION) fail(0xd5);
#endif
    if (cs_bq_borrow(&read_only, 1, 4, &run, &emitted) != CS_BQ_OK || !emitted ||
        run.length != 2 || run.write != 0 || run.read[0] != 0xaa || run.read[1] != 0xbb) fail(0xd6);
    if (cs_bq_open(&owner, 0x1004UL, 4, 3, &stale) != CS_BQ_OK ||
        cs_bq_owner_rebind(&owner, &env2) != CS_BQ_OK) fail(0xd7);
#ifdef CS_CAP_MUTATE_STALE
    if (cs_bq_borrow(&stale, 3, 4, &run, &emitted) != CS_BQ_OK) fail(0xd8);
#else
    if (cs_bq_borrow(&stale, 3, 4, &run, &emitted) != CS_BQ_STALE) fail(0xd8);
#endif
    if (cs_bq_open(&owner, 0x1004UL, 4, 3, &cap) != CS_BQ_OK ||
        cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_OK || !emitted ||
        run.length != 2 || run.write == 0 || run.read[0] != 0 || run.read[1] != 0) fail(0xd9);
    run.write[0]=0x12; run.write[1]=0x34;
    if (cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_OK || !emitted || run.length != 2) fail(0xda);
    run.write[0]=0x56; run.write[1]=0x78;
    if (a[3]!=0x44 || a[4]!=0xaa || a[5]!=0xbb || b[0]!=0xcc || b[1]!=0xdd || b[2]!=0x88 ||
        c[3]!=0x55 || c[4]!=0x12 || c[5]!=0x34 || d[0]!=0x56 || d[1]!=0x78 || d[2]!=0x99) fail(0xdb);
    if (cs_bq_release(&cap) != CS_BQ_OK) fail(0xdc);
#ifdef CS_CAP_MUTATE_RELEASE
    if (cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_OK) fail(0xdd);
#else
    if (cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_CAPABILITY) fail(0xdd);
#endif
    if (cs_bq_open(&owner, 0x1004UL, 4, 3, &cap) != CS_BQ_OK ||
        cs_bq_owner_advance(&owner) != CS_BQ_OK ||
        cs_bq_borrow(&cap, 3, 4, &run, &emitted) != CS_BQ_STALE) fail(0xde);
    if (cs_bq_open(&owner, 0x1004UL, 4, 3, &stale) != CS_BQ_OK ||
        cs_bq_owner_open(&registry, &env2, 4, &owner) != CS_BQ_OK ||
        cs_bq_borrow(&stale, 3, 4, &run, &emitted) != CS_BQ_STALE) fail(0xdf);
    if (cs_bq_owner_open(&registry, &env2, 4, &owner) != CS_BQ_EXHAUSTED ||
        registry.current != 2UL) fail(0xe0);
    post(0xa7); for (;;) { }
}
