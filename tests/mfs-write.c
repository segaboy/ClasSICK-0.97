/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/storage/mfs-write.h"
#include <stdio.h>
#include <string.h>

/* Original 32-sector volume. Data allocations 2, 4, 3 give LBAs 8, 10, 9.
 * The resource fork occupies allocation 5 (LBA 11). */
enum { BLOCKS = 32, SECTOR = 512 };
static uint8_t disk[BLOCKS][SECTOR], original[BLOCKS][SECTOR], expected[BLOCKS][SECTOR];
static unsigned checks, reads, writes;
static uint32_t fail_read, fail_write;
static cs_block_io_result failure;
static const unsigned data_sectors[] = {8, 10, 9};
static cs_mfsv_workspace work;
static cs_mfsv_volume volume, saved_volume;
static cs_mfs_entry entry, saved_entry;
static uint8_t input[1100], saved_input[1100];
static cs_mfsw_progress progress;

#define CHECK(predicate) do { ++checks; if (!(predicate)) { \
    fprintf(stderr,"line %d: %s\n",__LINE__,#predicate); return 1; } } while (0)

static cs_block_io_result read_one(void *context, uint32_t lba, uint8_t *block)
{
    (void)context;
    ++reads;
    if (lba >= BLOCKS) return CS_BLOCK_IO_UNAVAILABLE;
    if (lba == fail_read) {
        memset(block, 0xc3, 57);
        return failure;
    }
    memcpy(block, disk[lba], SECTOR);
    return CS_BLOCK_IO_OK;
}

static cs_block_io_result write_one(void *context, uint32_t lba, const uint8_t *block)
{
    (void)context;
    ++writes;
    if (lba >= BLOCKS) return CS_BLOCK_IO_UNAVAILABLE;
    if (lba == fail_write) {
        memset(disk[lba], 0x6e, 37); /* A torn write can affect unrequested bytes. */
        return failure;
    }
    memcpy(disk[lba], block, SECTOR);
    return CS_BLOCK_IO_OK;
}

static void initialize(void)
{
    unsigned i, j;
    for (i = 0; i < BLOCKS; ++i)
        for (j = 0; j < SECTOR; ++j)
            disk[i][j] = (uint8_t)(25u*i + 11u*j + j/7u);
    memset(disk[2], 0, SECTOR);
    memset(disk[3], 0, SECTOR);
    disk[2][0] = 0xd2; disk[2][1] = 0xd7;
    disk[2][13] = 1; disk[2][15] = 4; disk[2][17] = 1; disk[2][19] = 4;
    disk[2][22] = 2; disk[2][26] = 2; disk[2][29] = 8; disk[2][33] = 2;
    disk[2][36] = 1; disk[2][37] = 'V';
    /* Four packed 12-bit links: 004, 001, 003, 001. */
    disk[2][65] = 0x40; disk[2][66] = 1; disk[2][68] = 0x30; disk[2][69] = 1;
    memset(disk[4], 0, SECTOR);
    disk[4][0] = 0x80; disk[4][21] = 1; disk[4][23] = 2;
    disk[4][26] = 5; disk[4][27] = 1; disk[4][30] = 6;
    disk[4][33] = 5; disk[4][37] = 41; disk[4][40] = 2;
    disk[4][50] = 1; disk[4][51] = 'F';
    memcpy(original, disk, sizeof disk);
    for (i = 0; i < sizeof input; ++i) input[i] = (uint8_t)(i*37u + i/13u);
    memcpy(saved_input, input, sizeof input);
    fail_read = fail_write = UINT32_MAX;
}

static void reset_transfer(void)
{
    reads = writes = 0;
    fail_read = fail_write = UINT32_MAX;
    memcpy(disk, original, sizeof disk);
    memcpy(expected, original, sizeof expected);
    memset(&progress, 0xaa, sizeof progress);
}

static void expect_data(unsigned offset, unsigned count)
{
    unsigned i;
    for (i = 0; i < count; ++i)
        expected[data_sectors[(offset+i)/SECTOR]][(offset+i)%SECTOR] = input[i];
}

int main(void)
{
    cs_block_device reader = {BLOCKS, read_one, NULL};
    cs_block_writer writer = {BLOCKS, write_one, NULL};
    cs_mfsw_progress sentinel;
    cs_mfsv_result expected_result[] = {CS_MFSV_UNAVAILABLE, CS_MFSV_IO, CS_MFSV_PROVIDER};
    cs_block_io_result errors[] = {CS_BLOCK_IO_UNAVAILABLE, CS_BLOCK_IO_ERROR, (cs_block_io_result)99};
    unsigned i;
    initialize();
    CHECK(cs_mfsv_mount(&reader, &work, &volume) == CS_MFSV_OK);
    CHECK(cs_mfsv_find(&volume, &work, (const uint8_t *)"F", 1, &entry) == CS_MFSV_OK);
    memcpy(&saved_volume, &volume, sizeof volume);
    memcpy(&saved_entry, &entry, sizeof entry);
#define OVERWRITE(fork,offset,count,cap) \
    cs_mfsw_fork_overwrite(&volume,&work,&entry,fork,&writer,offset,count,input,cap,&progress)
    reset_transfer();
    CHECK(OVERWRITE(CS_MFSV_DATA,500,700,sizeof input) == CS_MFSV_OK);
    expect_data(500,700);
    CHECK(progress.completed == 700 && progress.phase == CS_MFSW_DONE && !progress.lba && !progress.uncertain);
    CHECK(reads == 2 && writes == 3);
    CHECK(OVERWRITE(CS_MFSV_RESOURCE,1,40,40) == CS_MFSV_OK);
    memcpy(expected[11]+1, input, 40);
    CHECK(memcmp(disk, expected, sizeof disk) == 0);
    reset_transfer();
    CHECK(OVERWRITE(CS_MFSV_DATA,512,512,512) == CS_MFSV_OK);
    expect_data(512,512);
    CHECK(reads == 0 && writes == 1 && memcmp(disk, expected, sizeof disk) == 0);
    reset_transfer();
    memcpy(&sentinel, &progress, sizeof progress);
    CHECK(OVERWRITE(CS_MFSV_DATA,UINT32_MAX,1,1) == CS_MFSV_RANGE);
    CHECK(OVERWRITE(CS_MFSV_DATA,1,5,4) == CS_MFSV_CAPACITY);
    CHECK(OVERWRITE(CS_MFSV_DATA,0,1,32768) == CS_MFSV_ARGUMENT);
    --writer.blocks;
    CHECK(OVERWRITE(CS_MFSV_RESOURCE,0,1,1) == CS_MFSV_ARGUMENT);
    ++writer.blocks;
    volume.map[2] = 0; /* Corrupt the final link beyond the request. */
    CHECK(OVERWRITE(CS_MFSV_DATA,0,1,1) == CS_MFSV_CHAIN);
    CHECK(OVERWRITE(CS_MFSV_DATA,entry.data_logical,0,0) == CS_MFSV_CHAIN);
    volume.map[2] = saved_volume.map[2];
    CHECK(memcmp(&sentinel,&progress,sizeof progress) == 0);
    CHECK(OVERWRITE(CS_MFSV_DATA,entry.data_logical,0,0) == CS_MFSV_OK);
    CHECK(progress.completed == 0 && progress.phase == CS_MFSW_DONE && !progress.lba && !progress.uncertain);
    CHECK(reads == 0 && writes == 0 && memcmp(disk, original, sizeof disk) == 0);
    for (i = 0; i < sizeof errors/sizeof errors[0]; ++i) {
        reset_transfer();
        failure = errors[i];
        fail_read = 9;
        CHECK(OVERWRITE(CS_MFSV_DATA,500,700,700) == expected_result[i]);
        expect_data(500,524);
        CHECK(progress.phase == CS_MFSW_READ && !progress.uncertain && progress.completed == 524 && progress.lba == 9);
        CHECK(reads == 2 && writes == 2 && memcmp(disk,expected,sizeof disk) == 0);
        reset_transfer();
        fail_write = 8;
        CHECK(OVERWRITE(CS_MFSV_DATA,500,700,700) == expected_result[i]);
        memset(expected[8], 0x6e, 37);
        CHECK(progress.phase == CS_MFSW_WRITE && progress.uncertain == 1 && progress.completed == 0 && progress.lba == 8);
        CHECK(reads == 1 && writes == 1 && memcmp(disk,expected,sizeof disk) == 0);
    }
    CHECK(memcmp(&volume,&saved_volume,sizeof volume) == 0 && memcmp(&entry,&saved_entry,sizeof entry) == 0);
    CHECK(memcmp(input,saved_input,sizeof input) == 0);
    for (i = 0; i < 3; ++i) {
        reset_transfer();
        if (i < 2) disk[2][10+i] = 0x80; /* One volume lock bit at a time. */
        else disk[4][0] |= 1;             /* The directory entry's file lock. */
        CHECK(cs_mfsv_mount(&reader,&work,&volume) == CS_MFSV_OK);
        CHECK(cs_mfsv_find(&volume,&work,(const uint8_t *)"F",1,&entry) == CS_MFSV_OK);
        reads = 0;
        memcpy(expected,disk,sizeof disk);
        CHECK(OVERWRITE(CS_MFSV_DATA,0,1,1) == CS_MFSV_UNAVAILABLE);
        CHECK(OVERWRITE(CS_MFSV_RESOURCE,0,1,1) == CS_MFSV_UNAVAILABLE);
        CHECK(memcmp(&progress,&sentinel,sizeof progress) == 0 && !reads && !writes);
        CHECK(OVERWRITE(CS_MFSV_RESOURCE,41,0,0) == CS_MFSV_OK);
        CHECK(!reads && !writes && memcmp(disk,expected,sizeof disk) == 0);
    }
    printf("MFS write: %u checks passed\n", checks);
    return 0;
}
