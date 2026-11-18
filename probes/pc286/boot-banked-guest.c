/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/pc286/banked-guest.h"

static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(uint8_t code)
{
    post(code);
    for (;;) { }
}
static void zero(void *object, unsigned bytes)
{
    volatile uint8_t *p = (volatile uint8_t *)object;
    unsigned i;
    for (i = 0; i < bytes; ++i) p[i] = 0;
}
static void copy(void *destination, const void *source, unsigned bytes)
{
    volatile uint8_t *d = (volatile uint8_t *)destination;
    const volatile uint8_t *s = (const volatile uint8_t *)source;
    unsigned i;
    for (i = 0; i < bytes; ++i) d[i] = s[i];
}
static int equal(const void *a, const void *b, unsigned bytes)
{
    const volatile uint8_t *x = (const volatile uint8_t *)a;
    const volatile uint8_t *y = (const volatile uint8_t *)b;
    unsigned i;
    for (i = 0; i < bytes; ++i) if (x[i] != y[i]) return 0;
    return 1;
}
static volatile uint8_t __far *far_byte(uint32_t encoded)
{
    return (volatile uint8_t __far *)encoded;
}
static int exact_four(void)
{
    return *far_byte(0x2000ffffUL) == 0x11u && *far_byte(0x32000001UL) == 0x22u &&
        *far_byte(0x4200000fUL) == 0x33u && *far_byte(0x52001234UL) == 0x44u;
}

void start(void)
{
    cs_pc286_guest service, snapshot;
    cs_pg_region regions[2];
    cs_pg_bank banks[4];
    uint32_t generation = 0xdeadbeefUL, readback = 0xfeedfaceUL, old_generation;
    unsigned i;
    zero(&service, sizeof service);
    regions[0].base = 0x1000UL; regions[0].bytes = 0x20000UL;
    regions[0].flags = 7; regions[0].first_bank = 0; regions[0].bank_count = 4;
    for (i = 0; i < 4u; ++i) {
        banks[i].id = (uint16_t)(i + 1u); banks[i].segment = (uint16_t)(0x2000u + i * 0x1000u);
        banks[i].offset = 0; banks[i].bytes = (i & 1u) == 0u ? 65535u : 1u;
    }
    {
        cs_pg_result result = cs_pc286_guest_bind(&service, regions, 1, banks, 4,
            0x20000UL, 0x40000UL, &generation);
        if (result != CS_PG_OK) fail((uint8_t)(0xc0u + (unsigned)result));
        if (generation != 1UL) fail(0xd0);
    }
    /* Full 128 KiB logical geometry is real scattered RAM, not a near array. */
    *far_byte(0x20000000UL) = 0x51; *far_byte(0x2000fffdUL) = 0x52;
    *far_byte(0x2000ffffUL) = 0x53; *far_byte(0x30000001UL) = 0x54;
    *far_byte(0x40000002UL) = 0x55; *far_byte(0x50000001UL) = 0x56;
    if (cs_pc286_guest_write(&service, generation, 0x10ffeUL, 4, 0x11223344UL) != CS_PG_OK)
        fail(0xd1);
    if (*far_byte(0x2000fffeUL) != 0x11 || *far_byte(0x30000000UL) != 0x22 ||
        *far_byte(0x40000000UL) != 0x33 || *far_byte(0x40000001UL) != 0x44 ||
        *far_byte(0x20000000UL) != 0x51 || *far_byte(0x2000fffdUL) != 0x52 ||
        *far_byte(0x2000ffffUL) != 0x53 || *far_byte(0x30000001UL) != 0x54 ||
        *far_byte(0x40000002UL) != 0x55) fail(0xe1);
    if (cs_pc286_guest_read(&service, generation, 0x10ffeUL, 4, 5, &readback) != CS_PG_OK ||
        readback != 0x11223344UL) fail(0xd2);
    if (cs_pc286_guest_write(&service, generation, 0x10ffeUL, 2, 0xa1b2UL) != CS_PG_OK ||
        cs_pc286_guest_read(&service, generation, 0x10ffeUL, 2, 1, &readback) != CS_PG_OK ||
        readback != 0xa1b2UL || *far_byte(0x2000fffeUL) != 0xa1 ||
        *far_byte(0x30000000UL) != 0xb2) fail(0xd3);
    if (cs_pc286_guest_write(&service, generation, 0x20fffUL, 1, 0x5aUL) != CS_PG_OK ||
        cs_pc286_guest_read(&service, generation, 0x20fffUL, 1, 1, &readback) != CS_PG_OK ||
        readback != 0x5aUL || *far_byte(0x50000000UL) != 0x5a ||
        *far_byte(0x50000001UL) != 0x56) fail(0xd4);
    readback = 0xfeedfaceUL;
    if (cs_pc286_guest_read(&service, generation, 0x20ffeUL, 4, 1, &readback) != CS_PG_ADDRESS ||
        readback != 0xfeedfaceUL || cs_pc286_guest_write(&service, generation, 0x1001UL, 2, 0) != CS_PG_ADDRESS ||
        cs_pc286_guest_read(&service, generation, 0x1000UL, 3, 1, &readback) != CS_PG_WIDTH ||
        cs_pc286_guest_read(&service, generation, 0x1000UL, 1, 0x10001UL, &readback) != CS_PG_ARGUMENT)
        fail(0xd5);
    if (cs_pc286_guest_read(&service, generation, 0x1000000UL, 1, 1, &readback) != CS_PG_ADDRESS ||
        cs_pc286_guest_read(&service, generation, 0xfffffeUL, 4, 1, &readback) != CS_PG_ADDRESS ||
        readback != 0xfeedfaceUL) fail(0xe4);
    copy(&snapshot, &service, sizeof service);
    if (cs_pc286_guest_read(&snapshot, generation, 0x1000UL, 1, 1, &readback) != CS_PG_STATE ||
        readback != 0xfeedfaceUL) fail(0xd6);
    /* Caller descriptors are no longer live bindings. */
    banks[0].segment = 0xa000u;
    if (cs_pc286_guest_read(&service, generation, 0x1000UL, 1, 1, &readback) != CS_PG_OK ||
        readback != 0x51UL) fail(0xd7);
    if (cs_pc286_guest_retire(&service, generation) != CS_PG_OK ||
        cs_pc286_guest_write(&service, generation, 0x1000UL, 1, 0) != CS_PG_STALE) fail(0xd8);
    old_generation = generation;

    /* Four separately located one-byte banks, including offset ffff. */
    regions[0].base = 0x2000UL; regions[0].bytes = 4; regions[0].flags = 3;
    banks[0].segment = 0x2000; banks[0].offset = 0xffff;
    banks[1].segment = 0x3200; banks[1].offset = 1;
    banks[2].segment = 0x4200; banks[2].offset = 15;
    banks[3].segment = 0x5200; banks[3].offset = 0x1234;
    for (i = 0; i < 4u; ++i) banks[i].bytes = 1;
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_OK || generation != 2UL ||
        cs_pc286_guest_write(&service, generation, 0x2000UL, 4, 0x11223344UL) != CS_PG_OK ||
        !exact_four()) fail(0xd9);
    readback = 0xfeedfaceUL;
    if (cs_pc286_guest_write(&service, old_generation, 0x2000UL, 4, 0) != CS_PG_STALE ||
        cs_pc286_guest_read(&service, old_generation, 0x2000UL, 4, 1, &readback) != CS_PG_STALE ||
        readback != 0xfeedfaceUL || !exact_four()) fail(0xe2);
    if (cs_pc286_guest_read(&service, generation, 0x2000UL, 4, 1, &readback) != CS_PG_OK ||
        readback != 0x11223344UL) fail(0xda);
    /* Defensive corrupted-later-bank control: no earlier byte may publish. */
    service.banks[3].segment = 0xa000;
    readback = 0xfeedfaceUL;
    if (cs_pc286_guest_write(&service, generation, 0x2000UL, 4, 0) != CS_PG_BACKING ||
        cs_pc286_guest_read(&service, generation, 0x2000UL, 4, 1, &readback) != CS_PG_BACKING ||
        readback != 0xfeedfaceUL || !exact_four()) fail(0xe3);
    service.banks[3].segment = 0x5200;
    service.regions[0].flags = 1;
    if (cs_pc286_guest_write(&service, generation, 0x2000UL, 4, 0) != CS_PG_PERMISSION ||
        !exact_four()) fail(0xdb);
    service.regions[0].flags = 3;
    copy(&snapshot, &service, sizeof service);
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x20000UL, 0x40000UL,
        &generation) != CS_PG_STATE || !equal(&snapshot, &service, sizeof service)) fail(0xdc);
    if (cs_pc286_guest_retire(&service, generation) != CS_PG_OK) fail(0xdd);
    copy(&snapshot, &service, sizeof service);
    banks[3].segment = 0x4200; banks[3].offset = 15; /* Physical alias. */
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_BACKING || !equal(&snapshot, &service, sizeof service)) fail(0xde);
    banks[3].segment = 0x1000; banks[3].offset = 0; /* Native segment. */
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x10000UL, 0x50000UL,
            &generation) != CS_PG_BACKING || !equal(&snapshot, &service, sizeof service)) fail(0xdf);
    banks[3].segment = 0x5200; banks[3].offset = 0x1234;
    regions[0].bytes = 6; banks[0].bytes = 2; banks[3].bytes = 2;
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_BACKING || generation != 2UL ||
        !equal(&snapshot, &service, sizeof service) || !exact_four()) fail(0xe5);
    regions[0].bytes = 4; banks[0].bytes = 1; banks[3].bytes = 1;
    regions[0].base = 0xfffffeUL;
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_MAP || generation != 2UL ||
        !equal(&snapshot, &service, sizeof service) || !exact_four()) fail(0xe6);
    regions[0].base = 0x2000UL;
    regions[0].flags = 0x10003UL;
    if (cs_pc286_guest_bind(&service, regions, 1, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_MAP || !equal(&snapshot, &service, sizeof service)) fail(0xec);
    regions[0].flags = 3;
    /* Adjacent regions do not jointly satisfy one operand. */
    regions[0].bytes = 2; regions[0].bank_count = 2;
    regions[1] = regions[0]; regions[1].base = 0x2002UL; regions[1].first_bank = 2;
    if (cs_pc286_guest_bind(&service, regions, 2, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_OK || generation != 3UL) fail(0xed);
    readback = 0xfeedfaceUL;
    if (cs_pc286_guest_read(&service, generation, 0x2000UL, 4, 1, &readback) != CS_PG_ADDRESS ||
        cs_pc286_guest_write(&service, generation, 0x2000UL, 4, 0) != CS_PG_ADDRESS ||
        readback != 0xfeedfaceUL || !exact_four()) fail(0xef);
    if (cs_pc286_guest_retire(&service, generation) != CS_PG_OK) fail(0xf0);
    service.generation = 0xffffffffUL; /* Explicit saturation injection. */
    copy(&snapshot, &service, sizeof service);
    if (cs_pc286_guest_bind(&service, regions, 2, banks, 4, 0x20000UL, 0x40000UL,
            &generation) != CS_PG_EXHAUSTED || generation != 3UL ||
        !equal(&snapshot, &service, sizeof service) || !exact_four()) fail(0xf1);
    post(0xb8);
    for (;;) { }
}
