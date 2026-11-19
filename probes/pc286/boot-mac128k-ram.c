/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/pc286/mac128k-ram.h"
#ifdef CS_QUALIFY_RAM_BLOCKS
#include "../../platform/pc286/mac128k-transfer.h"
#endif
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void stop(uint8_t value){post(value);for(;;) { }}
#define CHECK(x,mark) do {if(!(x))stop(mark);} while(0)

void start(void)
{
    cs_pc286_m68k_memory m;
    volatile uint8_t *raw=(volatile uint8_t *)&m;
    cs_m68k_bus_backend mirror,strict;
    cs_pg_region regions[2]={{0,0x20000UL,7,0,4},{0x400000UL,2,1,4,1}};
    cs_pg_bank banks[5]={{1,0x3000,0,0x8000},{2,0x3a00,0,0x8000},
        {3,0x4400,0,0x8000},{4,0x4e00,0,0x8000},{5,0x5800,0,2}};
    uint32_t value;
    for(unsigned i=0;i<sizeof m;++i)raw[i]=0;
    CHECK(cs_pc286_guest_bind(&m.guest,regions,2,banks,5,0x30000UL,0x29000UL,&m.generation)==CS_PG_OK,0xd0);
    CHECK(cs_pc286_m68k_backend(&strict,&m)==CS_PG_OK && cs_pc286_mac128k_backend(&mirror,&m)==CS_PG_OK,0xd1);
    CHECK(strict.read(&m,0x3fff0UL,2,1,&value)==CS_GUEST_ADDRESS,0xd2);
    CHECK(mirror.write(&m,0x1fff0UL,4,0xdecafbadUL)==CS_GUEST_OK,0xd3);
    for(unsigned bank=0;bank<32u;++bank){
        uint32_t alias=(uint32_t)bank*0x20000UL;
        CHECK(mirror.read(&m,alias+0x1fff0UL,4,5,&value)==CS_GUEST_OK && value==0xdecafbadUL,0xe1);
        CHECK(mirror.write(&m,alias+0x7ffe,4,0x12340000UL+bank)==CS_GUEST_OK,0xd4);
        CHECK(strict.read(&m,0x7ffe,4,1,&value)==CS_GUEST_OK && value==0x12340000UL+bank,0xe2);
    }
    CHECK(mirror.span(&m,0,0x400000UL,7)==CS_GUEST_OK &&
        mirror.span(&m,0x3ffe00UL,512,3)==CS_GUEST_OK &&
        mirror.span(&m,0x3ffe00UL,513,3)==CS_GUEST_ADDRESS,0xd5);
    CHECK(mirror.write(&m,0x3fffeUL,4,0xabcdf123UL)==CS_GUEST_OK,0xd6);
    CHECK(strict.read(&m,0x1fffeUL,2,1,&value)==CS_GUEST_OK && value==0xabcd,0xe3);
    CHECK(strict.read(&m,0,2,1,&value)==CS_GUEST_OK && value==0xf123,0xe4);
    CHECK(mirror.read(&m,0x5fffeUL,4,5,&value)==CS_GUEST_OK && value==0xabcdf123UL,0xd7);
    CHECK(mirror.write(&m,0x3ffffeUL,4,0x12345678UL)==CS_GUEST_ADDRESS &&
        mirror.write(&m,0x3fffffUL,2,0x1234)==CS_GUEST_ADDRESS,0xe5);
    CHECK(mirror.write(&m,0x400000UL,2,0x1234)==CS_GUEST_PERMISSION &&
        mirror.read(&m,0x400000UL,2,1,&value)==CS_GUEST_OK,0xe6);
    CHECK(mirror.read(&m,0x400002UL,2,1,&value)==CS_GUEST_ADDRESS,0xe7);
    CHECK(strict.read(&m,0x1fffeUL,2,1,&value)==CS_GUEST_OK && value==0xabcd,0xd8);
    CHECK(strict.read(&m,0,2,1,&value)==CS_GUEST_OK && value==0xf123,0xd9);
#ifdef CS_QUALIFY_RAM_BLOCKS
    cs_m68k_transfer_backend batch;
    uint8_t source[64];
    for(unsigned n=0;n<64;++n)source[n]=(uint8_t)(0x80u+n);
    CHECK(cs_pc286_mac128k_transfer_backend(&batch,&m)==CS_PG_OK,0xc0);
    for(unsigned n=1;n<=64;++n){
        CHECK(batch.write_bytes(&m,0x5ffffUL,n,source)==CS_GUEST_OK,0xc1);
        CHECK(strict.read(&m,0x1ffffUL,1,1,&value)==CS_GUEST_OK && value==source[0],0xc2);
        if(n>1)CHECK(strict.read(&m,n-2,1,1,&value)==CS_GUEST_OK && value==source[n-1],0xc3);
    }
    CHECK(batch.write_bytes(&m,0x3fffffUL,2,source+1)==CS_GUEST_ADDRESS &&
        strict.read(&m,0x1ffffUL,1,1,&value)==CS_GUEST_OK && value==source[0],0xc4);
    CHECK(batch.write_bytes(&m,0x400000UL,2,source)==CS_GUEST_PERMISSION &&
        batch.write_bytes(&m,0x10000,0,source)==CS_GUEST_WIDTH &&
        batch.write_bytes(&m,0x10000,65,source)==CS_GUEST_WIDTH,0xc5);
    /* Preserve the established independent post-boot physical readback. */
    CHECK(mirror.write(&m,0x5fffeUL,4,0xabcdf123UL)==CS_GUEST_OK,0xc6);
#endif
    value=0x11223344UL;
    CHECK(mirror.read(&m,0x3ffffeUL,4,1,&value)==CS_GUEST_ADDRESS && value==0x11223344UL,0xda);
    CHECK(cs_pc286_guest_retire(&m.guest,m.generation)==CS_PG_OK,0xdb);
    CHECK(mirror.read(&m,0,2,1,&value)==CS_GUEST_MAP && value==0x11223344UL,0xdc);
    regions[0].flags=CS_PG_READ;
    CHECK(cs_pc286_guest_bind(&m.guest,regions,2,banks,5,0x30000UL,0x29000UL,&m.generation)==CS_PG_OK &&
        cs_pc286_mac128k_backend(&mirror,&m)==CS_PG_OK,0xdd);
    CHECK(mirror.write(&m,0x1fffeUL,4,0x33445566UL)==CS_GUEST_PERMISSION &&
        mirror.read(&m,0x1fffeUL,4,1,&value)==CS_GUEST_OK && value==0xabcdf123UL,0xde);
#ifdef CS_QUALIFY_RAM_BLOCKS
    CHECK(cs_pc286_mac128k_transfer_backend(&batch,&m)==CS_PG_OK &&
        batch.write_bytes(&m,0x3ffffUL,2,source)==CS_GUEST_PERMISSION &&
        mirror.read(&m,0x1fffeUL,4,1,&value)==CS_GUEST_OK && value==0xabcdf123UL,0xc7);
#endif
    CHECK(cs_pc286_guest_retire(&m.guest,m.generation)==CS_PG_OK,0xdf);
    regions[0].base=2;
    CHECK(cs_pc286_guest_bind(&m.guest,regions,2,banks,5,0x30000UL,0x29000UL,&m.generation)==CS_PG_OK &&
        cs_pc286_mac128k_backend(&mirror,&m)==CS_PG_MAP,0xe8);
    stop(0xb8);
}
