/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc286/boothush-floppy-read.h"
#include "../../core/storage/mfs-read.h"

static cs_pc286_floppy provider;
static cs_block_device device;
static cs_mfs_workspace workspace;
static struct { uint8_t before; cs_mfs_volume value; uint8_t after; } volume;
static struct { uint8_t before; cs_mfs_entry value; uint8_t after; } entry;
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(uint8_t code) { post(code); for (;;) { } }

void start(void)
{
    uint16_t i;
    uint8_t *bytes = (uint8_t *)&volume;
    for (i = 0; i < sizeof(volume); ++i) bytes[i] = 0xa5u;
    bytes = (uint8_t *)&entry;
    for (i = 0; i < sizeof(entry); ++i) bytes[i] = 0xc3u;
    bytes = (uint8_t *)&workspace;
    for (i = 0; i < sizeof(workspace); ++i) bytes[i] = 0x6cu;
    if (!cs_pc286_floppy_bind(&provider, 60u, 128u, &device)) fail(0xd0u);
#ifdef CS_FLOPPY_MFS_EXPECT_FAULT
    if (cs_mfs_header_read(&device, &workspace, &volume.value) != CS_MFS_IO) fail(0xd1u);
    bytes = (uint8_t *)&volume;
    for (i = 0; i < sizeof(volume); ++i) if (bytes[i] != 0xa5u) fail(0xd2u);
    bytes = (uint8_t *)&workspace;
    for (i = 0; i < sizeof(workspace); ++i) if (bytes[i] != 0x6cu) fail(0xd2u);
    post(0xbbu);
#else
    if (cs_mfs_header_read(&device, &workspace, &volume.value) != CS_MFS_OK) fail(0xd1u);
    if (volume.value.created != 0x01234567UL || volume.value.backup != 0x89abcdefUL ||
        volume.value.attributes != 0x8081u || volume.value.file_count != 3u ||
        volume.value.dir_first != 4u || volume.value.dir_blocks != 2u ||
        volume.value.alloc_count != 31u || volume.value.alloc_bytes != 1024u ||
        volume.value.clump_bytes != 1536u || volume.value.alloc_first != 6u ||
        volume.value.next_file != 0x10203040UL || volume.value.free_blocks != 7u ||
        volume.value.name_length != 5u || volume.value.name[0] != 'L' ||
        volume.value.name[1] != 'a' || volume.value.name[2] != 'b' ||
        volume.value.name[3] != 0u || volume.value.name[4] != 0x80u) fail(0xd3u);
    for (i = 5; i < sizeof(volume.value.name); ++i)
        if (volume.value.name[i] != 0) fail(0xd3u);
    /* This is a known block/offset, not an enumeration or map traversal. */
    if (cs_block_read(&device, 4u, 1u, workspace.block, sizeof(workspace.block),
            workspace.staging, sizeof(workspace.staging)) != CS_BLOCK_OK ||
        cs_mfs_entry_decode(workspace.block, sizeof(workspace.block), 20u, &entry.value) != CS_MFS_OK)
        fail(0xd4u);
    if (entry.value.offset != 20u || entry.value.extent != 60u ||
        entry.value.flags != 0xd1u || entry.value.version != 0x37u ||
        entry.value.file_id != 0x89abcdefUL || entry.value.data_start != 0u ||
        entry.value.data_logical != 0u || entry.value.data_physical != 0u ||
        entry.value.resource_start != 0u || entry.value.resource_logical != 0u ||
        entry.value.resource_physical != 0u || entry.value.created != 0xdeadbeefUL ||
        entry.value.modified != 0x76543210UL || entry.value.name_length != 8u ||
        entry.value.name[0] != 'N' || entry.value.name[1] != 'a' || entry.value.name[2] != 't' ||
        entry.value.name[3] != 'i' || entry.value.name[4] != 'v' || entry.value.name[5] != 'e' ||
        entry.value.name[6] != 0 || entry.value.name[7] != 0xffu) fail(0xd5u);
    for (i = 0; i < 16u; ++i)
        if (entry.value.finder[i] != (uint8_t)(0xf0u ^ (i * 7u))) fail(0xd5u);
    for (i = 8; i < sizeof(entry.value.name); ++i)
        if (entry.value.name[i] != 0) fail(0xd5u);
    if (volume.before != 0xa5u || volume.after != 0xa5u ||
        entry.before != 0xc3u || entry.after != 0xc3u) fail(0xd6u);
    post(0xbau);
#endif
    for (;;) { }
}
