/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* A bounded original 16-bit diagnostic, not a production RAM provider. */
#include "span-build.h"
#include "span-access.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This diagnostic requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_build_boot_data_pointer_is_far, sizeof(uint8_t *) == 4);
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(unsigned char code) { post(code); for (;;) { } }

void start(void)
{
    cs_bm_bank banks[2]; cs_bm_region region; cs_bm_map map;
    cs_bc_bank catalog[2]; cs_bk_chunk chunks[3], saved;
    cs_bs_span spans[3]; cs_bx_env env; uint8_t *saved_ptr;
    uint16_t count = 0x5a5a; uint32_t value = 0;
    banks[0].id=1; banks[0].bytes=32768UL;
    banks[1].id=2; banks[1].bytes=6;
    region.base=0x1000UL; region.bytes=32774UL; region.flags=3;
    region.first_bank=0; region.bank_count=2;
    map.regions=&region; map.banks=banks; map.region_count=1; map.bank_count=2;
    catalog[0].id=1; catalog[0].physical_base=0x20000UL; catalog[0].native_offset=0;
    catalog[1].id=2; catalog[1].physical_base=0x30010UL; catalog[1].native_offset=16;
    chunks[0].bank=0xa5a5a5a5UL;
    if (cs_bk_plan(&map,catalog,2,0x20000UL,0x10016UL,chunks,2,&count) != CS_BK_CAPACITY ||
        count != 0x5a5au || chunks[0].bank != 0xa5a5a5a5UL) fail(0xb0);
    if (cs_bk_plan(&map,catalog,2,0x20000UL,0x10016UL,chunks,3,&count) != CS_BK_OK ||
        count != 3 || chunks[0].bank != 1 || chunks[0].offset != 0 ||
        chunks[0].bytes != 32767UL || chunks[1].bank != 1 ||
        chunks[1].offset != 32767UL || chunks[1].bytes != 1 ||
        chunks[2].bank != 2 || chunks[2].offset != 0 || chunks[2].bytes != 6 ||
        chunks[2].segment != 0x3000 || chunks[2].native_offset != 16) fail(0xb1);
    spans[0].bank=0x5a5a5a5aUL;
    if (cs_bk_bind(chunks,3,0,spans,2) != CS_BK_CAPACITY || spans[0].bank != 0x5a5a5a5aUL) fail(0xb2);
    if (cs_bk_bind(chunks,3,0,spans,3) != CS_BK_OK || spans[0].data == 0 ||
        spans[1].data == 0 || spans[2].data == 0) fail(0xb3);
    saved=chunks[2]; chunks[2].segment=0; chunks[2].native_offset=0;
    if (cs_bk_bind(chunks,3,0,spans,3) != CS_BK_REPRESENTATION ||
        spans[0].bank != 1 || spans[2].bank != 2) fail(0xb4);
    chunks[2]=saved;
    env.map=&map; env.catalog=catalog; env.spans=spans;
    env.ram_first=0x20000UL; env.ram_bytes=0x10016UL;
    env.current_epoch=1; env.expected_epoch=1; env.catalog_count=2; env.span_count=3;
    spans[0].data[32765]=0x44; spans[2].data[2]=0x88;
    if (cs_bx_write(&env,0x8ffeUL,4,0xaabbccddUL) != CS_BX_OK ||
        cs_bx_read(&env,0x8ffeUL,4,1,&value) != CS_BX_OK || value != 0xaabbccddUL) fail(0xb6);
    saved_ptr=spans[2].data; spans[2].data=0;
#ifdef CS_BUILD_MUTATE_ATOMIC
    if (cs_bx_write(&env,0x8ffeUL,4,0x10203040UL) != CS_BX_OK ||
#else
    if (cs_bx_write(&env,0x8ffeUL,4,0x10203040UL) != CS_BX_BINDING ||
#endif
        spans[0].data[32766]!=0xaa || spans[1].data[0]!=0xbb ||
        saved_ptr[0]!=0xcc || saved_ptr[1]!=0xdd) fail(0xb7);
    spans[2].data=saved_ptr;
    env.expected_epoch=2; value=0x12345678UL;
#ifdef CS_BUILD_MUTATE_EPOCH
    if (cs_bx_read(&env,0x8ffeUL,4,1,&value) != CS_BX_OK || value != 0x12345678UL) fail(0xb8);
#else
    if (cs_bx_read(&env,0x8ffeUL,4,1,&value) != CS_BX_STALE || value != 0x12345678UL) fail(0xb8);
#endif
    env.expected_epoch=1;
    if (cs_bx_write(&env,0x8fffUL,2,0) != CS_BX_ADDRESS ||
        spans[1].data[0]!=0xbb || spans[2].data[0]!=0xcc) fail(0xb9);
#ifdef CS_BUILD_MUTATE_BOUNDARY
    if (spans[1].data[0] != 0xbc) fail(0xb5);
#else
    if (spans[1].data[0] != 0xbb) fail(0xb5);
#endif
    post(0xa8); for (;;) { }
}
