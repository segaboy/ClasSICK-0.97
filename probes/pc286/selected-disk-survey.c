/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original read-only diagnostic survey of a declared MFS volume through the
 * modeled INT13 provider. It observes metadata consistency; it is not a mount,
 * File Manager, boot or production directory/allocation implementation.
 * Two candidate 12-bit map packings are evaluated side by side so a lawful
 * black-box observation can discriminate them; neither is assumed here. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc286/boothush-floppy-read.h"
#include "../../core/storage/mfs-read.h"

#ifndef CS_SURVEY_FIRST_SECTOR
#define CS_SURVEY_FIRST_SECTOR 60u
#endif
#ifndef CS_SURVEY_BLOCKS
#define CS_SURVEY_BLOCKS 800u
#endif
#define CS_SURVEY_MAX_FILES 40u
#define CS_SURVEY_FILE_BYTES 64u
#define CS_SURVEY_HEADER_BYTES 112u
#define CS_SURVEY_BYTES (CS_SURVEY_HEADER_BYTES + CS_SURVEY_MAX_FILES * CS_SURVEY_FILE_BYTES)
#define CS_CHAIN_BAD_LINK 0xffffu
#define CS_CHAIN_LOOP 0xfffeu

/* Little-endian report found by its magic; layout documented in the harness. */
uint8_t cs_survey_report[CS_SURVEY_BYTES];

static cs_pc286_floppy provider;
static cs_block_device device;
static cs_mfs_workspace workspace;
static cs_mfs_volume volume;
static cs_mfs_entry entry;
static uint8_t master[1024], master_staging[1024];

static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];

static void put16(uint16_t at, uint16_t value)
{
    cs_survey_report[at] = (uint8_t)value;
    cs_survey_report[at + 1u] = (uint8_t)(value >> 8);
}

static void put32(uint16_t at, uint32_t value)
{
    put16(at, (uint16_t)value);
    put16((uint16_t)(at + 2u), (uint16_t)(value >> 16));
}

static uint16_t get16(uint16_t at)
{
    return (uint16_t)((uint16_t)cs_survey_report[at] |
        (uint16_t)((uint16_t)cs_survey_report[at + 1u] << 8));
}

static void inc16(uint16_t at)
{
    put16(at, (uint16_t)(get16(at) + 1u));
}

static void finish(uint8_t marker)
{
    post(marker);
    for (;;) { }
}

/* Map bytes begin at block 2 offset 64 and end with block 3: 960 bytes. */
static uint16_t map_entry(uint8_t packing, uint16_t index)
{
    const uint8_t *map = master + 64u;
    uint16_t at = (uint16_t)(index + (index >> 1));
    uint8_t first = map[at], second = map[at + 1u];
    if (packing == 0u) { /* hypothesis A: high nibble first, big-endian */
        if ((index & 1u) == 0u) return (uint16_t)(((uint16_t)first << 4) | (second >> 4));
        return (uint16_t)((((uint16_t)first & 0x0fu) << 8) | second);
    }
    /* hypothesis B: low nibble first, little-endian */
    if ((index & 1u) == 0u) return (uint16_t)(first | (((uint16_t)second & 0x0fu) << 8));
    return (uint16_t)((first >> 4) | ((uint16_t)second << 4));
}

/* Counts allocation blocks from start through the end marker (value 1). */
static uint16_t chain_length(uint8_t packing, uint16_t start, uint32_t *bytes)
{
    uint16_t steps = 0, current = start, next;
    *bytes = 0;
    for (;;) {
        if (current < 2u || current > (uint16_t)(volume.alloc_count + 1u)) return CS_CHAIN_BAD_LINK;
        if (steps >= volume.alloc_count) return CS_CHAIN_LOOP;
        ++steps;
        *bytes += volume.alloc_bytes;
        next = map_entry(packing, (uint16_t)(current - 2u));
        if (next == 1u) return steps;
        current = next;
    }
}

static void count_fork(uint8_t packing, uint16_t start, uint32_t physical, uint16_t at_chain)
{
    uint16_t base = (uint16_t)(78u + packing * 16u), length = 0;
    uint32_t bytes = 0;
    inc16(base);
    if (start == 0u) {
        inc16((uint16_t)(base + (physical == 0u ? 2u : 8u)));
        put16(at_chain, 0u);
        return;
    }
    length = chain_length(packing, start, &bytes);
    put16(at_chain, length);
    if (length == CS_CHAIN_BAD_LINK) base = (uint16_t)(base + 4u);
    else if (length == CS_CHAIN_LOOP) base = (uint16_t)(base + 6u);
    else if (bytes != physical) base = (uint16_t)(base + 8u);
    else base = (uint16_t)(base + 2u);
    inc16(base);
}

void start(void)
{
    uint16_t i, block, offset, files = 0, packing, value;
    cs_mfs_result status;
    for (i = 0; i < CS_SURVEY_BYTES; ++i) cs_survey_report[i] = 0;
    cs_survey_report[0] = 'C'; cs_survey_report[1] = 'S';
    cs_survey_report[2] = 'S'; cs_survey_report[3] = 'V';
    cs_survey_report[7] = 1u;
    if (!cs_pc286_floppy_bind(&provider, CS_SURVEY_FIRST_SECTOR, CS_SURVEY_BLOCKS, &device))
        finish(0xd0u);
    cs_survey_report[4] = 1u;
    status = cs_mfs_header_read(&device, &workspace, &volume);
    cs_survey_report[5] = (uint8_t)status;
    if (status != CS_MFS_OK) finish(0xbdu);
    cs_survey_report[4] = 2u;
    put32(8u, volume.created); put32(12u, volume.backup);
    put16(16u, volume.attributes); put16(18u, volume.file_count);
    put16(20u, volume.dir_first); put16(22u, volume.dir_blocks);
    put16(24u, volume.alloc_count); put32(26u, volume.alloc_bytes);
    put32(30u, volume.clump_bytes); put16(34u, volume.alloc_first);
    put32(36u, volume.next_file); put16(40u, volume.free_blocks);
    cs_survey_report[42] = volume.name_length;
    for (i = 0; i < 27u; ++i) cs_survey_report[43u + i] = volume.name[i];

    if (cs_block_read(&device, 2u, 2u, master, sizeof(master), master_staging,
            sizeof(master_staging)) != CS_BLOCK_OK) {
        cs_survey_report[6] = 1u;
        finish(0xbdu);
    }
    cs_survey_report[4] = 3u;
    for (packing = 0; packing < 2u; ++packing) {
        uint16_t base = (uint16_t)(78u + packing * 16u), used = 0, unused = 0, invalid = 0;
        for (i = 0; i < volume.alloc_count; ++i) {
            value = map_entry((uint8_t)packing, i);
            if (value == 0u) ++unused;
            else if (value == 1u || (value >= 2u && value <= (uint16_t)(volume.alloc_count + 1u))) ++used;
            else ++invalid;
        }
        put16((uint16_t)(base + 10u), used);
        put16((uint16_t)(base + 12u), unused);
        put16((uint16_t)(base + 14u), invalid);
    }

    /* Declared enumeration rule under test: entries are consecutive from each
     * block's start; the first inactive byte or an insufficient fixed region
     * ends that block. Agreement with the recorded count is checked, not assumed. */
    for (block = 0; block < volume.dir_blocks; ++block) {
        if (cs_block_read(&device, (uint32_t)volume.dir_first + block, 1u, workspace.block,
                sizeof(workspace.block), workspace.staging, sizeof(workspace.staging)) != CS_BLOCK_OK) {
            inc16(76u);
            continue;
        }
        offset = 0;
        for (;;) {
            if (offset > 510u) break;
            status = cs_mfs_entry_decode(workspace.block, sizeof(workspace.block), offset, &entry);
            if (status == CS_MFS_INACTIVE) break;
            if (status != CS_MFS_OK) {
                inc16(72u);
                break;
            }
            if (files < CS_SURVEY_MAX_FILES) {
                uint16_t at = (uint16_t)(CS_SURVEY_HEADER_BYTES + files * CS_SURVEY_FILE_BYTES);
                cs_survey_report[at] = entry.flags;
                cs_survey_report[at + 1u] = entry.name_length;
                put16((uint16_t)(at + 2u), entry.data_start);
                put32((uint16_t)(at + 4u), entry.data_physical);
                put32((uint16_t)(at + 8u), entry.data_logical);
                put16((uint16_t)(at + 12u), entry.resource_start);
                put32((uint16_t)(at + 14u), entry.resource_physical);
                put32((uint16_t)(at + 18u), entry.resource_logical);
                put32((uint16_t)(at + 30u), entry.file_id);
                for (packing = 0; packing < 2u; ++packing) {
                    count_fork((uint8_t)packing, entry.data_start, entry.data_physical,
                        (uint16_t)(at + 22u + packing * 4u));
                    count_fork((uint8_t)packing, entry.resource_start, entry.resource_physical,
                        (uint16_t)(at + 24u + packing * 4u));
                }
                for (i = 0; i < 30u && i < entry.name_length; ++i)
                    cs_survey_report[at + 34u + i] = entry.name[i];
            }
            ++files;
            offset = (uint16_t)(offset + entry.extent);
        }
        for (i = offset; i < 512u; ++i)
            if (workspace.block[i] != 0u) {
                inc16(74u);
                break;
            }
    }
    put16(70u, files);
    cs_survey_report[110] = (uint8_t)(files < CS_SURVEY_MAX_FILES ? files : CS_SURVEY_MAX_FILES);
    cs_survey_report[4] = 5u;
    finish(0xbcu);
}
