/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/storage/mac-boot-image.h"
#include <stdio.h>
#include <string.h>

static uint8_t disk[CS_MAC_BOOT_AREA_BYTES];
static int fail_lba = -1;
static unsigned reads;
static cs_block_io_result read_one(void *context, uint32_t lba, uint8_t *out)
{
    (void)context;
    ++reads;
    if (lba >= 2)
        return CS_BLOCK_IO_ERROR;
    memcpy(out, disk + (size_t)lba * CS_BLOCK_BYTES, CS_BLOCK_BYTES);
    return (int)lba == fail_lba ? CS_BLOCK_IO_ERROR : CS_BLOCK_IO_OK;
}

#define CHECK(test) do { if (!(test)) { fprintf(stderr, "boot image check failed at %d\n", __LINE__); return 1; } } while (0)

int main(void)
{
    cs_block_device device = {2, read_one, NULL};
    cs_mac_boot_workspace workspace;
    struct guarded { uint32_t before; cs_mac_boot_image image; uint32_t after; } output, saved;
    memset(&output, 0xa7, sizeof output);
    memset(&workspace, 0x6b, sizeof workspace);
    for (size_t i = 0; i < sizeof disk; ++i)
        disk[i] = (uint8_t)(i ^ (i >> 5));
    for (unsigned i = 0; i < 7; ++i)
        disk[10 + 16*i] = 0;
    disk[0] = 0x4c;
    disk[1] = 0x4b;
    CHECK(cs_mac_boot_image_load(&device, &workspace, &output.image) == CS_MAC_BOOT_OK);
    CHECK(reads == 2 && memcmp(disk, output.image.blocks, sizeof disk) == 0);
    CHECK(output.image.header.signature == 0x4c4b);
    CHECK(output.before == UINT32_C(0xa7a7a7a7) && output.after == output.before);
    memcpy(&saved, &output, sizeof output);

    /* The prior successful image must survive failures on a different disk. */
    memset(disk + 138, 0xc3, sizeof disk - 138);
    for (fail_lba = 0; fail_lba < 2; ++fail_lba) {
        reads = 0;
        CHECK(cs_mac_boot_image_load(&device, &workspace, &output.image) == CS_MAC_BOOT_IO);
        CHECK(reads == (unsigned)fail_lba + 1 && memcmp(&saved, &output, sizeof output) == 0);
    }
    fail_lba = -1;
    disk[10] = 16;
    CHECK(cs_mac_boot_image_load(&device, &workspace, &output.image) == CS_MAC_BOOT_NAME);
    CHECK(memcmp(&saved, &output, sizeof output) == 0);
    disk[1] = 0;
    CHECK(cs_mac_boot_image_load(&device, &workspace, &output.image) == CS_MAC_BOOT_SIGNATURE);
    CHECK(memcmp(&saved, &output, sizeof output) == 0);
    device.blocks = 1;
    reads = 0;
    CHECK(cs_mac_boot_image_load(&device, &workspace, &output.image) == CS_MAC_BOOT_RANGE);
    CHECK(cs_mac_boot_image_load(NULL, &workspace, &output.image) == CS_MAC_BOOT_ARGUMENT);
    CHECK(cs_mac_boot_image_load(&device, NULL, &output.image) == CS_MAC_BOOT_ARGUMENT);
    CHECK(cs_mac_boot_image_load(&device, &workspace, NULL) == CS_MAC_BOOT_ARGUMENT);
    CHECK(cs_mac_boot_image_load(&device, &workspace, (cs_mac_boot_image *)(void *)&workspace) == CS_MAC_BOOT_ARGUMENT);
    CHECK(!reads && memcmp(&saved, &output, sizeof output) == 0);
    puts("Boot image preservation and full-sector publication passed.");
    return 0;
}
