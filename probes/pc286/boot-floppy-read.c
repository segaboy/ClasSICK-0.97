/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc286/boothush-floppy-read.h"

static cs_pc286_floppy provider;
static cs_block_device device;
static uint8_t output[1026], staging[1026];
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static uint16_t flags_word(void);
#pragma aux flags_word = "pushf" "pop ax" value [ax] modify exact [ax];
static uint16_t ds_word(void);
#pragma aux ds_word = "mov ax,ds" value [ax] modify exact [ax];
static void fail(uint8_t code) { post(code); for (;;) { } }

static uint8_t expected(uint16_t sector, uint16_t offset)
{
    return (uint8_t)(sector ^ (uint16_t)(offset * 13u) ^ (offset >> 1) ^ 0x5au);
}

static void reset_buffers(void)
{
    uint16_t i;
    for (i = 0; i < sizeof(output); ++i) {
        output[i] = 0xa5u;
        staging[i] = 0x6cu;
    }
}

static int filled(const uint8_t *buffer, uint16_t bytes, uint8_t value)
{
    uint16_t i;
    for (i = 0; i < bytes; ++i) if (buffer[i] != value) return 0;
    return 1;
}

static int sector_matches(const uint8_t *buffer, uint16_t sector)
{
    uint16_t i;
    for (i = 0; i < CS_BLOCK_BYTES; ++i)
        if (buffer[i] != expected(sector, i)) return 0;
    return 1;
}

static void read_ok(uint16_t logical, uint16_t count)
{
    uint16_t i, before = flags_word();
    reset_buffers();
    if (cs_block_read(&device, logical, count, output + 1, 1024,
            staging + 1, 1024) != CS_BLOCK_OK) fail(0xe1u);
    if (ds_word() != 0x1000u || (flags_word() & 0x0600u) != (before & 0x0200u))
        fail(0xe7u);
    for (i = 0; i < count; ++i)
        if (!sector_matches(output + 1 + i * CS_BLOCK_BYTES,
                (uint16_t)(60u + logical + i))) fail(0xe2u);
    if (output[0] != 0xa5u || staging[0] != 0x6cu ||
        !filled(output + 1 + count * CS_BLOCK_BYTES,
            (uint16_t)(1025u - count * CS_BLOCK_BYTES), 0xa5u) ||
        !filled(staging + 1 + count * CS_BLOCK_BYTES,
            (uint16_t)(1025u - count * CS_BLOCK_BYTES), 0x6cu)) fail(0xe3u);
}

void start(void)
{
    uint16_t segment;
    if (ds_word() != 0x1000u) fail(0xe7u);
    /* Pure numeric boundary oracles: no fabricated pointer is dereferenced. */
    if (!cs_pc286_floppy_buffer_fits(0x1000u, 0xfe00u) ||
        cs_pc286_floppy_buffer_fits(0x1000u, 0xfe01u) ||
        !cs_pc286_floppy_buffer_fits(0x1001u, 0xfdf0u) ||
        cs_pc286_floppy_buffer_fits(0x1001u, 0xfdf1u) ||
        !cs_pc286_floppy_buffer_fits(0x9fc0u, 0x0200u) ||
        cs_pc286_floppy_buffer_fits(0x9fc0u, 0x0201u) ||
        cs_pc286_floppy_buffer_fits(0xffffu, 0u)) fail(0xe8u);
    provider.first_sector = 123u;
    provider.blocks = 456u;
    provider.data_segment = 789u;
    device.blocks = 9u;
    device.read = NULL;
    device.context = NULL;
    if (cs_pc286_floppy_bind(NULL, 60u, 2340u, &device) ||
        cs_pc286_floppy_bind(&provider, 60u, 2340u, NULL) ||
        cs_pc286_floppy_bind(&provider, 60u, 0u, &device) ||
        cs_pc286_floppy_bind(&provider, 2400u, 1u, &device) ||
        cs_pc286_floppy_bind(&provider, 2399u, 2u, &device) ||
        cs_pc286_floppy_bind(&provider, 0xffffffffUL, 1u, &device) ||
        cs_pc286_floppy_bind(&provider, 0u, 0xffffffffUL, &device) ||
        provider.first_sector != 123u || provider.blocks != 456u ||
        provider.data_segment != 789u || device.blocks != 9u ||
        device.read != NULL || device.context != NULL) fail(0xe9u);
    if (!cs_pc286_floppy_bind(&provider, 60u, 2340u, &device) ||
        device.blocks != 2340u || device.context != &provider ||
        device.read != cs_pc286_floppy_read_one) fail(0xe9u);
    reset_buffers();
    if (cs_pc286_floppy_read_one(NULL, 0, staging + 1) != CS_BLOCK_IO_ERROR ||
        cs_pc286_floppy_read_one(&provider, 0, NULL) != CS_BLOCK_IO_ERROR ||
        cs_pc286_floppy_read_one(&provider, 0, provider.bounce) != CS_BLOCK_IO_ERROR ||
        cs_pc286_floppy_read_one(&provider, 2340u, staging + 1) != CS_BLOCK_IO_ERROR ||
        cs_pc286_floppy_read_one(&provider, 0xffffffffUL, staging + 1) != CS_BLOCK_IO_ERROR ||
        cs_block_read(&device, 2339u, 2u, output + 1, 1024,
            staging + 1, 1024) != CS_BLOCK_RANGE ||
        !filled(output, sizeof(output), 0xa5u) ||
        !filled(staging, sizeof(staging), 0x6cu)) fail(0xe5u);
    segment = provider.data_segment;
    provider.data_segment = (uint16_t)(segment + 1u);
    if (cs_pc286_floppy_read_one(&provider, 0, staging + 1) != CS_BLOCK_IO_ERROR ||
        !filled(staging, sizeof(staging), 0x6cu)) fail(0xe5u);
    provider.data_segment = segment;
    read_ok(0u, 1u);       /* C2:H0:S1, nonzero native DS. */
    read_ok(2339u, 1u);    /* C79:H1:S15, final medium sector. */
#ifdef CS_FLOPPY_EXPECT_FAULT
    reset_buffers();
    if (cs_block_read(&device, 14u, 2u, output + 1, 1024,
            staging + 1, 1024) != CS_BLOCK_IO) fail(0xe4u);
    if (!filled(output, sizeof(output), 0xa5u) ||
        !sector_matches(staging + 1, 74u) ||
        staging[0] != 0x6cu || !filled(staging + 513, 513, 0x6cu)) fail(0xe6u);
    post(0xb9u);
#else
    read_ok(14u, 2u);      /* Head transition: physical sectors74/75. */
    read_ok(29u, 2u);      /* Cylinder transition: sectors89/90. */
    post(0xb8u);
#endif
    for (;;) { }
}
