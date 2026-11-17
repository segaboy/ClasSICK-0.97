/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_BLOCK_READ_H
#define CLASSICK_CORE_STORAGE_BLOCK_READ_H

#include <stddef.h>
#include <stdint.h>

#define CS_BLOCK_BYTES 512u
#define CS_BLOCK_MAX_COUNT 63u
#define CS_BLOCK_NATIVE_LIMIT 32767u

typedef enum {
    CS_BLOCK_IO_OK = 0,
    CS_BLOCK_IO_UNAVAILABLE = 1,
    CS_BLOCK_IO_ERROR = 2
} cs_block_io_result;

/* Synchronous native callback. Success fills exactly 512 bytes; failure may
 * change any of those bytes. Never retain the pointer or write beyond it. */
typedef cs_block_io_result (*cs_block_read_one)(void *context, uint32_t lba,
    uint8_t *block);

typedef struct {
    uint32_t blocks;
    cs_block_read_one read;
    void *context;
} cs_block_device;

typedef enum {
    CS_BLOCK_OK = 0,
    CS_BLOCK_ARGUMENT = 1,
    CS_BLOCK_RANGE = 2,
    CS_BLOCK_CAPACITY = 3,
    CS_BLOCK_UNAVAILABLE = 4,
    CS_BLOCK_IO = 5,
    CS_BLOCK_PROVIDER = 6
} cs_block_result;

/* SPEC-0111: each admitted buffer span is <=32767 bytes, inside one native
 * object and confined to its native segment. Truthful live extents, all controls/provider backing and
 * buffers disjoint, exclusive stable access, and a conforming callback are
 * caller preconditions. Exact buffer equality refuses; arbitrary overlap and
 * far aliases are NOT detected. No guest addresses or physical pointers.
 * count=1..63; device->blocks is a count, so its maximum LBA is blocks-1.
 * All refusals preserve output, including its unused tail. Staging and provider
 * state may retain partial effects after entry. Publish only after every read
 * succeeds; no callback, allocation or fallible operation occurs during copy. */
cs_block_result cs_block_read(const cs_block_device *device, uint32_t lba,
    uint32_t count, uint8_t *output, size_t output_capacity,
    uint8_t *staging, size_t staging_capacity);

#endif
