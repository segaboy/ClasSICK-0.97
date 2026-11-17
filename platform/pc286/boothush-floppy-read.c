/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "boothush-floppy-read.h"
#include "../../arch/i286/c16.h"

#if !defined(__WATCOMC__) || !defined(__I86__)
#error This provider requires the admitted Open Watcom native 16-bit target.
#endif
CS_C16_ASSERT(cs_floppy_near_data, sizeof(void *) == 2);
CS_C16_ASSERT(cs_floppy_near_code, sizeof(cs_block_read_one) == 2);

static uint16_t native_ds(void);
#pragma aux native_ds = "mov ax,ds" value [ax] modify exact [ax];

static uint16_t read_sector(uint16_t cx, uint16_t dx, uint8_t *buffer);
#pragma aux read_sector = \
    "pushf" "push ds" "push es" "push si" "push di" "push bp" \
    "push ds" "pop es" "mov ax,0201h" "int 13h" "sbb ax,ax" \
    "pop bp" "pop di" "pop si" "pop es" "pop ds" "popf" "cld" \
    parm [cx] [dx] [bx] value [ax] modify exact [ax bx cx dx];

int cs_pc286_floppy_buffer_fits(uint16_t segment, uint16_t offset)
{
    uint32_t physical = ((uint32_t)segment << 4) + offset;
    return offset <= 0xfe00u && physical <= 0x9fe00UL &&
        (physical & 0xffffUL) <= 0xfe00UL;
}

static int range_fits(uint32_t first_sector, uint32_t blocks)
{
    return blocks != 0 && first_sector < CS_PC286_FLOPPY_BLOCKS &&
        blocks <= (uint32_t)CS_PC286_FLOPPY_BLOCKS - first_sector;
}

int cs_pc286_floppy_bind(cs_pc286_floppy *provider, uint32_t first_sector,
    uint32_t blocks, cs_block_device *device)
{
    uint16_t segment = native_ds();
    if (provider == NULL || device == NULL || (void *)provider == (void *)device ||
        !range_fits(first_sector, blocks) ||
        !cs_pc286_floppy_buffer_fits(segment, (uint16_t)(uintptr_t)provider->bounce))
        return 0;
    provider->first_sector = (uint16_t)first_sector;
    provider->blocks = (uint16_t)blocks;
    provider->data_segment = segment;
    device->blocks = blocks;
    device->read = cs_pc286_floppy_read_one;
    device->context = provider;
    return 1;
}

cs_block_io_result cs_pc286_floppy_read_one(void *context, uint32_t lba,
    uint8_t *block)
{
    cs_pc286_floppy *provider = (cs_pc286_floppy *)context;
    uint16_t physical_sector, cylinder, head, sector, offset;
    if (provider == NULL || block == NULL || block == provider->bounce ||
        provider->data_segment != native_ds() ||
        !range_fits(provider->first_sector, provider->blocks) || lba >= provider->blocks ||
        !cs_pc286_floppy_buffer_fits(provider->data_segment,
            (uint16_t)(uintptr_t)provider->bounce))
        return CS_BLOCK_IO_ERROR;
    physical_sector = (uint16_t)(provider->first_sector + (uint16_t)lba);
    cylinder = (uint16_t)(physical_sector / 30u);
    head = (uint16_t)((physical_sector / 15u) % 2u);
    sector = (uint16_t)(physical_sector % 15u + 1u);
    if (read_sector((uint16_t)((cylinder << 8) | sector),
                    (uint16_t)(head << 8), provider->bounce) != 0)
        return CS_BLOCK_IO_ERROR;
    for (offset = 0; offset < CS_BLOCK_BYTES; ++offset)
        block[offset] = provider->bounce[offset];
    return CS_BLOCK_IO_OK;
}
